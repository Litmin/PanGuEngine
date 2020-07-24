#pragma once
#include <unordered_map>

class ShaderManager : public Singleton<ShaderManager>
{
public:
	ShaderManager();
	~ShaderManager();

private:
	std::unordered_map<std::string, UINT> m_PropertyID;
};

