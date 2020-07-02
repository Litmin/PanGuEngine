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

	// 数据更新到Constant Buffer，如果所有帧资源的Buffer都更新了（也就是NumFramesDirty==0），就返回True表示更新完成
	bool UpdateToConstantBuffer();

private:
	PBRMaterialConstant m_ConstantData;
	UINT m_ConstantIndex;

	int m_NumFramesDirty;
};
