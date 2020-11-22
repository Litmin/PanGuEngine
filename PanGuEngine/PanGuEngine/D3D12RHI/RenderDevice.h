#pragma once
#include "DescriptorHeap.h"
#include "Buffer.h"
#include "Texture.h"

namespace RHI
{
	class RenderDevice
	{
	public:
		RenderDevice();
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

		// 安全释放GPU对象，把要释放的对象添加到队列中，当GPU不再使用这个对象时才释放它
		template <typename ObjectType>
		void SafeReleaseDeviceObject(ObjectType&& object)
		{

		}

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
	};
}


