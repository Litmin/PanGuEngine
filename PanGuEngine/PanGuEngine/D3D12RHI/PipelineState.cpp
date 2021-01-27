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

			for (UINT32 i = 0; i < m_ShaderArray.size(); ++i)
			{
				m_staticVarManagers.emplace_back(m_StaticResourceCaches[i]);
			}

			// 使用Shader中的ShaderResource来初始化ShaderResourceLayout，这一过程中也会初始化RootSignature
			InitShaderObjects();

			// 根签名完成初始化，创建Direct3D 12的RootSignature
			auto pd3d12Device = m_RenderDevice->GetD3D12Device();
			m_RootSignature.Finalize(pd3d12Device);

			// 设置Direc3D 12的PSO Desc

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
}

