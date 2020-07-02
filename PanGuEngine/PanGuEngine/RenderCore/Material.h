#pragma once

extern const int gNumFrameResources;

struct PBRMaterialConstant
{
	PBRMaterialConstant(float albedo, float metallic, float smoothness) :
		albedo(albedo),
		metallic(metallic),
		smoothness(smoothness)
	{}

	float albedo;
	float metallic;
	float smoothness;
};

class PBRMaterial
{
public:
	PBRMaterial(float albedo, float metallic, float smoothness);
	PBRMaterial() : PBRMaterial(0.5, 0.5, 0.5){ }

	// ���ݸ��µ�Constant Buffer���������֡��Դ��Buffer�������ˣ�Ҳ����NumFramesDirty==0�����ͷ���True��ʾ�������
	bool UpdateToConstantBuffer();

private:
	PBRMaterialConstant m_ConstantData;
	UINT m_ConstantIndex;

	int m_NumFramesDirty;
};
