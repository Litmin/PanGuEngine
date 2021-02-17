#include "pch.h"
#include "PipelineState.h"
#include "RenderDevice.h"

using namespace std;

namespace RHI
{
	PipelineState::PipelineState(RenderDevice* renderDevice, const PipelineStateDesc& desc) :
		m_RenderDevice{renderDevice},
		m_Desc{desc}
	{
		auto pd3d12Device = m_RenderDevice->GetD3D12Device();

		if (m_Desc.PipelineType == PIPELINE_TYPE_GRAPHIC)
		{
			m_ShaderTypeToIndexMap.fill(-1);

			// 把Shader保存到一个数组
			auto addShader = [&](shared_ptr<Shader> shader)
			{
				if (shader != nullptr)
				{
					m_Shaders.push_back(shader.get());

					SHADER_TYPE shaderType = shader->GetShaderType();
					switch (shaderType)
					{
					case SHADER_TYPE_VERTEX:	m_VertexShader = shader; break;
					case SHADER_TYPE_PIXEL:		m_PixelShader = shader;	break;
					case SHADER_TYPE_GEOMETRY:  m_GeometryShader = shader; break;
					case SHADER_TYPE_HULL:		m_HullShader = shader; break;
					case SHADER_TYPE_DOMAIN:	m_DomainShader = shader; break;
					default:
						LOG_ERROR("Unkonw Shader Type.");
					}

					INT32 shaderIndex = GetShaderTypePipelineIndex(shaderType, m_Desc.PipelineType);
					m_ShaderTypeToIndexMap[shaderIndex] = m_Shaders.size() - 1;
				}
			};

			addShader(m_Desc.GraphicsPipeline.VertexShader);
			addShader(m_Desc.GraphicsPipeline.PixelShader);
			addShader(m_Desc.GraphicsPipeline.GeometryShader);
			addShader(m_Desc.GraphicsPipeline.HullShader);
			addShader(m_Desc.GraphicsPipeline.DomainShader);

			assert(m_Shaders.size() > 0 && "没得Shader");


			// 使用Shader中的ShaderResource来初始化ShaderResourceLayout，这一过程中也会初始化RootSignature
			InitShaderObjects();

			// 根签名完成初始化，创建Direct3D 12的RootSignature
			m_RootSignature.Finalize(pd3d12Device);


			// 设置Direc3D 12的PSO Desc
			D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12PSODesc = m_Desc.GraphicsPipeline.GraphicPipelineState;

			// 设置Shader
			for (UINT32 i = 0; i < m_Shaders.size(); ++i)
			{
				auto* shader = m_Shaders[i];
				auto shaderType = shader->GetShaderType();

				D3D12_SHADER_BYTECODE* pd3d12ShaderBytecode = nullptr;
				switch (shaderType)
				{
				case SHADER_TYPE_VERTEX:   pd3d12ShaderBytecode = &d3d12PSODesc.VS; break;
				case SHADER_TYPE_PIXEL:    pd3d12ShaderBytecode = &d3d12PSODesc.PS; break;
				case SHADER_TYPE_GEOMETRY: pd3d12ShaderBytecode = &d3d12PSODesc.GS; break;
				case SHADER_TYPE_HULL:     pd3d12ShaderBytecode = &d3d12PSODesc.HS; break;
				case SHADER_TYPE_DOMAIN:   pd3d12ShaderBytecode = &d3d12PSODesc.DS; break;
				default:
					LOG_ERROR("UnExpected Shader Type.");
					break;
				}

				pd3d12ShaderBytecode->pShaderBytecode = shader->GetShaderByteCode()->GetBufferPointer();
				pd3d12ShaderBytecode->BytecodeLength = shader->GetShaderByteCode()->GetBufferSize();
			}


			// 设置RootSignature
			d3d12PSODesc.pRootSignature = m_RootSignature.GetD3D12RootSignature();

			memset(&d3d12PSODesc.StreamOutput, 0, sizeof(d3d12PSODesc.StreamOutput));

			// For single GPU operation, set this to zero. If there are multiple GPU nodes,
			// set bits to identify the nodes (the device's physical adapters) for which the
			// graphics pipeline state is to apply. Each bit in the mask corresponds to a single node.
			d3d12PSODesc.NodeMask = 0;

			d3d12PSODesc.CachedPSO.pCachedBlob = nullptr;
			d3d12PSODesc.CachedPSO.CachedBlobSizeInBytes = 0;

			// The only valid bit is D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG, which can only be set on WARP devices.
			d3d12PSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

			// 创建D3D12 PSO
			ThrowIfFailed(pd3d12Device->CreateGraphicsPipelineState(&d3d12PSODesc, IID_PPV_ARGS(&m_D3D12PSO)));

			// 设置名字
			m_D3D12PSO->SetName(m_Desc.Name.c_str());
			std::wstring rootSignatureName(L"RootSignature For PSO ");
			rootSignatureName.append(m_Desc.Name);
			m_RootSignature.GetD3D12RootSignature()->SetName(rootSignatureName.c_str());
		}

		// TODO: Compute Pipeline
		// TODO: Mesh Pipeline
	}

