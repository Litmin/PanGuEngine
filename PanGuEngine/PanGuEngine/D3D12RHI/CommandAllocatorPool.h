#pragma once

namespace RHI 
{
	/*
	*    CommandAllocator�Ķ���أ�
	*    ���󣺼��Pool����ǰ���Allocator�������ǰGPU��������Allocator������Ϳ��Ը��ã��������´���һ��
	*    ���գ���Allocator�͵�ǰ��FenceValueһ����������
	*    ����CommandContextʱ�ᴴ��CommandList����ʱ���������һ��Allocator����Reset CommandContext��CommandListʱҲ���������һ��Allocator��
	*    ��CommandContext Finishʱ��Discard Allocator��Ҳ���������¼����ˡ�
	*/
    class CommandAllocatorPool
    {
    public:
		CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type, ID3D12Device* Device);

		ID3D12CommandAllocator* RequestAllocator(UINT64 CompletedFenceValue);
		void DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator);

    private:
		const D3D12_COMMAND_LIST_TYPE m_cCommandListType;

		ID3D12Device* m_Device;
		std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> m_Allocators;
		std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> m_ReadyAllocators;
    };
}