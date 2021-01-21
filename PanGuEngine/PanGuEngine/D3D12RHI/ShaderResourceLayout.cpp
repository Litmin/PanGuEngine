#include "pch.h"
#include "ShaderResourceLayout.h"
#include "TextureView.h"
#include "BufferView.h"
#include "Buffer.h"
#include "ShaderResourceBindingUtility.h"
#include "RootSignature.h"

namespace RHI
{
	// CachedResourceType to D3D12_DESCRIPTOR_RANGE_TYPE
	D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(CachedResourceType cachedResType)
	{
		switch (cachedResType)
		{
		case CachedResourceType::CBV:
			return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		case CachedResourceType::BufSRV:
		case CachedResourceType::TexSRV:
			return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		case CachedResourceType::BufUAV:
		case CachedResourceType::TexUAV:
			return D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
		case CachedResourceType::Sampler:
			return D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER;
		default:
			LOG_ERROR("Unkown CachedResourceType.");
			return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		}
	}


	void ShaderResourceLayout::Initialize(ID3D12Device* pd3d12Device,
		PIPELINE_TYPE PipelineType,
		const PipelineResourceLayoutDesc& ResourceLayout,
		std::shared_ptr<const ShaderResource> shaderResource,
		const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
		UINT32 allowedTypeNum,
		ShaderResourceCache* resourceCache,
		RootSignature* rootSignature)

	{
		m_D3D12Device = pd3d12Device;
		m_ShaderResources = shaderResource;

		const UINT32 AllowedTypeBits = GetAllowedTypeBits(allowedVarTypes, allowedTypeNum);

		UINT32 StaticResCacheTblSizes[4] = { 0, 0, 0, 0 };

		// 把ShaderResource中的每个资源都通过RootSignature确定RootIndex和OffsetFromTableStart，然后存储下来
		auto AddResource = [&](const ShaderResourceAttribs& Attribs,
			CachedResourceType              ResType,
			SHADER_RESOURCE_VARIABLE_TYPE   VarType,
			UINT32                          SamplerId = D3D12Resource::InvalidSamplerId) //
		{
			UINT32 RootIndex = D3D12Resource::InvalidRootIndex;
			UINT32 Offset = D3D12Resource::InvalidOffset;

			D3D12_DESCRIPTOR_RANGE_TYPE DescriptorRangeType = GetDescriptorRangeType(ResType);

			if (rootSignature)
			{
				// 按照ShaderResource中的顺序添加到RootSignature中，并分配RootIndex和Offset
				rootSignature->AllocateResourceSlot(m_ShaderResources->GetShaderType(), PipelineType, Attribs, VarType, DescriptorRangeType, RootIndex, Offset);
			}
			else
			{
				// Static资源按下列RootIndex存放
				// SRVs at root index D3D12_DESCRIPTOR_RANGE_TYPE_SRV (0)
				// UAVs at root index D3D12_DESCRIPTOR_RANGE_TYPE_UAV (1)
				// CBVs at root index D3D12_DESCRIPTOR_RANGE_TYPE_CBV (2)
				// Samplers at root index D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER (3)

				RootIndex = DescriptorRangeType;
				Offset = Attribs.BindPoint;	// Offset就是寄存器的索引，所以最好把Static资源的声明放在前面
				StaticResCacheTblSizes[RootIndex] = std::max(StaticResCacheTblSizes[RootIndex], Offset + Attribs.BindCount);
			}

			if (ResType == CachedResourceType::Sampler)
			{

			}
			else
			{
				m_SrvCbvUavs[VarType].emplace_back(*this, Attribs, VarType, ResType, RootIndex, Offset, SamplerId);
			}
		};

		m_ShaderResources->ProcessResources(
			[&](const ShaderResourceAttribs& CB, UINT32)
			{
				auto VarType = m_ShaderResources->FindVariableType(CB, ResourceLayout);
				if (IsAllowedType(VarType, AllowedTypeBits))
					AddResource(CB, CachedResourceType::CBV, VarType);
			},
			[&](const ShaderResourceAttribs& Sampler, UINT32)
			{
			},
			[&](const ShaderResourceAttribs& TexSRV, UINT32)
			{
				auto VarType = m_ShaderResources->FindVariableType(TexSRV, ResourceLayout);
				if (IsAllowedType(VarType, AllowedTypeBits))
					AddResource(TexSRV, CachedResourceType::TexSRV, VarType);
			},
			[&](const ShaderResourceAttribs& TexUAV, UINT32)
			{
				auto VarType = m_ShaderResources->FindVariableType(TexUAV, ResourceLayout);
				if (IsAllowedType(VarType, AllowedTypeBits))
					AddResource(TexUAV, CachedResourceType::TexUAV, VarType);
			},
			[&](const ShaderResourceAttribs& BufSRV, UINT32)
			{
				auto VarType = m_ShaderResources->FindVariableType(BufSRV, ResourceLayout);
				if (IsAllowedType(VarType, AllowedTypeBits))
					AddResource(BufSRV, CachedResourceType::BufSRV, VarType);
			},
			[&](const ShaderResourceAttribs& BufUAV, UINT32)
			{
				auto VarType = m_ShaderResources->FindVariableType(BufUAV, ResourceLayout);
				if (IsAllowedType(VarType, AllowedTypeBits))
					AddResource(BufUAV, CachedResourceType::BufUAV, VarType);
			}
		);

		// 初始化Static Resource Cache
		if (resourceCache)
		{
			resourceCache->Initialize( _countof(StaticResCacheTblSizes), StaticResCacheTblSizes);
		}
	}

