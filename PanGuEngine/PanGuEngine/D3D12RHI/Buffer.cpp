#include "pch.h"
#include "Buffer.h"
#include "RenderDevice.h"

using namespace Microsoft::WRL;

namespace RHI
{
	void ValidateBufferInitData(const BufferDesc& Desc, const BufferData* pBuffData)
	{
		// Immutable的Buffer必须在创建时初始化
		if (Desc.Usage == USAGE_IMMUTABLE && (pBuffData == nullptr || pBuffData->pData == nullptr))
			LOG_ERROR("initial data must not be null as static buffers must be initialized at creation time.");

		// Dynamic的Buffer的初始数据必须为空
		if (Desc.Usage == USAGE_DYNAMIC && pBuffData != nullptr && pBuffData->pData != nullptr)
			LOG_ERROR("initial data must be null for dynamic buffers.");

		if (Desc.Usage == USAGE_STAGING)
		{
			if (Desc.CPUAccessFlags == CPU_ACCESS_WRITE)
			{
				assert((pBuffData == nullptr || pBuffData->pData == nullptr) &&
					"CPU-writable staging buffers must be updated via map.");
			}
		}
	}

	Buffer::Buffer(RenderDevice* pRenderDevice, 
				   const BufferDesc& desc, 
				   const BufferData* pBufferData) :
		m_RenderDevice{pRenderDevice},
		m_Desc{desc}
	{
		ValidateBufferInitData(desc, pBufferData);

		UINT32 AlignmentMask = 1;
		// Constant Buffer是256字节对齐的
		if (m_Desc.BindFlags & BIND_CONSTANT_BUFFER)
			AlignmentMask = 255;

		if (m_Desc.Usage == USAGE::USAGE_STAGING)
		{
			if (m_Desc.CPUAccessFlags == CPU_ACCESS_WRITE)
				AlignmentMask = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1;
		}

		if (AlignmentMask != 1)
			m_Desc.SizeInBytes = (m_Desc.SizeInBytes + AlignmentMask) & (~AlignmentMask);

		// TODO:DynamicBuffer
		if (m_Desc.Usage == USAGE::USAGE_DYNAMIC && (m_Desc.BindFlags & (BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS)) == 0)
		{
			// Dynamic constant/vertex/index buffers are suballocated in the upload heap when Map() is called.
			// Dynamic buffers with SRV or UAV flags need to be allocated in GPU-only memory
			// Dynamic upload heap buffer is always in D3D12_RESOURCE_STATE_GENERIC_READ state

			SetState(D3D12_RESOURCE_STATE_GENERIC_READ);
		}
		else
		{
			D3D12_RESOURCE_DESC D3D12BuffDesc = {};
			D3D12BuffDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			D3D12BuffDesc.Alignment = 0;
			D3D12BuffDesc.Width = m_Desc.SizeInBytes;
			D3D12BuffDesc.Height = 1;
			D3D12BuffDesc.DepthOrArraySize = 1;
			D3D12BuffDesc.MipLevels = 1;
			D3D12BuffDesc.Format = DXGI_FORMAT_UNKNOWN;
			D3D12BuffDesc.SampleDesc.Count = 1;
			D3D12BuffDesc.SampleDesc.Quality = 0;
			// Layout must be D3D12_TEXTURE_LAYOUT_ROW_MAJOR, as buffer memory layouts are
			// understood by applications and row-major texture data is commonly marshaled through buffers.
			D3D12BuffDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			D3D12BuffDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
			if (m_Desc.BindFlags & BIND_UNORDERED_ACCESS)
				D3D12BuffDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
			if (!(m_Desc.BindFlags & BIND_SHADER_RESOURCE))
				D3D12BuffDesc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

			auto* pd3d12Device = m_RenderDevice->GetD3D12Device();

			D3D12_HEAP_PROPERTIES HeapProps;
			if (m_Desc.Usage == USAGE_STAGING)
				HeapProps.Type = m_Desc.CPUAccessFlags == CPU_ACCESS_READ ? D3D12_HEAP_TYPE_READBACK : D3D12_HEAP_TYPE_UPLOAD;
			else
				HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

			if (HeapProps.Type == D3D12_HEAP_TYPE_READBACK)
				SetState(D3D12_RESOURCE_STATE_COPY_DEST);
			else if (HeapProps.Type == D3D12_HEAP_TYPE_UPLOAD)
				SetState(D3D12_RESOURCE_STATE_GENERIC_READ);
			HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			HeapProps.CreationNodeMask = 1;
			HeapProps.VisibleNodeMask = 1;

			bool bInitializeBuffer = (pBufferData != nullptr && pBufferData->pData != nullptr && pBufferData->DataSize > 0);
			if (bInitializeBuffer)
				SetState(D3D12_RESOURCE_STATE_COPY_DEST);

			ThrowIfFailed(pd3d12Device->CreateCommittedResource(&HeapProps,
															D3D12_HEAP_FLAG_NONE,
															&D3D12BuffDesc,
															m_State,
															nullptr,
															IID_PPV_ARGS(&m_D3d12Resource)));
			// 设置Buffer名字，方便调试
			if (!m_Desc.Name.empty())
				m_D3d12Resource->SetName(m_Desc.Name.c_str());

			if (bInitializeBuffer)
			{
				D3D12_HEAP_PROPERTIES UploadHeapProps;
				UploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
				UploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				UploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				UploadHeapProps.CreationNodeMask = 1;
				UploadHeapProps.VisibleNodeMask = 1;

				D3D12BuffDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
				ComPtr<ID3D12Resource> UploadBuffer;
				ThrowIfFailed(pd3d12Device->CreateCommittedResource(&UploadHeapProps,
							  D3D12_HEAP_FLAG_NONE,
							  &D3D12BuffDesc,
							  D3D12_RESOURCE_STATE_GENERIC_READ,
							  nullptr,
							  IID_PPV_ARGS(&UploadBuffer)));

				void* DestAddress = nullptr;
				ThrowIfFailed(UploadBuffer->Map(0, nullptr, &DestAddress));
				memcpy(DestAddress, pBufferData->pData, pBufferData->DataSize);
				UploadBuffer->Unmap(0, nullptr);

				// TODO: Copy from uploadBuffer To Buffer。And release uploadBuffer。
			}
		}
	}

