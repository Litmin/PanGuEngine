#pragma once
#include "RootSignature.h"
#include "ShaderResourceLayout.h"
#include "ShaderResourceCache.h"
#include "ShaderVariable.h"
#include "Shader.h"
#include "RenderPass.h"
#include "ShaderResourceBinding.h"
#include "Constant.h"


namespace RHI
{
	class RenderDevice;

	// Graphic管线
	struct GraphicsPipelineDesc
	{
		std::shared_ptr<Shader> VertexShader;
		std::shared_ptr<Shader> PixelShader;
		std::shared_ptr<Shader> DomainShader;
		std::shared_ptr<Shader> HullShader;
		std::shared_ptr<Shader> GeometryShader;
		std::shared_ptr<Shader> AmplificationShader;
		std::shared_ptr<Shader> MeshShader;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicPipelineState;

		// RenderPass
		UINT8 SubpassIndex = 0;
		std::shared_ptr<RenderPass> pRenderPass;
	};

	// Compute管线
	struct ComputePipelineDesc
	{
		Shader* ComputeShader;
	};

	struct ShaderResourceVariableDesc
	{
		SHADER_TYPE ShaderStage = SHADER_TYPE_UNKNOWN;
		std::string Name;
		SHADER_RESOURCE_VARIABLE_TYPE Type;
	};

	struct StaticSamplerDesc
	{
		SHADER_TYPE ShaderStages;
		std::string Name;
		D3D12_STATIC_SAMPLER_DESC Desc;
	};

	// 描述Shader Variable和Static Sampler
	struct PipelineResourceLayoutDesc
	{
		SHADER_RESOURCE_VARIABLE_TYPE DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

		std::vector<ShaderResourceVariableDesc> Variables;

		std::vector<StaticSamplerDesc> StaticSamplers;
	};

	struct PipelineStateDesc
	{
		PIPELINE_TYPE PipelineType;

		/// This member defines allocation granularity for internal resources required by the shader resource
		/// binding object instances.
		UINT32 SRBAllocationGranularity = 1;

		/// Defines which command queues this pipeline state can be used with
		UINT64 CommandQueueMask = 1;

		PipelineResourceLayoutDesc ResourceLayout;

		GraphicsPipelineDesc GraphicsPipeline;

		ComputePipelineDesc ComputePipeline;
	};

	// 
	class PipelineState
	{
	public:
		PipelineState(RenderDevice* renderDevice,
					  const PipelineStateDesc& desc);
		~PipelineState();

		// 绑定Static资源
		void BindStaticResources();
		// Static Variable数量
		UINT32 GetStaticVariableCount(SHADER_TYPE ShaderType) const;
		ShaderVariable* GetStaticVariableByName(SHADER_TYPE shaderType, std::string name);
		ShaderVariable* GetStaticVariableByIndex(SHADER_TYPE shaderType, UINT32 index);

		// 创建SRB,应用程序通过SRB绑定Mutable和Dynamic资源
		std::unique_ptr<ShaderResourceBinding> CreateShaderResourceBinding();

		bool IsCompatibleWith(const PipelineState* pso) const;

		ID3D12PipelineState* GetD3D12PipelineState() const { return m_D3D12PSO.Get(); }
		ID3D12RootSignature* GetD3D12RootSignature() const { return m_RootSignature.GetD3D12RootSignature(); }
		const RootSignature& GetRootSignature() const { return m_RootSignature; }
		const ShaderResourceLayout& GetShaderResLayout(UINT32 ShaderInd) const
		{
			return m_ShaderResourceLayouts[ShaderInd];
		}
		ShaderResourceLayout& GetStaticShaderResLayout(UINT32 ShaderInd)
		{
			return m_StaticShaderResourceLayouts[ShaderInd];
		}
		ShaderResourceCache& GetStaticShaderResCache(UINT32 ShaderInd)
		{
			return m_StaticResourceCaches[ShaderInd];
		}

		Shader* GetVertexShader() { return m_VertexShader.get(); }
		Shader* GetPixelShader() { return m_PixelShader.get(); }
		Shader* GetGeometryShader() { return m_GeometryShader.get(); }
		Shader* GetDomainShader() { return m_DomainShader.get(); }
		Shader* GetHullShader() { return m_HullShader.get(); }
		Shader* GetComputeShader() { return m_ComputeShader.get(); }

		bool ContainsShaderResources() const;

	private:
		RenderDevice* m_RenderDevice;
		PipelineStateDesc m_Desc;

		std::shared_ptr<Shader> m_VertexShader;
		std::shared_ptr<Shader> m_PixelShader;
		std::shared_ptr<Shader> m_GeometryShader;
		std::shared_ptr<Shader> m_DomainShader;
		std::shared_ptr<Shader> m_HullShader;
		std::shared_ptr<Shader> m_ComputeShader;
		std::shared_ptr<Shader> m_AmplificationShader;
		std::shared_ptr<Shader> m_MeshShader;

		std::shared_ptr<RenderPass> m_RenderPass;

		size_t m_ShaderResourceLayoutHash = 0;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_D3D12PSO;
		RootSignature m_RootSignature;

		std::vector<ShaderResourceLayout> m_ShaderResourceLayouts;
		std::vector<ShaderResourceLayout> m_StaticShaderResourceLayouts;
		// Static的Shader Variable在这存储，Mutable和Dynamic的存储在ShaderResourceBinding的Cache中
		std::vector<ShaderResourceCache> m_StaticResourceCaches;	
		std::vector<ShaderVariableManager> m_staticVarManagers;


		std::array<INT8, (size_t)MAX_SHADERS_IN_PIPELINE> m_ResourceLayoutIndex = { -1,-1,-1,-1,-1 };
	};
}

