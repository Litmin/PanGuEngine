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

		// Ŀǰֻ��Constant Buffer��ΪRoot Descriptor��
		struct RootDescriptor
		{
			std::shared_ptr<GpuBuffer> ConstantBuffer;
		};

        struct RootTable
        {
            RootTable(UINT32 tableSize) : Descriptors(tableSize){}

            // ÿ��Root Table��GPUDescriptorHeap�е���ʼλ��
            UINT32 TableStartOffset = InvalidDescriptorOffset;

            // ��Table����Դ������
            std::vector<std::shared_ptr<GpuResourceDescriptor>> Descriptors;
        };


        // ShaderResourceLayoutͨ���ú�������ȡDescriptor Handle������Ҫ�󶨵���Դ��Descriptor��������!!!
        // OffsetFromTableStart����Table�е�ƫ�ƣ���RootParam.m_TableStartOffset��ͬ
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

        // RootSignatureͨ���ú���������GPU Descriptor Handle��Ȼ���ύ����Ⱦ����!!!
    	// OffsetFromTableStart����Table�е�ƫ�ƣ���RootParam.m_TableStartOffset��ͬ
        template <D3D12_DESCRIPTOR_HEAP_TYPE HeapType>
        D3D12_GPU_DESCRIPTOR_HANDLE GetShaderVisibleTableGPUDescriptorHandle(UINT32 RootIndex, UINT32 OffsetFromTableStart = 0) const
        {
            const auto& RootParam = GetRootTable(RootIndex);

            // Dynamic��Դû����ShaderResourceCache��Heap�з���ռ䣬����m_TableStartOffsetӦ��ʱInvalid
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
        	// ����д�ᱨ����Ϊ[]��������û�з������Key�����ͻ����һ���µ�Ԫ�أ�����[]�Ƿ�const��
            //return m_RootTables[RootIndex];
        }
        
    private:
        // GPU Descriptor Heap
        DescriptorHeapAllocation m_CbvSrvUavGPUHeapSpace;

        std::unordered_map<UINT32/*RootIndex*/, RootDescriptor> m_RootViews;
        std::unordered_map<UINT32/*RootIndex*/, RootTable> m_RootTables;
    };
}