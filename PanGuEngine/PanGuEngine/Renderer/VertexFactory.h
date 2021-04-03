#pragma once

class VertexFactory : public Singleton<VertexFactory>
{
public:
	// ÿ�����붥��Ĳ��ֶ�ӦΨһ������
	UINT GetInputLayoutIndex(bool hasColor, bool hasNormal, bool hasTangent, bool hasuv0,
							 bool hasuv1, bool hasuv2, bool hasuv3);
	std::vector<D3D12_INPUT_ELEMENT_DESC>* GetInputElementDesc(UINT index);

private:
	void GenerateInputElementDesc(std::vector<D3D12_INPUT_ELEMENT_DESC>& desc, bool hasColor, bool hasNormal, 
								  bool hasTangent, bool hasuv0, bool hasuv1, bool hasuv2, bool hasuv3);

	std::unordered_map<USHORT, UINT> m_InputLayoutMap;
	std::vector<std::unique_ptr<std::vector<D3D12_INPUT_ELEMENT_DESC>>> m_InputLayouts;
};
