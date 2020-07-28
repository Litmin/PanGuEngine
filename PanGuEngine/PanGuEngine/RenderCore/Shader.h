#pragma once
#include <unordered_map>
#include "ShaderManager.h"

enum class ShaderParamType
{
	ConstantBuffer,
	CBVDescriptorHeap,
	SRVDescriptorHeap,
	UAVDescriptorHeap,
	StructuredBuffer
};

struct ShaderParameter
{
	std::string name;
	ShaderParamType type;
	UINT descriptorNums;
	UINT baseRegister;
	UINT registerSpace;

	ShaderParameter() {}
	ShaderParameter(
		const std::string& name,
		ShaderParamType type,
		UINT descriptorNums,
		UINT baseRegister,
		UINT registerSpace) :
		name(name),
		type(type),
		descriptorNums(descriptorNums),
		baseRegister(baseRegister),
		registerSpace(registerSpace) {}
};

//************************************************************
// Shader基类：
//		真正运行在GPU的Shader(VS、FS)
//		渲染状态(光栅化、深度模板测试、混合)
//		参数布局
//
// 具体的Shader需要继承Shader基类，在构造函数完成Shader参数的绑定
//************************************************************
class Shader
{
public:
	Shader();
	Shader(ID3D12Device* device);
	virtual ~Shader();

	virtual void BindShaderFilePath() {}
	virtual void BindShaderParam();

	void BindRootSignature(ID3D12GraphicsCommandList* commandList);
	// 绑定参数，材质通过该接口将保存的参数绑定到管线中
	// TODO:抽象GPU资源，绑定参数整合为一个接口
	void SetDescriptorTable(ID3D12GraphicsCommandList* commandList, UINT paramID, CD3DX12_GPU_DESCRIPTOR_HANDLE handle);
	void SetRootConstantBufferView(ID3D12GraphicsCommandList* commandList, UINT paramID, D3D12_GPU_VIRTUAL_ADDRESS address);

	void SetPSODesc(D3D12_GRAPHICS_PIPELINE_STATE_DESC* pso);
private:
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
	Microsoft::WRL::ComPtr<ID3DBlob> m_VS;
	Microsoft::WRL::ComPtr<ID3DBlob> m_PS;
	D3D12_RASTERIZER_DESC m_RasterizerState;
	D3D12_DEPTH_STENCIL_DESC m_DepthStencilState;
	D3D12_BLEND_DESC m_BlendState;

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();
protected:
	// Shader参数
	std::unordered_map<UINT/*PropertyID*/, UINT/*Index*/> m_ParamMap;
	std::vector<ShaderParameter> m_Params;

	std::wstring m_FilePath;
	std::string m_VSEntry;
	std::string m_PSEntry;
};



