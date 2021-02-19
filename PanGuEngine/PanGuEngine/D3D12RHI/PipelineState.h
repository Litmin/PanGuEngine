#pragma once
#include "RootSignature.h"
#include "ShaderResourceLayout.h"
#include "ShaderResourceCache.h"
#include "ShaderVariable.h"
#include "Shader.h"
#include "RenderPass.h"
#include "ShaderResourceBinding.h"
#include "ShaderResourceBindingUtility.h"


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
		SHADER_TYPE ShaderType = SHADER_TYPE_UNKNOWN;
		std::string Name;
		SHADER_RESOURCE_VARIABLE_TYPE Type;
	};

	// 描述该PSO中ShaderVariableType的配置，上层可以指定ShaderVariable的类型，没有指定的就是DefaultVariableType
	struct ShaderVariableConfig
	{
		SHADER_RESOURCE_VARIABLE_TYPE DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

		std::vector<ShaderResourceVariableDesc> Variables;
	};

	struct PipelineStateDesc
	{
		std::wstring Name = L"Default PSO";

		PIPELINE_TYPE PipelineType;

		ShaderVariableConfig VariableConfig;

		GraphicsPipelineDesc GraphicsPipeline;

		ComputePipelineDesc ComputePipeline;
	};

	/*
	* 包含了管线中的所有状态
	*/
	class PipelineState
	{
	public:
		PipelineState(RenderDevice* renderDevice,
					  const PipelineStateDesc& desc);
		~PipelineState();

		void InitShaderObjects();

		const PipelineStateDesc& GetDesc() const { return m_Desc; }
		
		UINT32 GetStaticVariableCount(SHADER_TYPE ShaderType) const;
		ShaderVariable* GetStaticVariableByName(SHADER_TYPE shaderType, std::string name);
		ShaderVariable* GetStaticVariableByIndex(SHADER_TYPE shaderType, UINT32 index);

		// 创建SRB,应用程序通过SRB绑定Mutable和Dynamic资源,SRB对象由PSO所有
		ShaderResourceBinding* CreateShaderResourceBinding();

		// 提交资源,用一个结构体来包含众多参数，不然参数太多，不好看
		struct CommitAndTransitionResourcesAttribs
		{
			ShaderResourceBinding*  pShaderResourceBinding = nullptr;
			bool                    CommitResources = false;		// 是否提交资源
			bool                    TransitionResources = false;	// 是否过度资源状态
			bool                    ValidateStates = false;			// 是否验证资源的状态
		};
		void CommitShaderResource(bool isStatic);

		/*
		 * 遍历Shader，执行操作
		 */
		template<typename TOperation>
		void ProcessShaders(TOperation operation) const
		{
			for(const auto& [shaderType, shader] : m_Shaders)
			{
				
			}
		}

		ID3D12PipelineState* GetD3D12PipelineState() const { return m_D3D12PSO.Get(); }
		ID3D12RootSignature* GetD3D12RootSignature() const { return m_RootSignature.GetD3D12RootSignature(); }
		const RootSignature& GetRootSignature() const { return m_RootSignature; }
		RenderDevice* GetRenderDevice() const { return m_RenderDevice; }

	private:
		RenderDevice* m_RenderDevice;
		PipelineStateDesc m_Desc;

		// 资源绑定系统
		RootSignature m_RootSignature;
		// 使用shared_ptr,一个Shader可能被多个PSO共用
		std::unordered_map<SHADER_TYPE, std::shared_ptr<Shader>> m_Shaders;
		std::unordered_map<SHADER_TYPE, ShaderResourceLayout> m_ShaderResourceLayouts;
		// 存储所有Static资源，在切换PSO时，提交Static资源
		std::unique_ptr<ShaderResourceBinding> m_StaticSRB;
		// 相当于Material
		std::vector<ShaderResourceBinding> m_MutableDynamicSRBs;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_D3D12PSO;

		
		std::shared_ptr<RenderPass> m_RenderPass;
	};
}