	PipelineState::~PipelineState()
	{
		if(m_D3D12PSO)
			m_RenderDevice->SafeReleaseDeviceObject(std::move(m_D3D12PSO));
	}

	// 初始化ShaderResourceLayout
	void PipelineState::InitShaderObjects()
	{
		auto pd3d12Device = m_RenderDevice->GetD3D12Device();

		m_ShaderResourceLayouts.insert(m_ShaderResourceLayouts.end(), m_Shaders.size(), ShaderResourceLayout{});

		for (UINT32 i = 0; i < m_Shaders.size(); ++i)
		{
			auto* shader = m_Shaders[i];
			m_ShaderResourceLayouts[i].InitializeForAll(pd3d12Device, 
														m_Desc.PipelineType, 
														m_Desc.VariableConfig, 
											shader->GetShaderResources(), 
														&m_RootSignature);
		}
		
		m_StaticResourceCache.Initialize();
		
		// 初始化Static Shader Variables
		const SHADER_RESOURCE_VARIABLE_TYPE StaticVarType[] = { SHADER_RESOURCE_VARIABLE_TYPE_STATIC };
		for(UINT32 i = 0;i < m_Shaders.size();++i)
		{
			
			m_staticVarManagers.emplace_back(m_StaticResourceCache, m_ShaderResourceLayouts[i], StaticVarType, _countof(StaticVarType));
		}
	}

	UINT32 PipelineState::GetStaticVariableCount(SHADER_TYPE ShaderType) const
	{
		INT32 shaderIndex = GetShaderTypePipelineIndex(ShaderType, m_Desc.PipelineType);

		return m_staticVarManagers[shaderIndex].GetVariableCount();
	}

	ShaderVariable* PipelineState::GetStaticVariableByName(SHADER_TYPE shaderType, std::string name)
	{
		INT32 shaderIndex = GetShaderTypePipelineIndex(shaderType, m_Desc.PipelineType);

		return m_staticVarManagers[shaderIndex].GetVariable(name);
	}

	ShaderVariable* PipelineState::GetStaticVariableByIndex(SHADER_TYPE shaderType, UINT32 index)
	{
		INT32 shaderIndex = GetShaderTypePipelineIndex(shaderType, m_Desc.PipelineType);

		return m_staticVarManagers[shaderIndex].GetVariable(index);
	}

	ShaderResourceBinding* PipelineState::CreateShaderResourceBinding(bool InitStaticResources)
	{
		m_SRBs.emplace_back(this);

		if (InitStaticResources)
			m_SRBs.back().InitializeStaticResources();

		return &m_SRBs.back();
	}

}

