#pragma once

namespace RHI 
{
    class RenderDevice;

    /**
    * GPU上所有资源的基类
    * GPU资源包括：
    *   GpuBuffer：StructuredBuffer、ByteAddressBuffer、IndirectArgsBuffer、ReadbackBuffer、TypedBuffer
    *   PixelBuffer：ColorBuffer、DepthBuffer
    *   UploadBuffer
    *   Texture：
    * GpuResource继承enable_shared_from_this，因为在内部会创建GpuResourceView，GpuResourceView会拥有GpuResource的所有权，来保证使用GpuResourceView时，GpuResource不被释放
    */
    class GpuResource : public std::enable_shared_from_this<GpuResource>
    {
    public:

        virtual ~GpuResource();

        void SetName(const std::wstring& name) { m_pResource->SetName(name.c_str()); }

		ID3D12Resource* GetResource() { return m_pResource.Get(); }
		const ID3D12Resource* GetResource() const { return m_pResource.Get(); }

    protected:

		Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
		D3D12_RESOURCE_STATES m_UsageState;
		D3D12_RESOURCE_STATES m_TransitioningState = (D3D12_RESOURCE_STATES)-1;
    };

}