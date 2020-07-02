#include "pch.h"
#include "Material.h"

PBRMaterial::PBRMaterial(float albedo, float metallic, float smoothness) : 
	m_ConstantData{ albedo, metallic, smoothness },
	m_NumFramesDirty{0},
	m_ConstantIndex{-1}
{

}

bool PBRMaterial::UpdateToConstantBuffer()
{
	if (m_NumFramesDirty > 0)
	{
		m_NumFramesDirty--;

		return false;
	}

	return true;
}
