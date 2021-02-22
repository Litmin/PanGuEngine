#pragma once

#include "DescriptorHeap.h"
#include "IShaderResource.h"

namespace RHI 
{
    class RootSignature;
    class RenderDevice;
	
    /**
    * ֻ�ṩ���洢�ռ䣨GPUDescriptorHeap����������ɶ����֪��
    * �洢�󶨵�Shader����Դ,ShaderResourceCache�����GPU-visible Descriptor Heap�ϵĿռ䣬���洢��Դ��Descriptor
    * Root View����GPUDescriptorHeap�з���ռ䣬��ֱ�Ӱ󶨵�RootSignature
    * Static��Mutable��Root Table����GPUDescriptorHeap�з���ռ䣬Dynamic��ÿ��Draw Call�ж�̬�ķ���ռ�
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
            // �ñ����洢����CPUDescriptorHeap�е�Descriptor Handle
            D3D12_CPU_DESCRIPTOR_HANDLE CPUDescriptorHandle = { 0 };
            // ǿ������Դ
            std::shared_ptr<IShaderResource> pObject;
        };

        class RootTable
        {
        public:
            RootTable(UINT32 tableSize) : m_Resources(tableSize){}

            // ����RootTable�е�ĳ����Դ
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

            // RootTable�Ĵ�С
            UINT32 Size() const { return m_Resources.size(); }

            // ÿ��Root Table��GPUDescriptorHeap�е���ʼλ��
            UINT32 m_TableStartOffset = InvalidDescriptorOffset;
        private:
            // ��Table����Դ������
            std::vector<Resource> m_Resources;
        };

    	class RootView
    	{
    	public:
            // ����RootTable�е�ĳ����Դ
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

        // ShaderResourceLayoutͨ���ú�������ȡDescriptor Handle������Ҫ�󶨵���Դ��Descriptor��������!!!
        // OffsetFromTableStart����Table�е�ƫ�ƣ���RootParam.m_TableStartOffset��ͬ
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
    	// OffsetFromTableStart����Table�е�ƫ�ƣ���RootParam.m_TableStartOffset��ͬ
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
        	// ����д�ᱨ����Ϊ[]��������û�з������Key�����ͻ����һ���µ�Ԫ�أ�����[]�Ƿ�const��
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