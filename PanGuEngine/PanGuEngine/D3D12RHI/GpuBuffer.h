#pragma once

#include "GpuResource.h"


namespace RHI 
{
    class UploadBuffer;
    class GpuResourceDescriptor;

    /**
    * Default: GPU读写  Upload: CPU写 GPU读
    */
    class GpuBuffer : public GpuResource
    {
    public:

        // 创建一个Buffer，如果提供了初始数据就会把数据上传到Upload堆中，然后Copy到Buffer
        GpuBuffer(UINT32 NumElements, UINT32 ElementSize, const void* initialData, 
                  D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState);

        GpuBuffer(UINT32 NumElements, UINT32 ElementSize, const UploadBuffer& srcData, UINT32 srcOffset, 
                  D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState);

        // Descriptor
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }

        D3D12_VERTEX_BUFFER_VIEW CreateVBV(size_t Offset, uint32_t Size, uint32_t Stride) const;
        D3D12_VERTEX_BUFFER_VIEW CreateVBV(size_t BaseVertexIndex = 0) const;

        D3D12_INDEX_BUFFER_VIEW CreateIBV(size_t Offset, uint32_t Size, bool b32Bit = false) const;
        D3D12_INDEX_BUFFER_VIEW CreateIBV(size_t StartIndex = 0) const;


        virtual std::shared_ptr<GpuResourceDescriptor> CreateSRV();
        virtual std::shared_ptr<GpuResourceDescriptor> CreateUAV();

        UINT64 GetBufferSize() const { return m_BufferSize; }
        UINT32 GetElementCount() const { return m_ElementCount; }
        UINT32 GetElementSize() const { return m_ElementSize; }

    protected:
        
		D3D12_RESOURCE_DESC DescribeBuffer();


        // Buffer的GPU地址
		D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;

		UINT64 m_BufferSize;
		UINT32 m_ElementCount;
		UINT32 m_ElementSize;
    };

    class GpuDefaultBuffer : public GpuBuffer
    {
    public:

        GpuDefaultBuffer(UINT32 NumElements, UINT32 ElementSize, const void* initialData) :
            GpuBuffer(NumElements, ElementSize, initialData, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON)
        {

        }

        GpuDefaultBuffer(UINT32 NumElements, UINT32 ElementSize, const UploadBuffer& srcData, UINT32 srcOffset) :
            GpuBuffer(NumElements, ElementSize, srcData, srcOffset, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COMMON)
        {

        }
    };

	class UploadBuffer : public GpuBuffer
	{
	public:
        UploadBuffer(UINT32 NumElements, UINT32 ElementSize) :
            GpuBuffer(NumElements, ElementSize, nullptr, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ)
        {
        }

        void* Map(void)
        {
			void* Memory;
			m_pResource->Map(0, &CD3DX12_RANGE(0, m_BufferSize), &Memory);
			return Memory;
        }

        void Unmap(size_t begin = 0, size_t end = -1)
        {
			m_pResource->Unmap(0, &CD3DX12_RANGE(begin, std::min(end, m_BufferSize)));
        }
	};

}