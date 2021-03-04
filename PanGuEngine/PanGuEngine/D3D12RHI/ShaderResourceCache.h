#pragma once

#include "DescriptorHeap.h"
#include "IShaderResource.h"
#include "GpuBuffer.h"
#include "GpuResourceDescriptor.h"

namespace RHI 
{
    class RootSignature;
    class RenderDevice;
	
    /**
    * 只提供个存储空间（GPUDescriptorHeap），其他的啥都不知道
    * 存储绑定到Shader的资源,ShaderResourceCache会分配GPU-visible Descriptor Heap上的空间，来存储资源的Descriptor
    * Root View不在GPUDescriptorHeap中分配空间，它直接绑定到RootSignature
    * Static和Mutable的Root Table会在GPUDescriptorHeap中分配空间，Dynamic在每次Draw Call中动态的分配空间
    */
    class ShaderResourceCache
    {
    public:
        ShaderResourceCache() = default;

        ShaderResourceCache(const ShaderResourceCache&) = delete;
        ShaderResourceCache(ShaderResourceCache&&) = delete;
        ShaderResourceCache& operator = (const ShaderResourceCache&) = delete;
        ShaderResourceCache& operator = (ShaderResourceCache&&) = delete;

        ~ShaderResourceCache() = default;

        void Initialize(RenderDevice* device,
						const RootSignature* rootSignature,
						const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
						UINT32 allowedTypeNum);

        static constexpr UINT32 InvalidDescriptorOffset = static_cast<UINT32>(-1);

		// 目前只有Constant Buffer作为Root Descriptor绑定
		struct RootDescriptor
		{
			std::shared_ptr<GpuBuffer> ConstantBuffer;
		};

        struct RootTable
        {
            RootTable(UINT32 tableSize) : Descriptors(tableSize){}

            // 每个Root Table在GPUDescriptorHeap中的起始位置
            UINT32 TableStartOffset = InvalidDescriptorOffset;

            // 该Table中资源的数量
            std::vector<std::shared_ptr<GpuResourceDescriptor>> Descriptors;
        };


        // ShaderResourceLayout通过该函数来获取Descriptor Handle，并把要绑定的资源的Descriptor拷贝过来!!!
        // OffsetFromTableStart是在Table中的偏移，跟RootParam.m_TableStartOffset不同
        template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
        D3D12_CPU_DESCRIPTOR_HANDLE GetShaderVisibleTableCPUDescriptorHandle(UINT32 RootIndex, UINT32 OffsetFromTableStart = 0) const
        {
            const auto& RootParam = GetRootTable(RootIndex);

            D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle = { 0 };

            if (RootParam.TableStartOffset != InvalidDescriptorOffset)
            {
                CPUDescriptorHandle = m_CbvSrvUavGPUHeapSpace.GetCpuHandle(RootParam.TableStartOffset + OffsetFromTableStart);
            }

            return CPUDescriptorHandle;
        }

        // RootSignature通过该函数来访问GPU Descriptor Handle，然后提交到渲染管线!!!
    	// OffsetFromTableStart是在Table中的偏移，跟RootParam.m_TableStartOffset不同
        template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
        D3D12_GPU_DESCRIPTOR_HANDLE GetShaderVisibleTableGPUDescriptorHandle(UINT32 RootIndex, UINT32 OffsetFromTableStart = 0) const
        {
            const auto& RootParam = GetRootTable(RootIndex);

            // Dynamic资源没有在ShaderResourceCache的Heap中分配空间，所以m_TableStartOffset应该时Invalid
            assert(RootParam.TableStartOffset != InvalidDescriptorOffset && "GPU descriptor handle must never be requested for dynamic resources");

            D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandle = m_CbvSrvUavGPUHeapSpace.GetGpuHandle(RootParam.TableStartOffset + OffsetFromTableStart);

            return GPUDescriptorHandle;
        }

        void CommitResource();
    	

    	RootDescriptor& GetRootDescriptor(UINT32 RootIndex)
        {
            return m_RootViews.at(RootIndex);
        }

    	const RootDescriptor& GetRootDescriptor(UINT32 RootIndex) const
        {
            return m_RootViews.at(RootIndex);
        }

        RootTable& GetRootTable(UINT32 RootIndex)
        {
            return m_RootTables.at(RootIndex);
        }

        const RootTable& GetRootTable(UINT32 RootIndex) const
        {
            return m_RootTables.at(RootIndex);
        	// 这样写会报错，因为[]运算符如果没有发现这个Key，它就会插入一个新的元素，所以[]是非const的
            //return m_RootTables[RootIndex];
        }
        
    private:
        // GPU Descriptor Heap
        DescriptorHeapAllocation m_CbvSrvUavGPUHeapSpace;

        std::unordered_map<UINT32/*RootIndex*/, RootDescriptor> m_RootViews;
        std::unordered_map<UINT32/*RootIndex*/, RootTable> m_RootTables;
    };
}