#include "pch.h"
#include "ShaderResourceCache.h"
#include "RootSignature.h"
#include "RenderDevice.h"

using namespace std;

namespace RHI
{
	// ֻ����RootTable��Resource����
	void ShaderResourceCache::Initialize(RenderDevice* device, 
										 const RootSignature* rootSignature,
										 const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
										 UINT32 allowedTypeNum)
	{
		assert(rootSignature != nullptr);

		const UINT32 allowedTypeBits = GetAllowedTypeBits(allowedVarTypes, allowedTypeNum);

		rootSignature->ProcessRootViews([&](RootParameter& rootView)
		{
			SHADER_RESOURCE_VARIABLE_TYPE variableType = rootView.GetShaderVariableType();
			UINT32 rootIndex = rootView.GetRootIndex();
			
			if (IsAllowedType(variableType, allowedTypeBits))
				m_RootViews.insert(make_pair(rootIndex, RootView()));
		});

		UINT32 descriptorNum = 0;
		
		rootSignature->ProcessRootTables([&](RootParameter& rootTable)
		{
			SHADER_RESOURCE_VARIABLE_TYPE variableType = rootTable.GetShaderVariableType();
			UINT32 rootIndex = rootTable.GetRootIndex();
			UINT32 rootTableSize = rootTable.GetDescriptorTableSize();
			const auto& D3D12RootParam = static_cast<const D3D12_ROOT_PARAMETER&>(rootTable); // ��ʽת��

			assert(D3D12RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);
			assert(rootTableSize > 0 && "Unexpected empty descriptor table");


			if (IsAllowedType(variableType, allowedTypeBits))
				m_RootTables.insert(make_pair(rootIndex, RootTable(rootTableSize)));

			if (variableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
			{
				// ����ÿ��Root Table��Heap�е���ʼλ�ã���ΪRoot Table�ǽ������еģ�������ʼλ�þ��ǵ�ǰ������
				m_RootTables[rootIndex].m_TableStartOffset = descriptorNum;
				descriptorNum += rootTableSize;
			}
		});

		// ����GPU Descriptor Heap�ϵĿռ�
		if (descriptorNum)
		{
			m_CbvSrvUavGPUHeapSpace = device->AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, descriptorNum);
			assert(!m_CbvSrvUavGPUHeapSpace.IsNull() && "Failed to allocate  GPU-visible CBV/SRV/UAV descriptor");
		}
	}
}