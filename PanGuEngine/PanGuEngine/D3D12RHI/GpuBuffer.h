#pragma once

#include "GpuResource.h"


namespace RHI 
{
    class GpuUploadBuffer;
    class GpuResourceDescriptor;

    /**
    * Default: GPU读写  
    * Upload: CPU写 GPU读 
    * Dynamic: 动态Buffer，CPU写，GPU读
    */
    class GpuBuffer : public GpuResource
    {
    public:

        // 动态资源使用，构造函数中不会创建资源
        GpuBuffer(UINT32 NumElements, UINT32 ElementSize, D3D12_RESOURCE_STATES initialState, D3D12_HEAP_TYPE heapType);

        // Descriptor
		virtual D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }

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
        void CreateBufferResource(const void* initialData);
        void CreateBufferResource(const GpuUploadBuffer& srcData, UINT32 srcOffset);
        
		D3D12_RESOURCE_DESC DescribeBuffer();


        // Buffer的GPU地址
		D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
        D3D12_HEAP_TYPE m_HeapType;

		UINT64 m_BufferSize;
		UINT32 m_ElementCount;
		UINT32 m_ElementSize;
    };

    class GpuDefaultBuffer : public GpuBuffer
    {
    public:

        GpuDefaultBuffer(UINT32 NumElements, UINT32 ElementSize, const void* initialData) :
            GpuBuffer(NumElements, ElementSize, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT)
        {
            CreateBufferResource(initialData);
        }

        GpuDefaultBuffer(UINT32 NumElements, UINT32 ElementSize, const GpuUploadBuffer& srcData, UINT32 srcOffset) :
            GpuBuffer(NumElements, ElementSize, D3D12_RESOURCE_STATE_COMMON, D3D12_HEAP_TYPE_DEFAULT)
        {
            CreateBufferResource(srcData, srcOffset);
        }
    };

	class GpuUploadBuffer : public GpuBuffer
	{
	public:
        GpuUploadBuffer(UINT32 NumElements, UINT32 ElementSize) :
            GpuBuffer(NumElements, ElementSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD)
        {
            CreateBufferResource(nullptr);
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

    class GpuDynamicBuffer : public GpuBuffer
    {
    public:
        GpuDynamicBuffer(UINT32 NumElements, UINT32 ElementSize) :
            GpuBuffer(NumElements, ElementSize, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_HEAP_TYPE_UPLOAD)
        {

        }

        virtual D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const override;

        void* Map();
        void Unmap(size_t begin = 0, size_t end = -1);
        
    protected:

    };

}