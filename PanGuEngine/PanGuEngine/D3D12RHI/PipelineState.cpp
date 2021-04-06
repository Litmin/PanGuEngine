#include "pch.h"
#include "PipelineState.h"
#include "RenderDevice.h"

using namespace std;

namespace RHI
{
	PipelineState::PipelineState(RenderDevice* renderDevice, const PipelineStateDesc& desc) :
		m_RenderDevice{renderDevice},
		m_Desc{desc},
		m_RootSignature{renderDevice}
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

			// 遍历所有Shader，初始化ShaderResourceLayout和RootSignature
			for (const auto& [shaderType, shader] : m_Shaders)
			{
				m_ShaderResourceLayouts.insert(make_pair(shaderType, ShaderResourceLayout(pd3d12Device,
																						  m_Desc.PipelineType,
																						  m_Desc.VariableConfig,
																						  shader->GetShaderResources(),
																						  &m_RootSignature)));
			}

			// 根签名完成初始化，创建Direct3D 12的RootSignature
			m_RootSignature.Finalize(pd3d12Device);


			// 设置Direc3D 12的PSO Desc
			D3D12_GRAPHICS_PIPELINE_STATE_DESC d3d12PSODesc = {};

			// 外部设置的状态
			d3d12PSODesc.InputLayout = m_Desc.GraphicsPipeline.GraphicPipelineState.InputLayout;
			d3d12PSODesc.RasterizerState = m_Desc.GraphicsPipeline.GraphicPipelineState.RasterizerState;
			d3d12PSODesc.BlendState = m_Desc.GraphicsPipeline.GraphicPipelineState.BlendState;
			d3d12PSODesc.DepthStencilState = m_Desc.GraphicsPipeline.GraphicPipelineState.DepthStencilState;
			d3d12PSODesc.SampleMask = m_Desc.GraphicsPipeline.GraphicPipelineState.SampleMask;
			d3d12PSODesc.PrimitiveTopologyType = m_Desc.GraphicsPipeline.GraphicPipelineState.PrimitiveTopologyType;
			d3d12PSODesc.NumRenderTargets = m_Desc.GraphicsPipeline.GraphicPipelineState.NumRenderTargets;
			for (UINT32 i = 0; i < d3d12PSODesc.NumRenderTargets; ++i)
			{
				d3d12PSODesc.RTVFormats[i] = m_Desc.GraphicsPipeline.GraphicPipelineState.RTVFormats[i];
			}
			d3d12PSODesc.SampleDesc.Count = m_Desc.GraphicsPipeline.GraphicPipelineState.SampleDesc.Count;
			d3d12PSODesc.SampleDesc.Quality = m_Desc.GraphicsPipeline.GraphicPipelineState.SampleDesc.Quality;
			d3d12PSODesc.DSVFormat = m_Desc.GraphicsPipeline.GraphicPipelineState.DSVFormat;


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

			d3d12PSODesc.NodeMask = 0;
			d3d12PSODesc.CachedPSO.pCachedBlob = nullptr;
			d3d12PSODesc.CachedPSO.CachedBlobSizeInBytes = 0;
			d3d12PSODesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

			// 创建D3D12 PSO
			ThrowIfFailed(pd3d12Device->CreateGraphicsPipelineState(&d3d12PSODesc, IID_PPV_ARGS(&m_D3D12PSO)));

			// 设置名字
			m_D3D12PSO->SetName(m_Desc.Name.c_str());
			std::wstring rootSignatureName(L"RootSignature For PSO ");
			rootSignatureName.append(m_Desc.Name);
			m_RootSignature.GetD3D12RootSignature()->SetName(rootSignatureName.c_str());

			const static SHADER_RESOURCE_VARIABLE_TYPE staticVarType[] = { SHADER_RESOURCE_VARIABLE_TYPE_STATIC };
			m_StaticSRB = std::make_unique<ShaderResourceBinding>(this, staticVarType, 1);
		}

		// TODO: Compute Pipeline
		// TODO: Mesh Pipeline
	}

	PipelineState::~PipelineState()
	{
		if(m_D3D12PSO)
			m_RenderDevice->SafeReleaseDeviceObject(std::move(m_D3D12PSO));
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

	std::unique_ptr<ShaderResourceBinding> PipelineState::CreateShaderResourceBinding()
	{
		const static SHADER_RESOURCE_VARIABLE_TYPE mutableDynamicVarType[] = { SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC };

 		std::unique_ptr<ShaderResourceBinding> SRB = std::make_unique<ShaderResourceBinding>(this, mutableDynamicVarType, 2);

		return SRB;
	}

	void PipelineState::CommitSRB(CommandContext& cmdContext, ShaderResourceBinding* SRB)
	{
		if(SRB != nullptr)
		{
			SRB->m_ShaderResourceCache.CommitResource(cmdContext);
		}
	}

	void PipelineState::CommitDynamic(CommandContext& cmdContext, ShaderResourceBinding* SRB)
	{
		m_StaticSRB->m_ShaderResourceCache.CommitDynamic(cmdContext);

		if (SRB != nullptr)
		{
			SRB->m_ShaderResourceCache.CommitDynamic(cmdContext);
		}
	}

	void PipelineState::CommitStaticSRB(CommandContext& cmdContext)
	{
		m_StaticSRB->m_ShaderResourceCache.CommitResource(cmdContext);
	}

}

