#pragma once
#include "DescriptorHeap.h"

namespace RHI 
{
    /**
    * CommandList和CommandListAllocator的集合体
    */
    class CommandContext
    {
    public:

    private:
        Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_pCommandList;
        Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    m_pCurrentAllocator;

        ID3D12PipelineState* m_pCurPipelineState = nullptr;
        ID3D12RootSignature* m_pCurGraphicsRootSignature = nullptr;
        ID3D12RootSignature* m_pCurComputeRootSignature = nullptr;

        ID3D12DescriptorHeap* m_BoundSrvCbvUavHeap;

        static constexpr int MaxPendingBarriers = 16;
        std::vector<D3D12_RESOURCE_BARRIER> m_PendingResourceBarriers;

        DynamicSuballocationsManager* m_DynamicGPUDescriptorAllocators = nullptr;

        D3D12_PRIMITIVE_TOPOLOGY m_PrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

    };

    class GraphicsContext : public CommandContext
    {
    public:

    private:

    };

    class ComputeContext : public CommandContext
    {

    };
}