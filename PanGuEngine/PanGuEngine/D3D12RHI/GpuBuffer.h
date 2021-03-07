#pragma once

#include "GpuResource.h"


namespace RHI 
{
    class UploadBuffer;
    class GpuResourceDescriptor;

    /**
    * 
    */
    class GpuBuffer : public GpuResource
    {
    public:

        // 创建一个Buffer，如果提供了初始数据就会把数据上传到Upload堆中，然后Copy到Buffer
        GpuBuffer(const std::wstring& name, UINT32 NumElements, UINT32 ElementSize, const void* initialData);

        GpuBuffer(const std::wstring& name, UINT32 NumElements, UINT32 ElementSize, const UploadBuffer& srcData, UINT32 srcOffset);

        // Descriptor
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }

        D3D12_VERTEX_BUFFER_VIEW CreateVBV(size_t Offset, uint32_t Size, uint32_t Stride) const;
        D3D12_VERTEX_BUFFER_VIEW CreateVBV(size_t BaseVertexIndex = 0) const;

        D3D12_INDEX_BUFFER_VIEW CreateIBV(size_t Offset, uint32_t Size, bool b32Bit = false) const;
        D3D12_INDEX_BUFFER_VIEW CreateIBV(size_t StartIndex = 0) const;


        virtual GpuResourceDescriptor CreateSRV() = 0;
        virtual GpuResourceDescriptor CreateUAV() = 0;

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

    class GpuStructuredBuffer : public GpuBuffer
    {
    public:

        GpuStructuredBuffer(const std::wstring& name, UINT32 NumElements, UINT32 ElementSize, const void* initialData) :
            GpuBuffer(name, NumElements, ElementSize, initialData)
        {

        }

        GpuStructuredBuffer(const std::wstring& name, UINT32 NumElements, UINT32 ElementSize, const UploadBuffer& srcData, UINT32 srcOffset) :
            GpuBuffer(name, NumElements, ElementSize, srcData, srcOffset)
        {

        }

		virtual GpuResourceDescriptor CreateSRV() override;
		virtual GpuResourceDescriptor CreateUAV() override;

    };

}