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

	// Graphic����
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
	public:
		PipelineState(RenderDevice* renderDevice,
					  const PipelineStateDesc& desc);
		~PipelineState();

		void InitShaderObjects();

		const PipelineStateDesc& GetDesc() const { return m_Desc; }
		
		UINT32 GetStaticVariableCount(SHADER_TYPE ShaderType) const;
		ShaderVariable* GetStaticVariableByName(SHADER_TYPE shaderType, std::string name);
		ShaderVariable* GetStaticVariableByIndex(SHADER_TYPE shaderType, UINT32 index);

		// ����SRB,Ӧ�ó���ͨ��SRB��Mutable��Dynamic��Դ,SRB������PSO����
		ShaderResourceBinding* CreateShaderResourceBinding();

		// �ύ��Դ,��һ���ṹ���������ڶ��������Ȼ����̫�࣬���ÿ�
		struct CommitAndTransitionResourcesAttribs
		{
			ShaderResourceBinding*  pShaderResourceBinding = nullptr;
			bool                    CommitResources = false;		// �Ƿ��ύ��Դ
			bool                    TransitionResources = false;	// �Ƿ������Դ״̬
			bool                    ValidateStates = false;			// �Ƿ���֤��Դ��״̬
		};
		void CommitShaderResource(bool isStatic);

		/*
		 * ����Shader��ִ�в���
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

		// ��Դ��ϵͳ
		RootSignature m_RootSignature;
		// ʹ��shared_ptr,һ��Shader���ܱ����PSO����
		std::unordered_map<SHADER_TYPE, std::shared_ptr<Shader>> m_Shaders;
		std::unordered_map<SHADER_TYPE, ShaderResourceLayout> m_ShaderResourceLayouts;
		// �洢����Static��Դ�����л�PSOʱ���ύStatic��Դ
		std::unique_ptr<ShaderResourceBinding> m_StaticSRB;
		// �൱��Material
		std::vector<ShaderResourceBinding> m_MutableDynamicSRBs;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_D3D12PSO;

		
		std::shared_ptr<RenderPass> m_RenderPass;
	};
}

