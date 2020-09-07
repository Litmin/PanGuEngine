#pragma once
#include <unordered_map>
#include "ShaderManager.h"

/*----------------------------------------------------
	Shader�Ĳ�������
		���������������ֱ����Ϊ��������
----------------------------------------------------*/
enum class ShaderParamType
{
	ConstantBuffer,		// ����������CBV
	CBVDescriptorHeap,	// ����������CBV
	SRVDescriptorHeap,	// ����������SRV
	UAVDescriptorHeap,	// ����������UAV
	StructuredBuffer	// TODO:
};

/*----------------------------------------------------
	Shader����
		���֡����͡�������������Register��RegisterSpace
----------------------------------------------------*/
struct ShaderParameter
{
	std::string name;
	ShaderParamType type;
	UINT descriptorNums;
	UINT baseRegister;
	UINT registerSpace;

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

/*----------------------------------------------------
	Shader���ࣺ
		����������GPU��Shader(VS��FS)
		��Ⱦ״̬(��դ�������ģ����ԡ����)
		��������

	�����Shader��Ҫ�̳�Shader���࣬�ڹ��캯�����Shader�����İ�
----------------------------------------------------*/
class Shader
{
public:
	Shader();
	virtual ~Shader();
	void Initialize(ID3D12Device* device);

	void BindRootSignature(ID3D12GraphicsCommandList* commandList);
	// �󶨲���������ͨ���ýӿڽ�����Ĳ����󶨵�������
	// TODO:����GPU��Դ���󶨲�������Ϊһ���ӿ�
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
	// Shader����
	// <Shader�Ĳ���ID����������ڸ�ǩ���ĸ���������>
	std::unordered_map<UINT, UINT> m_ParamMap;
	std::vector<ShaderParameter> m_Params;

	std::wstring m_FilePath;
	std::string m_VSEntry;
	std::string m_PSEntry;
};



