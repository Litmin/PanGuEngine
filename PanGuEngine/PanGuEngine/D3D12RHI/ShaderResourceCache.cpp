#include "pch.h"
#include "ShaderResourceCache.h"

namespace RHI
{
	// ֻ����RootTable��Resource����
	void ShaderResourceCache::Initialize(UINT32 tableNum, UINT32 tableSizes[])
	{
		for (UINT32 i = 0; i < tableNum; ++i)
		{
			m_RootTables.emplace_back(tableSizes[i]);
		}
	}
}