	Buffer::Buffer(RenderDevice* pRenderDevice, 
				   const BufferDesc& desc, 
				   D3D12_RESOURCE_STATES initialState, 
				   ID3D12Resource* pD3D12Buffer) :
		m_RenderDevice{pRenderDevice},
		m_Desc{desc}
	{
		m_D3d12Resource = pD3D12Buffer;
		SetState(initialState);
	}

	Buffer::~Buffer()
	{
		m_RenderDevice->SafeReleaseDeviceObject(std::move(m_D3d12Resource));
	}

	std::unique_ptr<BufferView> Buffer::CreateView(const BufferViewDesc& viewDesc)
	{
		// TODO:SRV、UAV
		assert(viewDesc.ViewType == BUFFER_VIEW_TYPE::BUFFER_VIEW_CONSTANT_BUFFER && "SRV and UAV not implemented");

		if (viewDesc.ViewType == BUFFER_VIEW_TYPE::BUFFER_VIEW_CONSTANT_BUFFER)
		{
			auto CBVDescriptorAllocation = m_RenderDevice->AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);

			D3D12_CONSTANT_BUFFER_VIEW_DESC D3D12_CBVDesc;
			D3D12_CBVDesc.BufferLocation = GetD3D12Resource()->GetGPUVirtualAddress();
			D3D12_CBVDesc.SizeInBytes = m_Desc.SizeInBytes;

			m_RenderDevice->GetD3D12Device()->CreateConstantBufferView(&D3D12_CBVDesc, CBVDescriptorAllocation.GetCpuHandle());

			return std::make_unique<BufferView>(m_RenderDevice, viewDesc, this, std::move(CBVDescriptorAllocation));
		}

		return nullptr;
	}

	BufferView* Buffer::GetDefaultView(BUFFER_VIEW_TYPE viewType)
	{
		switch (viewType)
		{
		case BUFFER_VIEW_TYPE::BUFFER_VIEW_CONSTANT_BUFFER:
			return m_DefaultCBV.get();
		case BUFFER_VIEW_TYPE::BUFFER_VIEW_SHADER_RESOURCE:
			return m_DefaultSRV.get();
		case BUFFER_VIEW_TYPE::BUFFER_VIEW_UNORDERED_ACCESS:
			return m_DefaultUAV.get();
		}

		return nullptr;
	}

	void Buffer::CreateDefaultViews()
	{
		if (m_Desc.BindFlags & BIND_CONSTANT_BUFFER)
		{
			BufferViewDesc ViewDesc = {};
			ViewDesc.ViewType = BUFFER_VIEW_CONSTANT_BUFFER;
			m_DefaultCBV = CreateView(ViewDesc);
		}
		// TODO:SRV、UAV

	}

	ID3D12Resource* Buffer::GetD3D12Buffer(UINT64& DataStartByteOffset, DeviceContext* pContext)
	{
		return GetD3D12Resource();
	}

	D3D12_GPU_VIRTUAL_ADDRESS Buffer::GetGPUAddress(UINT32 contextId, DeviceContext* pContext)
	{
		// TODO:Dynamic Buffer
		return GetD3D12Resource()->GetGPUVirtualAddress();
	}
}


