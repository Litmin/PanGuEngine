#pragma once

namespace RHI 
{
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
		GpuResource() :
			m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
			m_UsageState(D3D12_RESOURCE_STATE_COMMON),
			m_TransitioningState((D3D12_RESOURCE_STATES)-1)
		{
		}

		GpuResource(ID3D12Resource* pResource, D3D12_RESOURCE_STATES CurrentState) :
			m_GpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
			m_pResource(pResource),
			m_UsageState(CurrentState),
			m_TransitioningState((D3D12_RESOURCE_STATES)-1)
		{
		}

		virtual ~GpuResource() 
		{ 
			m_pResource = nullptr;
			m_GpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		}

		ID3D12Resource* GetResource() { return m_pResource.Get(); }
		const ID3D12Resource* GetResource() const { return m_pResource.Get(); }

		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return m_GpuVirtualAddress; }

    protected:

		Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
		D3D12_RESOURCE_STATES m_UsageState;
		D3D12_RESOURCE_STATES m_TransitioningState;
		D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;
    };

}