#pragma once
#include "DescriptorHeap.h"
#include "Buffer.h"
#include "Texture.h"
#include "CommandQueue.h"
#include "StaleResourceWrapper.h"

namespace RHI
{
	class RenderDevice : public Singleton<RenderDevice>
	{
	public:
		RenderDevice(Microsoft::WRL::ComPtr<ID3D12Device> d3d12Device);
		~RenderDevice();

		// ������Դ


		// ����Descriptor
		DescriptorHeapAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);
		DescriptorHeapAllocation AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);

		// ��ȫ�ͷ�GPU���󣬵�GPU����ʹ���������ʱ�������ͷ���������Ķ������ʵ���ƶ�����
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


		// �����ͷ���Դ�Ķ���
		// ����SafeReleaseDeviceObject�ͷ���Դʱ����Ѹ���Դ��ӵ�m_StaleResources�У�
		// ���ύһ��CommandListʱ�������һ��CommandList�ı�ź�m_StaleResources�е���Դ��ӵ�m_ReleaseQueue�У�
		// ��ÿ֡��ĩβ������PurgeReleaseQueue���ͷſ��԰�ȫ�ͷŵ���Դ��Ҳ���Ǽ�¼��Cmd��ű�GPU�Ѿ���ɵ�CmdList����С��������Դ��
		using ReleaseQueueElementType = std::pair<UINT64, StaleResourceWrapper>;
		std::deque<ReleaseQueueElementType> m_ReleaseQueue;
		std::deque<ReleaseQueueElementType> m_StaleResources;

		UINT64 m_NextCmdListNum;
	};

	template<typename DeviceObjectType>
	inline void RenderDevice::SafeReleaseDeviceObject(DeviceObjectType&& object)
	{
		auto wrapper = StaleResourceWrapper::Create(object);
		m_StaleResources.emplace_back(m_NextCmdListNum, std::move(wrapper));
	}
}


