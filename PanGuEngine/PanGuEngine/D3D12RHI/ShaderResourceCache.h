#pragma once

#include "DescriptorHeap.h"
#include "IShaderResource.h"

namespace RHI 
{
    /**
    * 只提供个存储空间（GPUDescriptorHeap），其他的啥都不知道
    * 存储绑定到Shader的资源,ShaderResourceCache会分配GPU-visible Descriptor Heap上的空间，来存储资源的Descriptor
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

    	/* 缓存绑定的资源，包括：RootView和RootTable
    	 *      RootView:不提供GPUDescriptorHeap的空间，只保存资源的引用和资源的CPU Descriptor
    	 *      RootTable:为Static和Mutable的资源提供GPUDescriptorHeap的空间，Dynamic在每帧中分配、释放
    	*/
        void Initialize(UINT32 rootViewNum, UINT32 tableNum, UINT32 tableSizes[]);

        static constexpr UINT32 InvalidDescriptorOffset = static_cast<UINT32>(-1);

        struct Resource
        {
            CachedResourceType Type = CachedResourceType::Unknown;
            // 该变量存储的是CPUDescriptorHeap中的Descriptor Handle
            D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle = { 0 };
            // 强引用资源
            std::shared_ptr<IShaderResource> pObject;
        };

        // CBV被当作只有一个Descriptor的Table
        class RootTable
        {
        public:
            RootTable(UINT32 tableSize) : m_Resources(tableSize){}

            // 访问RootTable中的某个资源
            inline const Resource& GetResource(UINT32 OffsetFromTableStart) const
            {
                assert((OffsetFromTableStart < m_Resources.size()) && "Index out of range.");
                return m_Resources[OffsetFromTableStart];
            }

            inline Resource& GetResource(UINT32 OffsetFromTableStart)
            {
                assert((OffsetFromTableStart < m_Resources.size()) && "Index out of range.");
                return m_Resources[OffsetFromTableStart];
            }

            // RootTable的大小
            inline UINT32 Size() const { return m_Resources.size(); }

            // 在GPUDescriptorHeap中的起始位置
            UINT32 m_TableStartOffset = InvalidDescriptorOffset;
        private:
            // 该Table中资源的数量
            std::vector<Resource> m_Resources;
        };

    	class RootView
    	{
    	public:

    	private:
            Resource m_Resource;
    	};

        // 外部在GPUDescriptorHeap中进行分配，然后赋值给ShaderResourceCache
        void SetDescriptorHeapSpace(DescriptorHeapAllocation&& CbcSrvUavHeapSpace)
        {
            m_CbvSrvUavGPUHeapSpace = std::move(CbcSrvUavHeapSpace);
        }

        // ShaderResourceLayout通过该函数来获取Descriptor Handle，并把要绑定的资源的Descriptor拷贝过来!!!
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
        template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
        D3D12_GPU_DESCRIPTOR_HANDLE GetShaderVisibleTableGPUDescriptorHandle(UINT32 RootIndex, UINT32 OffsetFromTableStart = 0) const
        {
            const auto& RootParam = GetRootTable(RootIndex);

            assert(RootParam.m_TableStartOffset != InvalidDescriptorOffset && "GPU descriptor handle must never be requested for dynamic resources");

            D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandle = m_CbvSrvUavGPUHeapSpace.GetGpuHandle(RootParam.m_TableStartOffset + OffsetFromTableStart);

            return GPUDescriptorHandle;
        }

        // 访问RootTable
        inline RootTable& GetRootTable(UINT32 RootIndex)
        {
            assert(RootIndex < m_RootTables.size());
            return m_RootTables[RootIndex];
        }

        inline const RootTable& GetRootTable(UINT32 RootIndex) const
        {
            assert(RootIndex < m_RootTables.size());
            return m_RootTables[RootIndex];
        }

        inline UINT32 GetRootTablesNum() const { return m_RootTables.size(); }


    private:
        // GPU Descriptor Heap
        DescriptorHeapAllocation m_CbvSrvUavGPUHeapSpace;

        std::vector<RootTable> m_RootTables;

        std::unordered_map<UINT32/*RootIndex*/, RootTable> m_RootTables;
        std::unordered_map<UINT32/*RootIndex*/, RootView> m_RootViews;
    };
}