#pragma once

#include "DescriptorHeap.h"
#include "IShaderResource.h"

namespace RHI 
{
    /**
    * ֻ�ṩ���洢�ռ䣨GPUDescriptorHeap����������ɶ����֪��
    * �洢�󶨵�Shader����Դ,ShaderResourceCache�����GPU-visible Descriptor Heap�ϵĿռ䣬���洢��Դ��Descriptor
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

    	/* ����󶨵���Դ��������RootView��RootTable
    	 *      RootView:���ṩGPUDescriptorHeap�Ŀռ䣬ֻ������Դ�����ú���Դ��CPU Descriptor
    	 *      RootTable:ΪStatic��Mutable����Դ�ṩGPUDescriptorHeap�Ŀռ䣬Dynamic��ÿ֡�з��䡢�ͷ�
    	*/
        void Initialize(UINT32 rootViewNum, UINT32 tableNum, UINT32 tableSizes[]);

        static constexpr UINT32 InvalidDescriptorOffset = static_cast<UINT32>(-1);

        struct Resource
        {
            CachedResourceType Type = CachedResourceType::Unknown;
            // �ñ����洢����CPUDescriptorHeap�е�Descriptor Handle
            D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle = { 0 };
            // ǿ������Դ
            std::shared_ptr<IShaderResource> pObject;
        };

        // CBV������ֻ��һ��Descriptor��Table
        class RootTable
        {
        public:
            RootTable(UINT32 tableSize) : m_Resources(tableSize){}

            // ����RootTable�е�ĳ����Դ
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

            // RootTable�Ĵ�С
            inline UINT32 Size() const { return m_Resources.size(); }

            // ��GPUDescriptorHeap�е���ʼλ��
            UINT32 m_TableStartOffset = InvalidDescriptorOffset;
        private:
            // ��Table����Դ������
            std::vector<Resource> m_Resources;
        };

    	class RootView
    	{
    	public:

    	private:
            Resource m_Resource;
    	};

        // �ⲿ��GPUDescriptorHeap�н��з��䣬Ȼ��ֵ��ShaderResourceCache
        void SetDescriptorHeapSpace(DescriptorHeapAllocation&& CbcSrvUavHeapSpace)
        {
            m_CbvSrvUavGPUHeapSpace = std::move(CbcSrvUavHeapSpace);
        }

        // ShaderResourceLayoutͨ���ú�������ȡDescriptor Handle������Ҫ�󶨵���Դ��Descriptor��������!!!
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

        // RootSignatureͨ���ú���������GPU Descriptor Handle��Ȼ���ύ����Ⱦ����!!!
        template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
        D3D12_GPU_DESCRIPTOR_HANDLE GetShaderVisibleTableGPUDescriptorHandle(UINT32 RootIndex, UINT32 OffsetFromTableStart = 0) const
        {
            const auto& RootParam = GetRootTable(RootIndex);

            assert(RootParam.m_TableStartOffset != InvalidDescriptorOffset && "GPU descriptor handle must never be requested for dynamic resources");

            D3D12_GPU_DESCRIPTOR_HANDLE GPUDescriptorHandle = m_CbvSrvUavGPUHeapSpace.GetGpuHandle(RootParam.m_TableStartOffset + OffsetFromTableStart);

            return GPUDescriptorHandle;
        }

        // ����RootTable
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