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

			// 把Shader缓存下来
			auto addShader = [&](shared_ptr<Shader> shader)
			{
				if (shader != nullptr)
				{
					m_ShaderArray.push_back(shader.get());

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
					m_ShaderTypeToIndexMap[shaderIndex] = m_ShaderArray.size() - 1;
				}
			};

			addShader(m_Desc.GraphicsPipeline.VertexShader);
			addShader(m_Desc.GraphicsPipeline.PixelShader);
			addShader(m_Desc.GraphicsPipeline.GeometryShader);
			addShader(m_Desc.GraphicsPipeline.HullShader);
			addShader(m_Desc.GraphicsPipeline.DomainShader);

			assert(m_ShaderArray.size() > 0 && "没得Shader");

			// 构造ShaderResourceCache、ShaderResourceLayout、ShaderResourceVariable
			m_StaticResourceCaches.insert(m_StaticResourceCaches.end(), m_ShaderArray.size(), ShaderResourceCache{});
			
			m_ShaderResourceLayouts.insert(m_ShaderResourceLayouts.end(), m_ShaderArray.size(), ShaderResourceLayout{});
			m_StaticShaderResourceLayouts.insert(m_StaticShaderResourceLayouts.end(), m_ShaderArray.size(), ShaderResourceLayout{});

			// 使用Shader中的ShaderResource来初始化ShaderResourceLayout，这一过程中也会初始化RootSignature
			InitShaderObjects();

			// 根签名完成初始化，创建Direct3D 12的RootSignature
			m_RootSignature.Finalize(pd3d12Device);


			// 设置Direc3D 12的PSO Desc
			D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12PSODesc = m_Desc.GraphicsPipeline.GraphicPipelineState;

			// 设置Shader
			for (UINT32 i = 0; i < m_ShaderArray.size(); ++i)
			{
				auto* shader = m_ShaderArray[i];
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
		const auto& pipelineResourceLayout = m_Desc.ResourceLayout;

		for (UINT32 i = 0; i < m_ShaderArray.size(); ++i)
		{
			auto* shader = m_ShaderArray[i];

			// 初始化所有资源的ShaderResourceLayout
			m_ShaderResourceLayouts[i].InitializeForAll(pd3d12Device, m_Desc.PipelineType, m_Desc.ResourceLayout, shader->GetShaderResources(), &m_RootSignature);

			// 初始化用来存储Static资源的ShaderResourceCache和ShaderResourceLayout
			const SHADER_RESOURCE_VARIABLE_TYPE StaticVarType[] = { SHADER_RESOURCE_VARIABLE_TYPE_STATIC };
			m_StaticShaderResourceLayouts[i].InitializeForStatic(pd3d12Device, 
																 m_Desc.PipelineType, 
																 m_Desc.ResourceLayout, 
																 shader->GetShaderResources(), 
																 StaticVarType, 
																 _countof(StaticVarType), 
																 &m_StaticResourceCaches[i]);

			// 初始化Static Shader Variables
			m_staticVarManagers.emplace_back(m_StaticResourceCaches[i], m_StaticShaderResourceLayouts[i], StaticVarType, _countof(StaticVarType));
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

	bool PipelineState::ContainsShaderResources() const
	{
		for (auto VarType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC; VarType < SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES; VarType = static_cast<SHADER_RESOURCE_VARIABLE_TYPE>(VarType + 1))
		{
			if (m_RootSignature.GetTotalSrvCbvUavSlots(VarType) != 0 ||
				m_RootSignature.GetTotalRootViews(VarType) != 0)
				return true;
		}
		return false;
	}
}

