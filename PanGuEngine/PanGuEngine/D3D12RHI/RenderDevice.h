#pragma once
#include "DescriptorHeap.h"
#include "Buffer.h"
#include "Texture.h"
#include "CommandQueue.h"

namespace RHI
{
	class RenderDevice
	{
	public:
		RenderDevice(ComPtr<ID3D12Device> d3d12Device);
		~RenderDevice();

		// ������Դ
		// �ṩ���ݴ���Buffer
		std::unique_ptr<Buffer> CreateBuffer(const BufferDesc& bufferDesc,
											 const BufferData* bufferData);
		// �����е�D3D12��Դ����Buffer
		std::unique_ptr<Buffer> CreateBufferFromD3DResource(ID3D12Resource* buffer,
															const BufferDesc& bufferDesc,
															D3D12_RESOURCE_STATES initialState);
		// �ṩ���ݴ���Texture
		std::unique_ptr<Texture> CreateTexture(const TextureDesc& texDesc,
											   const TextureData* texData);
		// �����е�D3D12��Դ����Texture
		std::unique_ptr<Texture> CreateTextureFromD3DResource(ID3D12Resource* texture,
															  const TextureDesc& texDesc,
															  D3D12_RESOURCE_STATES initialState);

		std::unique_ptr<Sampler> CreateSampler(const D3D12_SAMPLER_DESC& samplerDesc);

		// ����Descriptor
		DescriptorHeapAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);
		DescriptorHeapAllocation AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);

		// ��ȫ�ͷ�GPU���󣬵�GPU����ʹ���������ʱ�������ͷ���
		template <typename DeviceObjectType>
		void SafeReleaseDeviceObject(DeviceObjectType&& object);
		void PurgeReleaseQueue(bool forceRelease);

		ID3D12Device* GetD3D12Device() { return m_D3D12Device.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_D3D12Device;

		// Descriptor Heap
		// �ĸ�CPUDescriptorHeap���󣬶�ӦDX12������Descriptor Heap����:CBV_SRV_UAV,Sampler,RTV,DSV
		// ÿ����������ж��DX12 Descriptor Heap
		CPUDescriptorHeap m_CPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		// ����GPUDescriptorHeap������Ϊֻ��CBV_SRV_UAV,Sampler���Զ�GPU�ɼ���
		// ÿ��GPUDescriptorHeap����ֻ�ᴴ��һ��DX12 Descriptor Heap����Ϊ�л�Descriptor Heap�ǳ�����
		// ������Դʱ��ÿ����Դ��Descriptor������CPUDescriptorHeap�У��ڻ�������ִ��ǰ���´����GPUDescriptorHeap
		GPUDescriptorHeap m_GPUDescriptorHeaps[2];

		// TODO:ʵ�ֶ��CommandQueue
		CommandQueue m_CommandQueue;

		// �����ͷ���Դ�Ķ���
		// ����SafeReleaseDeviceObject�ͷ���Դʱ����Ѹ���Դ��ӵ�m_StaleResources�У�
		// ���ύһ��CommandListʱ�������һ��CommandList�ı�ź�m_StaleResources�е���Դ��ӵ�m_ReleaseQueue�У�
		// ��ÿ֡��ĩβ������PurgeReleaseQueue���ͷſ��԰�ȫ�ͷŵ���Դ��Ҳ���Ǽ�¼��Cmd��ű�GPU�Ѿ���ɵ�CmdList����С��������Դ��
		// ʹ��unique_ptr������Դ���ͷŸ���Դʱ�ᵯ�����У��Զ�������Դ�������������ͷ�
		std::deque<std::unique_ptr<D3D12DeviceObject>> m_ReleaseQueue;
		std::deque<std::unique_ptr<D3D12DeviceObject>> m_StaleResources;
	};

	template<typename DeviceObjectType>
	inline void RenderDevice::SafeReleaseDeviceObject(DeviceObjectType&& object)
	{

	}
}


