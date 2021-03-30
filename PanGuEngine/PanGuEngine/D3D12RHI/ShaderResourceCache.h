#pragma once

#include "DescriptorHeap.h"
#include "GpuBuffer.h"
#include "GpuResourceDescriptor.h"

namespace RHI 
{
    class RootSignature;
    class RenderDevice;
    class CommandContext;
	
    /**
    * ֻ�ṩ���洢�ռ䣨GPUDescriptorHeap����������ɶ����֪��
    * �洢�󶨵�Shader����Դ,ShaderResourceCache�����GPU-visible Descriptor Heap�ϵĿռ䣬���洢��Դ��Descriptor
    * Root View����GPUDescriptorHeap�з���ռ䣬��ֱ�Ӱ󶨵�RootSignature
    * Static��Mutable��Root Table����GPUDescriptorHeap�з���ռ䣬Dynamic��ÿ��Draw Call�ж�̬�ķ���ռ�
    */
    class ShaderResourceCache
    {
        friend class CommandContext;

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
            RootDescriptor(SHADER_RESOURCE_VARIABLE_TYPE _VariableType) : VariableType(_VariableType) {}

            SHADER_RESOURCE_VARIABLE_TYPE VariableType;
			std::shared_ptr<GpuBuffer> ConstantBuffer = nullptr;
		};

        struct RootTable
        {
            // TODO: RootTableû��Ĭ�Ϲ��캯���ᱨ��
            // Error C2512	'RHI::ShaderResourceCache::RootTable::RootTable': no appropriate default constructor available	
            // PanGuEngine	F:\Tools\VS2019\VC\Tools\MSVC\14.28.29333\include\tuple	980	
            RootTable(SHADER_RESOURCE_VARIABLE_TYPE _VariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC, UINT32 tableSize = 0) : Descriptors(tableSize){}

			SHADER_RESOURCE_VARIABLE_TYPE VariableType;

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

        // �ύStatic��Mutable����Դ��
        void CommitResource(CommandContext& cmdContext);
        /*�ύDynamic Shader Variable����Դ���Լ�Dynamic Resource����Դ��ÿ��Drawǰ�Զ��ύ
        * Dynamic Resource������Dynamic Shader Variable��Dynamic Resource��ʾ��Դ�����޸ĵ�Ƶ�ʣ�Dynamic Shader Variable��ʾ��Դ���л���Ƶ��
        * ����Transform��Constant Buffer��Shader Variable���;���Static���������Buffer��Dynamic Buffer
        */
        void CommitDynamic(CommandContext& cmdContext);

    	RootDescriptor& GetRootDescriptor(UINT32 RootIndex)
        {
            return m_RootDescriptors.at(RootIndex);
        }

    	const RootDescriptor& GetRootDescriptor(UINT32 RootIndex) const
        {
            return m_RootDescriptors.at(RootIndex);
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

        UINT32 m_NumDynamicDescriptor = 0;

        std::unordered_map<UINT32/*RootIndex*/, RootDescriptor> m_RootDescriptors;
        std::unordered_map<UINT32/*RootIndex*/, RootTable> m_RootTables;

        ID3D12Device* m_D3D12Device;
    };
}