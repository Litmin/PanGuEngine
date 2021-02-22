#include "pch.h"
#include "ShaderResourceLayout.h"
#include "TextureView.h"
#include "BufferView.h"
#include "Buffer.h"
#include "ShaderResourceBindingUtility.h"
#include "RootSignature.h"

namespace RHI
{
	

	ShaderResourceLayout::ShaderResourceLayout(ID3D12Device* pd3d12Device,
											   PIPELINE_TYPE pipelineType,
											   const ShaderVariableConfig& shaderVariableConfig, 
											   const ShaderResource* shaderResource,
											   RootSignature* rootSignature) :
		m_D3D12Device(pd3d12Device)
	{
		// ��ShaderResource�е�ÿ����Դ��ͨ��RootSignatureȷ��RootIndex��OffsetFromTableStart��Ȼ��洢����
		auto AddResource = [&](const ShaderResourceAttribs& Attribs,
							   CachedResourceType              ResType,
							   SHADER_RESOURCE_VARIABLE_TYPE   VarType) //
		{
			assert(rootSignature != nullptr);
			
			UINT32 RootIndex = Resource::InvalidRootIndex;
			UINT32 Offset = Resource::InvalidOffset;

			D3D12_DESCRIPTOR_RANGE_TYPE DescriptorRangeType = GetDescriptorRangeType(ResType);
			SHADER_TYPE shaderType = shaderResource->GetShaderType();
			
			// ����ShaderResource�е�˳����ӵ�RootSignature�У�������RootIndex��Offset
			rootSignature->AllocateResourceSlot(shaderType, pipelineType, Attribs, VarType, DescriptorRangeType, RootIndex, Offset);

			m_SrvCbvUavs[VarType].emplace_back(*this, Attribs, VarType, ResType, RootIndex, Offset);
		};

		shaderResource->ProcessResources(
			[&](const ShaderResourceAttribs& CB, UINT32)
		{
			auto VarType = shaderResource->FindVariableType(CB, shaderVariableConfig);
			AddResource(CB, CachedResourceType::CBV, VarType);
		},
			[&](const ShaderResourceAttribs& TexSRV, UINT32)
		{
			auto VarType = shaderResource->FindVariableType(TexSRV, shaderVariableConfig);
			AddResource(TexSRV, CachedResourceType::TexSRV, VarType);
		},
			[&](const ShaderResourceAttribs& TexUAV, UINT32)
		{
			auto VarType = shaderResource->FindVariableType(TexUAV, shaderVariableConfig);
			AddResource(TexUAV, CachedResourceType::TexUAV, VarType);
		},
			[&](const ShaderResourceAttribs& BufSRV, UINT32)
		{
			auto VarType = shaderResource->FindVariableType(BufSRV, shaderVariableConfig);
			AddResource(BufSRV, CachedResourceType::BufSRV, VarType);
		},
			[&](const ShaderResourceAttribs& BufUAV, UINT32)
		{
			auto VarType = shaderResource->FindVariableType(BufUAV, shaderVariableConfig);
			AddResource(BufUAV, CachedResourceType::BufUAV, VarType);
		}
		);
	}

	void ShaderResourceLayout::Resource::CacheCB(IShaderResource* pBuffer,
	                                             ShaderResourceCache::Resource* dstRes) const
	{
		Buffer* buffer = dynamic_cast<Buffer*>(pBuffer);

		if (buffer)
		{
			if (VariableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC && dstRes->pObject != nullptr)
			{
				// ����Ѿ�������Դ�Ͳ����£�������Dynamic��Դ
				return;
			}

			// Constant Buffer����ҪDescriptor��ֱ�Ӱ󶨵�Root View
			dstRes->Type = ResourceType;
			dstRes->pObject.reset(buffer);
		}
	}

	// ��ģ�庯��������cpp�ж��壬��Ϊֻ�ڱ��ļ���ʹ��
	template<typename TResourceViewType>
	void ShaderResourceLayout::Resource::CacheResourceView(IShaderResource* pView,
														   ShaderResourceCache::Resource* dstRes,
														   D3D12_CPU_DESCRIPTOR_HANDLE shaderVisibleHeapCPUDescriptorHandle) const
	{
		TResourceViewType* resourceView = dynamic_cast<TResourceViewType*>(pView);

		if (resourceView)
		{
			if (VariableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC && dstRes->pObject != nullptr)
			{
				// ����Ѿ�������Դ�Ͳ����£�������Dynamic��Դ
				return;
			}

			dstRes->Type = ResourceType;
			dstRes->CPUDescriptorHandle = resourceView->GetCPUDescriptorHandle();

			if (shaderVisibleHeapCPUDescriptorHandle.ptr != 0)
			{
				ID3D12Device* d3d12Device = ParentResLayout.m_D3D12Device;
				d3d12Device->CopyDescriptorsSimple(1, 
								shaderVisibleHeapCPUDescriptorHandle, 
								dstRes->CPUDescriptorHandle, 
												   D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}

			dstRes->pObject = resourceView;
		}
	}

	bool ShaderResourceLayout::Resource::IsBound(UINT32 arrayIndex, const ShaderResourceCache& resourceCache) const
	{
		if (RootIndex < resourceCache.GetRootTablesNum())
		{
			// ShaderResourceCache::RootTable
			const ShaderResourceCache::RootTable& rootTable = resourceCache.GetRootTable(RootIndex);
			if (OffsetFromTableStart + arrayIndex < rootTable.Size())
			{
				// ShaderResourceCache::Resource
				const ShaderResourceCache::Resource* CachedRes =
					rootTable.GetResource(OffsetFromTableStart + arrayIndex);

				if (CachedRes->pObject != nullptr)
					return true;
			}
		}

		return false;
	}

	// ����Դ������������
	// ����ֻ�ǰ���Դ��Descriptor��������Cache�У�û���ύ����Ⱦ����
	void ShaderResourceLayout::Resource::BindResource(IShaderResource* pObject, UINT32 arrayIndex, ShaderResourceCache& resourceCache) const
	{
		ShaderResourceCache::Resource* DstRes;
		D3D12_CPU_DESCRIPTOR_HANDLE ShdrVisibleHeapCPUDescriptorHandle = {0};

		if(ResourceType == CachedResourceType::CBV)
		{
			DstRes = resourceCache.GetRootView(RootIndex).GetResource();

		}
		else
		{
			DstRes = resourceCache.GetRootTable(RootIndex).GetResource(OffsetFromTableStart + arrayIndex);
			ShdrVisibleHeapCPUDescriptorHandle = resourceCache.GetShaderVisibleTableCPUDescriptorHandle<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>(RootIndex, OffsetFromTableStart + arrayIndex);
		}
		
		if (pObject)
		{
			switch (ResourceType)
			{
			case CachedResourceType::CBV:
				CacheCB(pObject, DstRes);
				break;

			case CachedResourceType::TexSRV:
				CacheResourceView<TextureView>(pObject, DstRes, ShdrVisibleHeapCPUDescriptorHandle);
				break;

			case CachedResourceType::TexUAV:
				CacheResourceView<TextureView>(pObject, DstRes, ShdrVisibleHeapCPUDescriptorHandle);
				break;

			case CachedResourceType::BufSRV:
				CacheResourceView<BufferView>(pObject, DstRes, ShdrVisibleHeapCPUDescriptorHandle);
				break;

			case CachedResourceType::BufUAV:
				CacheResourceView<BufferView>(pObject, DstRes, ShdrVisibleHeapCPUDescriptorHandle);
				break;

			default:
				LOG_ERROR("Unkown CachedResourceType.");
				break;
			}
		}
	}
}