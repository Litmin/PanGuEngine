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
		if (m_Desc.PipelineType == PIPELINE_TYPE_GRAPHIC)
		{
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

			for (UINT32 i = 0; i < m_ShaderArray.size(); ++i)
			{
				m_staticVarManagers.emplace_back(m_StaticResourceCaches[i]);
			}

			// 初始化ShaderResourceLayout
			InitResourceLayout();




			m_RenderPass = m_Desc.GraphicsPipeline.pRenderPass;
			// TODO: 设置RenderPass

		}


		// TODO: Compute Pipeline
		// TODO: Mesh Pipeline
	}

	PipelineState::~PipelineState()
	{
		m_RenderDevice->SafeReleaseDeviceObject(std::move(m_D3D12PSO));
	}

	// 初始化ShaderResourceLayout
	void PipelineState::InitResourceLayout()
	{
		auto pd3d12Device = m_RenderDevice->GetD3D12Device();
		const auto& pipelineResourceLayout = m_Desc.ResourceLayout;

		for (UINT32 i = 0; i < m_ShaderArray.size(); ++i)
		{

		}


	}
}

