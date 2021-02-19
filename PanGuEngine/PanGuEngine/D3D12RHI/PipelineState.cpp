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
			auto addShader = [&](shared_ptr<Shader> shader)
			{
				if (shader != nullptr)
				{
					m_Shaders.insert(make_pair(shader->GetShaderType(), shader));
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

			// 设置PSO的Shader
			for(const auto& [shaderType, shader] : m_Shaders)
			{
				switch (shaderType)
				{
				case SHADER_TYPE_VERTEX:
					d3d12PSODesc.VS.pShaderBytecode = shader->GetShaderByteCode()->GetBufferPointer();
					d3d12PSODesc.VS.BytecodeLength = shader->GetShaderByteCode()->GetBufferSize();
					break;
				case SHADER_TYPE_PIXEL:
					d3d12PSODesc.PS.pShaderBytecode = shader->GetShaderByteCode()->GetBufferPointer();
					d3d12PSODesc.PS.BytecodeLength = shader->GetShaderByteCode()->GetBufferSize();
					break;
				case SHADER_TYPE_GEOMETRY:
					d3d12PSODesc.GS.pShaderBytecode = shader->GetShaderByteCode()->GetBufferPointer();
					d3d12PSODesc.GS.BytecodeLength = shader->GetShaderByteCode()->GetBufferSize();
					break;
				case SHADER_TYPE_HULL:
					d3d12PSODesc.HS.pShaderBytecode = shader->GetShaderByteCode()->GetBufferPointer();
					d3d12PSODesc.HS.BytecodeLength = shader->GetShaderByteCode()->GetBufferSize();
					break;
				case SHADER_TYPE_DOMAIN:
					d3d12PSODesc.DS.pShaderBytecode = shader->GetShaderByteCode()->GetBufferPointer();
					d3d12PSODesc.DS.BytecodeLength = shader->GetShaderByteCode()->GetBufferSize();
					break;
				default:
					LOG_ERROR("UnExpected Shader Type.");
					break;
				}
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

		for (UINT32 i = 0; i < m_Shaders.size(); ++i)
		{
			auto* shader = m_Shaders[i];
			m_ShaderResourceLayouts[i].InitializeForAll(pd3d12Device, 
														m_Desc.PipelineType, 
														m_Desc.VariableConfig, 
											shader->GetShaderResources(), 
														&m_RootSignature);
		}

		
		for (const auto& [shaderType, shader] : m_Shaders)
		{
			m_ShaderResourceLayouts.insert(make_pair(shaderType, ShaderResourceLayout()));
		}
	}

	UINT32 PipelineState::GetStaticVariableCount(SHADER_TYPE ShaderType) const
	{
		return m_StaticSRB->GetVariableCount(ShaderType);
	}

	ShaderVariable* PipelineState::GetStaticVariableByName(SHADER_TYPE shaderType, std::string name)
	{
		return m_StaticSRB->GetVariableByName(shaderType, name);
	}

	ShaderVariable* PipelineState::GetStaticVariableByIndex(SHADER_TYPE shaderType, UINT32 index)
	{
		return m_StaticSRB->GetVariableByIndex(shaderType, index);
	}

	ShaderResourceBinding* PipelineState::CreateShaderResourceBinding()
	{
		m_MutableDynamicSRBs.emplace_back(this);

		return &m_MutableDynamicSRBs.back();
	}

}

