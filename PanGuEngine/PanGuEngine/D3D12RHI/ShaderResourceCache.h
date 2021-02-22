#pragma once

#include "DescriptorHeap.h"
#include "IShaderResource.h"

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

        struct Resource
        {
            CachedResourceType Type = CachedResourceType::Unknown;
            // 该变量存储的是CPUDescriptorHeap中的Descriptor Handle
            D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle = { 0 };
            // 强引用资源
            std::shared_ptr<IShaderResource> pObject;
        };

        class RootTable
        {
        public:
            RootTable(UINT32 tableSize) : m_Resources(tableSize){}

            // 访问RootTable中的某个资源
            const Resource* GetResource(UINT32 OffsetFromTableStart) const
            {
                assert((OffsetFromTableStart < m_Resources.size()) && "Index out of range.");
                return &m_Resources[OffsetFromTableStart];
            }

            Resource* GetResource(UINT32 OffsetFromTableStart)
            {
                assert((OffsetFromTableStart < m_Resources.size()) && "Index out of range.");
                return &m_Resources[OffsetFromTableStart];
            }

            // RootTable的大小
            UINT32 Size() const { return m_Resources.size(); }

            // 每个Root Table在GPUDescriptorHeap中的起始位置
            UINT32 m_TableStartOffset = InvalidDescriptorOffset;
        private:
            // 该Table中资源的数量
            std::vector<Resource> m_Resources;
        };

    	class RootView
    	{
    	public:
            // 访问RootTable中的某个资源
            const Resource* GetResource() const
            {
                return &m_Resource;
            }

            Resource* GetResource()
            {
                return &m_Resource;
            }
    		
    	private:
            Resource m_Resource;
    	};

        // ShaderResourceLayout通过该函数来获取Descriptor Handle，并把要绑定的资源的Descriptor拷贝过来!!!
        // OffsetFromTableStart是在Table中的偏移，跟RootParam.m_TableStartOffset不同
        template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
        D3D12_CPU_DESCRIPTOR_HANDLE GetShaderVisibleTableCPUDescriptorHandle(UINT32 RootIndex, UINT32 OffsetFromTableStart = 0) const
        {
            const auto& RootParam = GetRootTable(RootIndex);

            D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle = { 0 };

            if (RootParam.m_TableStartOffset != InvalidDescriptorOffset)
            {
                CPUDescriptorHandle = m_CbvSrvUavGPUHeapSpace.GetCpuHandle(RootParam.m_TableStartOffset + OffsetFromTableStart);
            }

            return CPUDescriptorHandle;
        }

        // RootSignature通过该函数来访问GPU Descriptor Handle，然后提交到渲染管线!!!
    	// OffsetFromTableStart是在Table中的偏移，跟RootParam.m_TableStartOffset不同
        template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
        D3D12_GPU_DESCRIPTOR_HANDLE GetShaderVisibleTableGPUDescriptorHandle(UINT32 RootIndex, UINT32 OffsetFromTableStart = 0) const
        {
            const auto& RootParam = GetRootTable(RootIndex);

            assert(RootParam.m_TableStartOffset != InvalidDescriptorOffset && "GPU descriptor handle must never be requested for dynamic resources");

            D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandle = m_CbvSrvUavGPUHeapSpace.GetGpuHandle(RootParam.m_TableStartOffset + OffsetFromTableStart);

            return GPUDescriptorHandle;
        }

    	RootView& GetRootView(UINT32 RootIndex)
        {
            return m_RootViews.at(RootIndex);
        }

    	const RootView& GetRootView(UINT32 RootIndex) const
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
        
        UINT32 GetRootTablesNum() const { return m_RootTables.size(); }


    private:
        // GPU Descriptor Heap
        DescriptorHeapAllocation m_CbvSrvUavGPUHeapSpace;

        std::unordered_map<UINT32/*RootIndex*/, RootView> m_RootViews;
        std::unordered_map<UINT32/*RootIndex*/, RootTable> m_RootTables;
    };
}