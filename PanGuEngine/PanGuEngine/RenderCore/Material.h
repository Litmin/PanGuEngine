#pragma once

extern const int gNumFrameResources;

struct PBRMaterialConstant
{
	float albedo;
	float metallic;
	float smoothness;
};

class PBRMaterial
{
public:
	void ApplyMaterialParam();

private:
	PBRMaterialConstant m_ConstantData;
	UINT m_ConstantIndex;

	int NumFramesDirty;
};
