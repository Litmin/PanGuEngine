#pragma once
#include <unordered_map>

class ShaderManager : public Singleton<ShaderManager>
{
public:
	ShaderManager();
	~ShaderManager();

	UINT PropertyToID(std::string property);
	UINT GetPerObjectBufferID() { return m_PerObjectBufferID; }
	UINT GetPerCameraBufferID() { return m_PerCameraBufferID; }
	UINT GetPerMaterialBuffer() { return m_PerMaterialBufferID; }
private:
	UINT m_CurID;
	std::unordered_map<std::string, UINT> m_PropertyID;

	UINT m_PerObjectBufferID;
	UINT m_PerCameraBufferID;
	UINT m_PerMaterialBufferID;
};

