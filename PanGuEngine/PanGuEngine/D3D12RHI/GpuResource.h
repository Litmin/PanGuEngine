#pragma once

namespace RHI 
{
    class RenderDevice;

    /**
    * GPU��������Դ�Ļ���
    * GPU��Դ������
    *   GpuBuffer��StructuredBuffer��ByteAddressBuffer��IndirectArgsBuffer��ReadbackBuffer��TypedBuffer
    *   PixelBuffer��ColorBuffer��DepthBuffer
    *   UploadBuffer
    *   Texture��
    * GpuResource�̳�enable_shared_from_this����Ϊ���ڲ��ᴴ��GpuResourceView��GpuResourceView��ӵ��GpuResource������Ȩ������֤ʹ��GpuResourceViewʱ��GpuResource�����ͷ�
    */
    class GpuResource : public std::enable_shared_from_this<GpuResource>
    {
    public:

        virtual ~GpuResource();

		ID3D12Resource* GetResource() { return m_pResource.Get(); }
		const ID3D12Resource* GetResource() const { return m_pResource.Get(); }

    protected:

		Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
		D3D12_RESOURCE_STATES m_UsageState;
		D3D12_RESOURCE_STATES m_TransitioningState;
    };

}