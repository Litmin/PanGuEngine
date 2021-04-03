#pragma once
#include "RootSignature.h"
#include "ShaderResourceLayout.h"
#include "ShaderResourceCache.h"
#include "ShaderVariable.h"
#include "Shader.h"
#include "ShaderResourceBinding.h"
#include "ShaderResourceBindingUtility.h"


namespace RHI
{
	class RenderDevice;
	class CommandContext;

	// Graphic����
	struct GraphicsPipelineDesc
	{
		std::shared_ptr<Shader> VertexShader = nullptr;
		std::shared_ptr<Shader> PixelShader = nullptr;
		std::shared_ptr<Shader> DomainShader = nullptr;
		std::shared_ptr<Shader> HullShader = nullptr;
		std::shared_ptr<Shader> GeometryShader = nullptr;
		std::shared_ptr<Shader> AmplificationShader = nullptr;
		std::shared_ptr<Shader> MeshShader = nullptr;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicPipelineState;

		// RenderPass
		UINT8 SubpassIndex = 0;
	};

	// Compute����
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

	// ������PSO��ShaderVariableType�����ã��ϲ����ָ��ShaderVariable�����ͣ�û��ָ���ľ���DefaultVariableType
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
	* �����˹����е�����״̬
	*/
	class PipelineState
	{
		friend class CommandContext;

	public:
		PipelineState(RenderDevice* renderDevice,
					  const PipelineStateDesc& desc);
		~PipelineState();

		const PipelineStateDesc& GetDesc() const { return m_Desc; }
		
		UINT32 GetStaticVariableCount(SHADER_TYPE ShaderType) const;
		ShaderVariable* GetStaticVariableByName(SHADER_TYPE shaderType, std::string name);
		ShaderVariable* GetStaticVariableByIndex(SHADER_TYPE shaderType, UINT32 index);

		// ����SRB,Ӧ�ó���ͨ��SRB��Mutable��Dynamic��Դ,SRB������PSO����
		ShaderResourceBinding* CreateShaderResourceBinding();

		/*
		 * ����Shader��ִ�в���
		 */
		template<typename TOperation>
		void ProcessShaders(TOperation Operation) const
		{
			for(const auto& [shaderType, shader] : m_Shaders)
			{
				Operation(shaderType, m_ShaderResourceLayouts.at(shaderType));
			}
		}

		ID3D12PipelineState* GetD3D12PipelineState() const 
		{
			return m_D3D12PSO.Get(); 
		}
		ID3D12RootSignature* GetD3D12RootSignature() const { return m_RootSignature.GetD3D12RootSignature(); }
		const RootSignature* GetRootSignature() const { return &m_RootSignature; }
		RenderDevice* GetRenderDevice() const { return m_RenderDevice; }

	private:
		void CommitStaticSRB(CommandContext& cmdContext);
		void CommitSRB(CommandContext& cmdContext, ShaderResourceBinding* SRB);
		void CommitDynamic(CommandContext& cmdContext, ShaderResourceBinding* SRB);

		RenderDevice* m_RenderDevice;
		PipelineStateDesc m_Desc;

		// ��Դ��ϵͳ
		RootSignature m_RootSignature;
		// ʹ��shared_ptr,һ��Shader���ܱ����PSO����
		std::unordered_map<SHADER_TYPE, std::shared_ptr<Shader>> m_Shaders;
		std::unordered_map<SHADER_TYPE, ShaderResourceLayout> m_ShaderResourceLayouts;
		// �洢����Static��Դ�����л�PSOʱ���ύStatic��Դ
		std::unique_ptr<ShaderResourceBinding> m_StaticSRB;
		// �൱��Material
		std::vector<std::unique_ptr<ShaderResourceBinding>> m_MutableDynamicSRBs;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_D3D12PSO;
	};
}

