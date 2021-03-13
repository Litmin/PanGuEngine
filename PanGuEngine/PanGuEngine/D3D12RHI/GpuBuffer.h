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

        // ����һ��Buffer������ṩ�˳�ʼ���ݾͻ�������ϴ���Upload���У�Ȼ��Copy��Buffer
        GpuBuffer(UINT32 NumElements, UINT32 ElementSize, const void* initialData);

        GpuBuffer(UINT32 NumElements, UINT32 ElementSize, const UploadBuffer& srcData, UINT32 srcOffset);

        // Descriptor
		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }

        D3D12_VERTEX_BUFFER_VIEW CreateVBV(size_t Offset, uint32_t Size, uint32_t Stride) const;
        D3D12_VERTEX_BUFFER_VIEW CreateVBV(size_t BaseVertexIndex = 0) const;

        D3D12_INDEX_BUFFER_VIEW CreateIBV(size_t Offset, uint32_t Size, bool b32Bit = false) const;
        D3D12_INDEX_BUFFER_VIEW CreateIBV(size_t StartIndex = 0) const;


        virtual std::shared_ptr<GpuResourceDescriptor> CreateSRV() = 0;
        virtual std::shared_ptr<GpuResourceDescriptor> CreateUAV() = 0;

        UINT64 GetBufferSize() const { return m_BufferSize; }
        UINT32 GetElementCount() const { return m_ElementCount; }
        UINT32 GetElementSize() const { return m_ElementSize; }

    protected:
        
		D3D12_RESOURCE_DESC DescribeBuffer();


        // Buffer��GPU��ַ
		D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;

		UINT64 m_BufferSize;
		UINT32 m_ElementCount;
		UINT32 m_ElementSize;
    };

    class GpuStructuredBuffer : public GpuBuffer
    {
    public:

        GpuStructuredBuffer(UINT32 NumElements, UINT32 ElementSize, const void* initialData) :
            GpuBuffer(NumElements, ElementSize, initialData)
        {

        }

        GpuStructuredBuffer(UINT32 NumElements, UINT32 ElementSize, const UploadBuffer& srcData, UINT32 srcOffset) :
            GpuBuffer(NumElements, ElementSize, srcData, srcOffset)
        {

        }

		virtual std::shared_ptr<GpuResourceDescriptor> CreateSRV() override;
		virtual std::shared_ptr<GpuResourceDescriptor> CreateUAV() override;

    };

}