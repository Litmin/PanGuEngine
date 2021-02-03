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

		ShaderVariableConfig ResourceLayout;

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
		ShaderResourceBinding* CreateShaderResourceBinding(bool InitStaticResources);

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
		SHADER_TYPE GetShaderType(UINT32 shaderInd)
		{
			return m_ShaderArray[shaderInd]->GetShaderType();
		}

		Shader* GetVertexShader() { return m_VertexShader.get(); }
		Shader* GetPixelShader() { return m_PixelShader.get(); }
		Shader* GetGeometryShader() { return m_GeometryShader.get(); }
		Shader* GetDomainShader() { return m_DomainShader.get(); }
		Shader* GetHullShader() { return m_HullShader.get(); }
		Shader* GetComputeShader() { return m_ComputeShader.get(); }

		UINT32 GetNumShaders() const { return m_ShaderArray.size(); }

		bool ContainsShaderResources() const;

		RenderDevice* GetRenderDevice() const { return m_RenderDevice; }

	private:
		RenderDevice* m_RenderDevice;
		PipelineStateDesc m_Desc;

		// ʹ��shared_ptr,һ��Shader���ܱ����PSO����
		std::shared_ptr<Shader> m_VertexShader;
		std::shared_ptr<Shader> m_PixelShader;
		std::shared_ptr<Shader> m_GeometryShader;
		std::shared_ptr<Shader> m_DomainShader;
		std::shared_ptr<Shader> m_HullShader;
		std::shared_ptr<Shader> m_ComputeShader;
		std::shared_ptr<Shader> m_AmplificationShader;
		std::shared_ptr<Shader> m_MeshShader;

		std::vector<Shader*> m_ShaderArray;

		std::shared_ptr<RenderPass> m_RenderPass;

		size_t m_ShaderResourceLayoutHash = 0;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_D3D12PSO;
		RootSignature m_RootSignature;

		// ���ResourceLayout���ڶ�Ϊ���е�Resource
		std::vector<ShaderResourceLayout> m_ShaderResourceLayouts;
		// ���ResourceLayout���ڰ�������Static Resource
		std::vector<ShaderResourceLayout> m_StaticShaderResourceLayouts;
		// Static��Shader Variable����洢��Mutable��Dynamic�Ĵ洢��ShaderResourceBinding��Cache��
		std::vector<ShaderResourceCache> m_StaticResourceCaches;	
		std::vector<ShaderVariableManager> m_staticVarManagers;


		// ����ShaderTypeת�ɵ�Index�������ShaderType��m_ShaderArray��m_ShaderResourceLayouts�����漸�������е�����
		// �������ҵ�ĳ��Shader��Ӧ��m_ShaderResourceLayouts��m_StaticResourceCaches�ȶ���
		std::array<INT8, (size_t)MAX_SHADERS_IN_PIPELINE> m_ShaderTypeToIndexMap = { -1,-1,-1,-1,-1 };

		std::vector<ShaderResourceBinding> m_SRBs;
	};
}

