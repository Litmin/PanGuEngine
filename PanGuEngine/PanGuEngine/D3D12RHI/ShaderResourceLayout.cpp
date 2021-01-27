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

	// ���ֳ�ʼ������;�Ƕ�Ϊ���е���Դ     RootIndex��OffsetFromTableStart�ڳ�ʼ���Ĺ����з���
	void ShaderResourceLayout::InitializeForAll(ID3D12Device* pd3d12Device,
												PIPELINE_TYPE PipelineType,
												const PipelineResourceLayoutDesc& ResourceLayout,
												std::shared_ptr<const ShaderResource> shaderResource,
												RootSignature* rootSignature)

	{
		m_D3D12Device = pd3d12Device;
		m_ShaderResources = shaderResource;


		// ��ShaderResource�е�ÿ����Դ��ͨ��RootSignatureȷ��RootIndex��OffsetFromTableStart��Ȼ��洢����
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
				// ����ShaderResource�е�˳����ӵ�RootSignature�У�������RootIndex��Offset
				rootSignature->AllocateResourceSlot(m_ShaderResources->GetShaderType(), PipelineType, Attribs, VarType, DescriptorRangeType, RootIndex, Offset);
			}
			else
			{
				LOG_ERROR("No Rootsignature.");
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
				AddResource(CB, CachedResourceType::CBV, VarType);
			},
			[&](const ShaderResourceAttribs& Sampler, UINT32)
			{
			},
			[&](const ShaderResourceAttribs& TexSRV, UINT32)
			{
				auto VarType = m_ShaderResources->FindVariableType(TexSRV, ResourceLayout);
				AddResource(TexSRV, CachedResourceType::TexSRV, VarType);
			},
			[&](const ShaderResourceAttribs& TexUAV, UINT32)
			{
				auto VarType = m_ShaderResources->FindVariableType(TexUAV, ResourceLayout);
				AddResource(TexUAV, CachedResourceType::TexUAV, VarType);
			},
			[&](const ShaderResourceAttribs& BufSRV, UINT32)
			{
				auto VarType = m_ShaderResources->FindVariableType(BufSRV, ResourceLayout);
				AddResource(BufSRV, CachedResourceType::BufSRV, VarType);
			},
			[&](const ShaderResourceAttribs& BufUAV, UINT32)
			{
				auto VarType = m_ShaderResources->FindVariableType(BufUAV, ResourceLayout);
				AddResource(BufUAV, CachedResourceType::BufUAV, VarType);
			}
		);
	}

	// ���ַ�ʽ��ʼ������;�ǰ�������Static ��Դ
	void ShaderResourceLayout::InitializeForStatic(ID3D12Device* pd3d12Device, 
												   PIPELINE_TYPE pipelineType, 
												   const PipelineResourceLayoutDesc& resourceLayout, 
												   std::shared_ptr<const ShaderResource> shaderResource, 
												   const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes, 
												   UINT32 allowedTypeNum, 
												   ShaderResourceCache* resourceCache)
	{
		m_D3D12Device = pd3d12Device;
		m_ShaderResources = shaderResource;

		const UINT32 AllowedTypeBits = GetAllowedTypeBits(allowedVarTypes, allowedTypeNum);

		UINT32 StaticResCacheTblSizes[4] = { 0, 0, 0, 0 };

		// ��ShaderResource�е�ÿ����Դ��ͨ��RootSignatureȷ��RootIndex��OffsetFromTableStart��Ȼ��洢����
		auto AddResource = [&](const ShaderResourceAttribs& Attribs,
			CachedResourceType              ResType,
			SHADER_RESOURCE_VARIABLE_TYPE   VarType,
			UINT32                          SamplerId = D3D12Resource::InvalidSamplerId) //
		{
			UINT32 RootIndex = D3D12Resource::InvalidRootIndex;
			UINT32 Offset = D3D12Resource::InvalidOffset;

			D3D12_DESCRIPTOR_RANGE_TYPE DescriptorRangeType = GetDescriptorRangeType(ResType);

			// Static��Դ������RootIndex���
			// SRVs at root index D3D12_DESCRIPTOR_RANGE_TYPE_SRV (0)
			// UAVs at root index D3D12_DESCRIPTOR_RANGE_TYPE_UAV (1)
			// CBVs at root index D3D12_DESCRIPTOR_RANGE_TYPE_CBV (2)
			// Samplers at root index D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER (3)

			RootIndex = DescriptorRangeType;
			Offset = Attribs.BindPoint;	// Offset���ǼĴ�����������������ð�Static��Դ����������ǰ��
			StaticResCacheTblSizes[RootIndex] = std::max(StaticResCacheTblSizes[RootIndex], Offset + Attribs.BindCount);

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

		// ��ʼ��Static Resource Cache
		if (resourceCache)
		{
			resourceCache->Initialize(_countof(StaticResCacheTblSizes), StaticResCacheTblSizes);
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
				// ����Ѿ�������Դ�Ͳ����£�������Dynamic��Դ
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

	// ��ģ�庯��������cpp�ж��壬��Ϊֻ�ڱ��ļ���ʹ��
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
				// ����Ѿ�������Դ�Ͳ����£�������Dynamic��Դ
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
		if (RootIndex < resourceCache.GetRootTablesNum())
		{
			// ShaderResourceCache::RootTable
			const ShaderResourceCache::RootTable& rootTable = resourceCache.GetRootTable(RootIndex);
			if (OffsetFromTableStart + arrayIndex < rootTable.Size())
			{
				// ShaderResourceCache::Resource
				const ShaderResourceCache::Resource& CachedRes =
					rootTable.GetResource(OffsetFromTableStart + arrayIndex);

				if (CachedRes.pObject != nullptr)
					return true;
			}
		}

		return false;
	}

	// ����Դ������������
	// ����ֻ�ǰ���Դ��Descriptor��������Cache�У�û���ύ����Ⱦ����
	void ShaderResourceLayout::D3D12Resource::BindResource(IDeviceObject* pObject, UINT32 arrayIndex, ShaderResourceCache& resourceCache) const
	{
		const bool IsSampler = ResourceType == CachedResourceType::Sampler;
		auto       DescriptorHeapType = IsSampler ? D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER : D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		auto& DstRes = resourceCache.GetRootTable(RootIndex).GetResource(OffsetFromTableStart + arrayIndex);

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

	void ShaderResourceLayout::CopyStaticResourceDesriptorHandles(const ShaderResourceCache& SrcCache, 
																				 const ShaderResourceLayout& DstLayout, 
																				 ShaderResourceCache& DstCache) const
	{
		// Static shader resources������˳��洢:
		// CBVs at root index D3D12_DESCRIPTOR_RANGE_TYPE_CBV ��0��,
		// SRVs at root index D3D12_DESCRIPTOR_RANGE_TYPE_SRV ��1��,
		// UAVs at root index D3D12_DESCRIPTOR_RANGE_TYPE_UAV ��2��, and
		// Samplers at root index D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER ��3��
		// ÿ����Դ��Table�е�Offset����BindPoint��Ҳ���ǼĴ������

		const auto& CbvSrvUavCount = DstLayout.GetCbvSrvUavCount(SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
		for (UINT32 i = 0; i < CbvSrvUavCount; ++i)
		{
			const auto& res = DstLayout.GetSrvCbvUav(SHADER_RESOURCE_VARIABLE_TYPE_STATIC, i);
			auto        RangeType = GetDescriptorRangeType(res.ResourceType);

			for (UINT32 arrayInd = 0; arrayInd < res.Attribs.BindCount; ++arrayInd)
			{
				auto BindPoint = res.Attribs.BindPoint + arrayInd;

				// GetҪ��������Դ������ͨ��RangeType���ҵ�RootTable��Ȼ����BindPoint��BindPoint + �������е��������ҵ���Ӧ����Դ
				const auto& SrcRes = SrcCache.GetRootTable(RangeType).GetResource(BindPoint);

				if (!SrcRes.pObject)
					LOG_ERROR("No resource is assigned to static shader variable");

				// GetҪ��������Ŀ����Դ����Ϊ���DstCache���Ǵ洢Static��Դ�ģ�����RootTable�ǰ���RootIndex���еģ�Table�е���Դ�ǰ���Offset���е�
				auto& DstRes = DstCache.GetRootTable(res.RootIndex).GetResource(res.OffsetFromTableStart + arrayInd);

				if (DstRes.pObject != SrcRes.pObject)
				{
					assert(DstRes.pObject == nullptr && "Static��Դ�Ѿ�����ʼ����, ����Shader�е�Static��Դ��֮ǰ��ʼ���Ĳ�һ��");

					// Copy
					DstRes.pObject = SrcRes.pObject;
					DstRes.Type = SrcRes.Type;
					DstRes.CPUDescriptorHandle = SrcRes.CPUDescriptorHandle;
					// Get DstCache��GPUDescriptorHeap��Handle����Descriptor������ȥ
					auto ShdrVisibleHeapCPUDescriptorHandle = DstCache.GetShaderVisibleTableCPUDescriptorHandle<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>(res.RootIndex, res.OffsetFromTableStart + arrayInd);
					if (ShdrVisibleHeapCPUDescriptorHandle.ptr != 0)
					{
						m_D3D12Device->CopyDescriptorsSimple(1, ShdrVisibleHeapCPUDescriptorHandle, SrcRes.CPUDescriptorHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
					}
				}
				else
				{
					assert(DstRes.pObject == SrcRes.pObject);
					assert(DstRes.Type == SrcRes.Type);
					assert(DstRes.CPUDescriptorHandle.ptr == SrcRes.CPUDescriptorHandle.ptr);
				}
			}
		}

		// TODO: Sampler
	}

}