	void ShaderResourceLayout::D3D12Resource::CacheCB(IDeviceObject* pBuffer, 
													  ShaderResourceCache::Resource& dstRes, 
													  UINT32 arrayIndex, 
													  D3D12_CPU_DESCRIPTOR_HANDLE shaderVisibleHeapCPUDescriptorHandle) const
	{
		Buffer* buffer = dynamic_cast<Buffer*>(pBuffer);

		if (buffer)
		{
			if (VariableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC && dstRes.pObject != nullptr)
			{
				// 如果已经绑定了资源就不更新，除非是Dynamic资源
				return;
			}

			dstRes.Type = ResourceType;
			dstRes.CPUDescriptorHandle = buffer->GetCBVHandle();

			if (shaderVisibleHeapCPUDescriptorHandle.ptr != 0)
			{
				ID3D12Device* d3d12Device = ParentResLayout.m_D3D12Device;
				d3d12Device->CopyDescriptorsSimple(1, shaderVisibleHeapCPUDescriptorHandle, dstRes.CPUDescriptorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}

			dstRes.pObject = buffer;
		}
	}

	// 该模板函数可以在cpp中定义，因为只在本文件中使用
	template<typename TResourceViewType, typename TViewTypeEnum>
	void ShaderResourceLayout::D3D12Resource::CacheResourceView(IDeviceObject* pView, 
																ShaderResourceCache::Resource& dstRes, 
																UINT32 arrayIndex, 
																D3D12_CPU_DESCRIPTOR_HANDLE shaderVisibleHeapCPUDescriptorHandle, 
																TViewTypeEnum expectedViewType) const
	{
		TResourceViewType* resourceView = dynamic_cast<TResourceViewType*>(pView);

		if (resourceView)
		{
			if (VariableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC && dstRes.pObject != nullptr)
			{
				// 如果已经绑定了资源就不更新，除非是Dynamic资源
				return;
			}

			dstRes.Type = ResourceType;
			dstRes.CPUDescriptorHandle = resourceView->GetCPUDescriptorHandle();

			if (shaderVisibleHeapCPUDescriptorHandle.ptr != 0)
			{
				ID3D12Device* d3d12Device = ParentResLayout.m_D3D12Device;
				d3d12Device->CopyDescriptorsSimple(1, shaderVisibleHeapCPUDescriptorHandle, dstRes.CPUDescriptorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}

			dstRes.pObject = resourceView;
		}
	}

	bool ShaderResourceLayout::D3D12Resource::IsBound(UINT32 arrayIndex, const ShaderResourceCache& resourceCache) const
	{
		if (RootIndex < resourceCache.GetRootTableNum())
		{
			// ShaderResourceCache::RootTable
			const ShaderResourceCache::RootTable& rootTable = resourceCache.GetRootTable(RootIndex);
			if (OffsetFromTableStart + arrayIndex < rootTable.GetSize())
			{
				// ShaderResourceCache::Resource
				const ShaderResourceCache::Resource& CachedRes =
					rootTable.GetResource(OffsetFromTableStart + ArrayIndex,
						GetResType() == CachedResourceType::Sampler ? D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
						ParentResLayout.m_ShaderResources->GetShaderType());

				if (CachedRes.pObject != nullptr)
					return true;
			}
		}

		return false;
	}

	void ShaderResourceLayout::D3D12Resource::BindResource(IDeviceObject* pObject, UINT32 arrayIndex, ShaderResourceCache& resourceCache) const
	{
		const bool IsSampler = GetResType() == CachedResourceType::Sampler;
		auto       DescriptorHeapType = IsSampler ? D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		auto& DstRes = resourceCache.GetRootTable(RootIndex).GetResource(OffsetFromTableStart + arrayIndex, 
																		 DescriptorHeapType, ParentResLayout.m_ShaderResources->GetShaderType());

		auto ShdrVisibleHeapCPUDescriptorHandle = IsSampler ?
			resourceCache.GetShaderVisibleTableCPUDescriptorHandle<D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER>(RootIndex, OffsetFromTableStart + arrayIndex) :
			resourceCache.GetShaderVisibleTableCPUDescriptorHandle<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>(RootIndex, OffsetFromTableStart + arrayIndex);

		if (pObject)
		{
			switch (ResourceType)
			{
			case CachedResourceType::CBV:
				CacheCB(pObject, DstRes, arrayIndex, ShdrVisibleHeapCPUDescriptorHandle);
				break;

			case CachedResourceType::TexSRV:
				CacheResourceView<TextureView>(pObject, DstRes, arrayIndex, ShdrVisibleHeapCPUDescriptorHandle, TEXTURE_VIEW_SHADER_RESOURCE);
				break;

			case CachedResourceType::TexUAV:
				CacheResourceView<TextureView>(pObject, DstRes, arrayIndex, ShdrVisibleHeapCPUDescriptorHandle, TEXTURE_VIEW_UNORDERED_ACCESS);
				break;

			case CachedResourceType::BufSRV:
				CacheResourceView<BufferView>(pObject, DstRes, arrayIndex, ShdrVisibleHeapCPUDescriptorHandle, BUFFER_VIEW_SHADER_RESOURCE);
				break;

			case CachedResourceType::BufUAV:
				CacheResourceView<BufferView>(pObject, DstRes, arrayIndex, ShdrVisibleHeapCPUDescriptorHandle, BUFFER_VIEW_UNORDERED_ACCESS);
				break;

			case CachedResourceType::Sampler:
				CacheSampler(pObject, DstRes, arrayIndex, ShdrVisibleHeapCPUDescriptorHandle);
				break;

			default:
				LOG_ERROR("Unkown CachedResourceType.");
				break;
			}
		}
	}

}