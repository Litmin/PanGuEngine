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

		// 创建资源
		// 提供数据创建Buffer
		std::unique_ptr<Buffer> CreateBuffer(const BufferDesc& bufferDesc,
											 const BufferData* bufferData);
		// 用已有的D3D12资源创建Buffer
		std::unique_ptr<Buffer> CreateBufferFromD3DResource(ID3D12Resource* buffer,
															const BufferDesc& bufferDesc,
															D3D12_RESOURCE_STATES initialState);
		// 提供数据创建Texture
		std::unique_ptr<Texture> CreateTexture(const TextureDesc& texDesc,
											   const TextureData* texData);
		// 用已有的D3D12资源创建Texture
		std::unique_ptr<Texture> CreateTextureFromD3DResource(ID3D12Resource* texture,
															  const TextureDesc& texDesc,
															  D3D12_RESOURCE_STATES initialState);

		std::unique_ptr<Sampler> CreateSampler(const D3D12_SAMPLER_DESC& samplerDesc);

		// 分配Descriptor
		DescriptorHeapAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);
		DescriptorHeapAllocation AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);

		// 安全释放GPU对象，当GPU不再使用这个对象时才真正释放它
		template <typename DeviceObjectType>
		void SafeReleaseDeviceObject(DeviceObjectType&& object);
		void PurgeReleaseQueue(bool forceRelease);

		ID3D12Device* GetD3D12Device() { return m_D3D12Device.Get(); }

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

		// TODO:实现多个CommandQueue
		CommandQueue m_CommandQueue;

		// 负责释放资源的队列
		// 调用SafeReleaseDeviceObject释放资源时，会把该资源添加到m_StaleResources中，
		// 当提交一个CommandList时，会把下一个CommandList的编号和m_StaleResources中的资源添加到m_ReleaseQueue中，
		// 在每帧的末尾，调用PurgeReleaseQueue来释放可以安全释放的资源（也就是记录的Cmd编号比GPU已经完成的CmdList数量小的所有资源）
		// 使用unique_ptr引用资源，释放该资源时会弹出队列，自动调用资源的析构函数来释放
		std::deque<std::unique_ptr<D3D12DeviceObject>> m_ReleaseQueue;
		std::deque<std::unique_ptr<D3D12DeviceObject>> m_StaleResources;
	};

	template<typename DeviceObjectType>
	inline void RenderDevice::SafeReleaseDeviceObject(DeviceObjectType&& object)
	{

	}
}


