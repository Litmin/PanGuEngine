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

		// 创建资源


		// 在CPU Descriptor Heap中分配资源的Descriptor
		DescriptorHeapAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);
		// 把资源绑定到Shader时，在GPU Descriptor Heap中分配Descriptor
		DescriptorHeapAllocation AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);

		// 安全释放GPU对象，当GPU不再使用这个对象时才真正释放它，传入的对象必须实现移动操作
		template <typename DeviceObjectType>
		void SafeReleaseDeviceObject(DeviceObjectType&& object);

		void PurgeReleaseQueue(bool forceRelease);

		ID3D12Device* GetD3D12Device() { return m_D3D12Device.Get(); }

		GPUDescriptorHeap& GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type);

	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_D3D12Device;

		// Descriptor Heap
		// 四个CPUDescriptorHeap对象，对应DX12的四种Descriptor Heap类型:CBV_SRV_UAV,Sampler,RTV,DSV
		// 每个对象可能有多个DX12 Descriptor Heap
		CPUDescriptorHeap m_CPUDescriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
		// 两个GPUDescriptorHeap对象，因为只有CBV_SRV_UAV,Sampler可以对GPU可见，
		// 每个GPUDescriptorHeap对象只会创建一个DX12 Descriptor Heap，因为切换Descriptor Heap非常昂贵
		// 创建资源时，每个资源的Descriptor保存在CPUDescriptorHeap中，在绘制命令执行前，会拷贝到GPUDescriptorHeap
		GPUDescriptorHeap m_GPUDescriptorHeaps[2];


		// 负责释放资源的队列
		// 调用SafeReleaseDeviceObject释放资源时，会把该资源添加到m_StaleResources中，
		// 当提交一个CommandList时，会把下一个CommandList的编号和m_StaleResources中的资源添加到m_ReleaseQueue中，
		// 在每帧的末尾，调用PurgeReleaseQueue来释放可以安全释放的资源（也就是记录的Cmd编号比GPU已经完成的CmdList数量小的所有资源）
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


