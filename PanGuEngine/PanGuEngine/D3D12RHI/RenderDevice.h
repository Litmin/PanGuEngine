#pragma once
#include "DescriptorHeap.h"

namespace RHI
{
	class RenderDevice
	{
	public:
		ID3D12Device* GetD3D12Device() { return m_D3D12Device.Get(); }

		// ����Descriptor
		DescriptorHeapAllocation AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);
		DescriptorHeapAllocation AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE Type, UINT Count = 1);

		// ��ȫ�ͷ�GPU���󣬰�Ҫ�ͷŵĶ�����ӵ������У���GPU����ʹ���������ʱ���ͷ���
		template <typename ObjectType>
		void SafeReleaseDeviceObject(ObjectType&& object)
		{

		}

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
	};
}


