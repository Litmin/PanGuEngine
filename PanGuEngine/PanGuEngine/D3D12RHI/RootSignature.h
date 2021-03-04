#pragma once
#include "ShaderResourceCache.h"
#include "ShaderResource.h"
#include "ShaderResourceBindingUtility.h"

namespace RHI
{
    class RenderDevice;

	class RootParameter
	{
	public:
		// �������캯��,��ӦDX12��RootParamater�������ͣ�Root Constant��Root View��Root Table
        // ��ò��洢Root Constant��Root Parameter���ܴ�С�����ƣ�Root Constant��ռ�ô����ռ�
        // Root Table��΢����һ��,һ��Root Table�����ж��Descriptor Range��һ��Descriptor Range��ʵ����һ����ͬ����(CBV��SRV��UAV)��Descriptor����
        // Root View
        RootParameter(D3D12_ROOT_PARAMETER_TYPE     ParameterType,
                      UINT32                        RootIndex,
                      UINT                          Register,
                      UINT                          RegisterSpace,
                      D3D12_SHADER_VISIBILITY       Visibility,
                      SHADER_RESOURCE_VARIABLE_TYPE VarType) noexcept :
            m_RootIndex{ RootIndex },
            m_ShaderVarType{ VarType }
        {
            assert(ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV || ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV || ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV, "Unexpected parameter type - verify argument list");
            m_RootParam.ParameterType = ParameterType;
            m_RootParam.ShaderVisibility = Visibility;
            m_RootParam.Descriptor.ShaderRegister = Register;
            m_RootParam.Descriptor.RegisterSpace = RegisterSpace;
        }
        // Root Constant
        RootParameter(D3D12_ROOT_PARAMETER_TYPE     ParameterType,
                      UINT32                        RootIndex,
                      UINT                          Register,
                      UINT                          RegisterSpace,
                      UINT                          NumDwords,
                      D3D12_SHADER_VISIBILITY       Visibility,
                      SHADER_RESOURCE_VARIABLE_TYPE VarType) noexcept :
            m_RootIndex{ RootIndex },
            m_ShaderVarType{ VarType }
        {
            assert(ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS && "Unexpected parameter type - verify argument list");
            m_RootParam.ParameterType = ParameterType;
            m_RootParam.ShaderVisibility = Visibility;
            m_RootParam.Constants.Num32BitValues = NumDwords;
            m_RootParam.Constants.ShaderRegister = Register;
            m_RootParam.Constants.RegisterSpace = RegisterSpace;
        }
        // Root Table
        RootParameter(D3D12_ROOT_PARAMETER_TYPE     ParameterType,
                      UINT32                        RootIndex,
                      UINT                          NumRanges,
                      D3D12_SHADER_VISIBILITY       Visibility,
                      SHADER_RESOURCE_VARIABLE_TYPE VarType) noexcept :
            m_RootIndex{ RootIndex },
            m_ShaderVarType{ VarType },
            m_DescriptorRanges{NumRanges}
        {
            assert(ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && "Unexpected parameter type - verify argument list");
            m_RootParam.ParameterType = ParameterType;
            m_RootParam.ShaderVisibility = Visibility;
            m_RootParam.DescriptorTable.NumDescriptorRanges = NumRanges;
            m_RootParam.DescriptorTable.pDescriptorRanges = &m_DescriptorRanges[0];
        }

        RootParameter(const RootParameter& RP) noexcept :
            m_RootParam{ RP.m_RootParam },
            m_DescriptorTableSize{ RP.m_DescriptorTableSize },
            m_ShaderVarType{ RP.m_ShaderVarType },
            m_RootIndex{ RP.m_RootIndex }
        {
            assert(m_RootParam.ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && "Use another constructor to copy descriptor table");
        }

        RootParameter& operator=(const RootParameter&) = delete;
        RootParameter& operator=(RootParameter&&) = delete;

        // �����е�Root Table�����Descriptor Range
        void AddDescriptorRanges(UINT32 addRangesNum)
        {
            m_DescriptorRanges.insert(m_DescriptorRanges.end(), addRangesNum, D3D12_DESCRIPTOR_RANGE{});

            // ������µ�Descriptor Range��Ҫ���¸�ֵָ�룬��Ϊ��vector���ݺ󣬴洢λ�û�仯!!!!!!
            m_RootParam.DescriptorTable.pDescriptorRanges = &m_DescriptorRanges[0];
            m_RootParam.DescriptorTable.NumDescriptorRanges += addRangesNum;
        }

        // ����ָ��Descriptor Range������
        void SetDescriptorRange(UINT                        RangeIndex,
                                D3D12_DESCRIPTOR_RANGE_TYPE Type,
                                UINT                        Register,
                                UINT                        Count,
                                UINT                        Space = 0,
                                UINT                        OffsetFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
        {
            assert(m_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && "Incorrect parameter table: descriptor table is expected");
            auto& table = m_RootParam.DescriptorTable;
            assert(RangeIndex < table.NumDescriptorRanges && "Invalid descriptor range index");
            D3D12_DESCRIPTOR_RANGE& range = const_cast<D3D12_DESCRIPTOR_RANGE&>(table.pDescriptorRanges[RangeIndex]);
            assert(range.RangeType == static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(-1) && "Descriptor range has already been initialized. m_DescriptorTableSize may be updated incorrectly");
            range.RangeType = Type;
            range.NumDescriptors = Count;
            range.BaseShaderRegister = Register;
            range.RegisterSpace = Space;
            range.OffsetInDescriptorsFromTableStart = OffsetFromTableStart;
            m_DescriptorTableSize = std::max(m_DescriptorTableSize, OffsetFromTableStart + Count);
        }

        SHADER_RESOURCE_VARIABLE_TYPE GetShaderVariableType() const { return m_ShaderVarType; }

        // ��Root Table��Descriptor������Root Table���ж��Descriptor Range�������ǰ�ÿ��Descriptor Range��Descriptor����������
        UINT32 GetDescriptorTableSize() const
        {
            assert(m_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && "Incorrect parameter table: descriptor table is expected");
            return m_DescriptorTableSize;
        }

        D3D12_SHADER_VISIBILITY   GetShaderVisibility() const { return m_RootParam.ShaderVisibility; }
        D3D12_ROOT_PARAMETER_TYPE GetParameterType() const { return m_RootParam.ParameterType; }

        UINT32 GetRootIndex() const { return m_RootIndex; }

        // ���嵽D3D12_ROOT_PARAMETER����ʽת��
        operator const D3D12_ROOT_PARAMETER& () const { return m_RootParam; }

        bool operator==(const RootParameter& rhs) const
        {
            if (m_ShaderVarType != rhs.m_ShaderVarType ||
                m_DescriptorTableSize != rhs.m_DescriptorTableSize ||
                m_RootIndex != rhs.m_RootIndex)
                return false;

            if (m_RootParam.ParameterType != rhs.m_RootParam.ParameterType ||
                m_RootParam.ShaderVisibility != rhs.m_RootParam.ShaderVisibility)
                return false;

            switch (m_RootParam.ParameterType)
            {
            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            {
                const auto& tbl0 = m_RootParam.DescriptorTable;
                const auto& tbl1 = rhs.m_RootParam.DescriptorTable;
                if (tbl0.NumDescriptorRanges != tbl1.NumDescriptorRanges)
                    return false;
                for (UINT r = 0; r < tbl0.NumDescriptorRanges; ++r)
                {
                    const auto& rng0 = tbl0.pDescriptorRanges[r];
                    const auto& rng1 = tbl1.pDescriptorRanges[r];
                    if (memcmp(&rng0, &rng1, sizeof(rng0)) != 0)
                        return false;
                }
            }
            break;

            case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            {
                const auto& cnst0 = m_RootParam.Constants;
                const auto& cnst1 = rhs.m_RootParam.Constants;
                if (memcmp(&cnst0, &cnst1, sizeof(cnst0)) != 0)
                    return false;
            }
            break;

            case D3D12_ROOT_PARAMETER_TYPE_CBV:
            case D3D12_ROOT_PARAMETER_TYPE_SRV:
            case D3D12_ROOT_PARAMETER_TYPE_UAV:
            {
                const auto& dscr0 = m_RootParam.Descriptor;
                const auto& dscr1 = rhs.m_RootParam.Descriptor;
                if (memcmp(&dscr0, &dscr1, sizeof(dscr0)) != 0)
                    return false;
            }
            break;

            default: LOG_ERROR("Unexpected root parameter type");
            }

            return true;
        }

        bool operator!=(const RootParameter& rhs) const
        {
            return !(*this == rhs);
        }

        size_t GetHash() const
        {
            size_t hash = ComputeHash(m_ShaderVarType, m_DescriptorTableSize, m_RootIndex);
            HashCombine(hash, m_RootParam.ParameterType, m_RootParam.ShaderVisibility);

            switch (m_RootParam.ParameterType)
            {
            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            {
                const auto& tbl = m_RootParam.DescriptorTable;
                HashCombine(hash, tbl.NumDescriptorRanges);
                for (UINT r = 0; r < tbl.NumDescriptorRanges; ++r)
                {
                    const auto& rng = tbl.pDescriptorRanges[r];
                    HashCombine(hash, rng.BaseShaderRegister, rng.NumDescriptors, rng.OffsetInDescriptorsFromTableStart, rng.RangeType, rng.RegisterSpace);
                }
            }
            break;

            case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            {
                const auto& cnst = m_RootParam.Constants;
                HashCombine(hash, cnst.Num32BitValues, cnst.RegisterSpace, cnst.ShaderRegister);
            }
            break;

            case D3D12_ROOT_PARAMETER_TYPE_CBV:
            case D3D12_ROOT_PARAMETER_TYPE_SRV:
            case D3D12_ROOT_PARAMETER_TYPE_UAV:
            {
                const auto& dscr = m_RootParam.Descriptor;
                HashCombine(hash, dscr.RegisterSpace, dscr.ShaderRegister);
            }
            break;

            default: LOG_ERROR("Unexpected root parameter type");
            }

            return hash;
        }

	private:
		// TODO: ��-1��ʼ����ʲô���ã�����
		SHADER_RESOURCE_VARIABLE_TYPE m_ShaderVarType = static_cast<SHADER_RESOURCE_VARIABLE_TYPE>(-1);
		D3D12_ROOT_PARAMETER m_RootParam = {};
		UINT32 m_DescriptorTableSize = 0;
		UINT32 m_RootIndex = static_cast<UINT32>(-1);
        std::vector<D3D12_DESCRIPTOR_RANGE> m_DescriptorRanges;
	};


	class RootSignature
	{
	public:
        RootSignature(RenderDevice* renderDevice);
        ~RootSignature();

        // ���Root Signature�Ĺ��죬����Direct3D 12��Root Signature
        void Finalize(ID3D12Device* pd3d12Device);

        ID3D12RootSignature* GetD3D12RootSignature() const { return m_pd3d12RootSignature.Get(); }

        // ΪShader�е�ÿ��ShaderResource����һ������֮��
        void AllocateResourceSlot(SHADER_TYPE                     ShaderType,
                                  PIPELINE_TYPE                   PipelineType,
                                  const ShaderResourceAttribs& ShaderResAttribs,
                                  SHADER_RESOURCE_VARIABLE_TYPE   VariableType,
                                  D3D12_DESCRIPTOR_RANGE_TYPE     RangeType,
                                  UINT32& RootIndex,
                                  UINT32& OffsetFromTableStart);

        // VarType���͵�RootTable������Descriptor��������
        UINT32 GetNumDescriptorInRootTable(SHADER_RESOURCE_VARIABLE_TYPE VarType) const
        {
            return m_NumDescriptorInRootTable[VarType];
        }

        // VarType���͵�RootView������
        UINT32 GetNumRootDescriptor(SHADER_RESOURCE_VARIABLE_TYPE VarType) const
        {
            return m_NumRootDescriptor[VarType];
        }

        template <typename TOperation>
        void ProcessRootDescriptors(TOperation Operation) const
        {
            m_RootParams.ProcessRootDescriptors(Operation);
        }

		template <typename TOperation>
		void ProcessRootTables(TOperation Operation) const
		{
            m_RootParams.ProcessRootTables(Operation);
		}
		
		/*
		 * �ύ��Դ
		 * @param isStatic:�ύ��Դ�����ͣ����л�PSOʱ���ύStatic��Դ���л�SRBʱ���ύMutable/Dynamic��Դ
		 */
        void CommitResource(bool isStatic);
		

	private:
        // �ڲ�Ƕ���࣬��������RootParam
        class RootParamsManager
        {
        public:
            UINT32 GetRootTableNum() const { return m_RootTables.size(); }
            UINT32 GetRootDescriptorNum() const { return m_RootDescriptors.size(); }

            const RootParameter& GetRootTable(UINT32 tableIndex) const
            {
                assert(tableIndex < m_RootTables.size());
                return m_RootTables[tableIndex];
            }

            RootParameter& GetRootTable(UINT32 tableIndex)
            {
                assert(tableIndex < m_RootTables.size());
                return m_RootTables[tableIndex];
            }

            const RootParameter& GetRootDescriptor(UINT32 descriptorIndex) const
            {
                assert(descriptorIndex < m_RootDescriptors.size());
                return m_RootDescriptors[descriptorIndex];
            }

            RootParameter& GetRootDescriptor(UINT32 descriptorIndex)
            {
                assert(descriptorIndex < m_RootDescriptors.size());
                return m_RootDescriptors[descriptorIndex];
            }

            // ���һ���µ�RootView
            void AddRootDescriptor(D3D12_ROOT_PARAMETER_TYPE     ParameterType,
                             UINT32                        RootIndex,
                             UINT                          Register,
                             D3D12_SHADER_VISIBILITY       Visibility,
                             SHADER_RESOURCE_VARIABLE_TYPE VarType);
            // ���һ���µ�RootTable
            void AddRootTable(UINT32                        RootIndex,
                              D3D12_SHADER_VISIBILITY       Visibility,
                              SHADER_RESOURCE_VARIABLE_TYPE VarType,
                              UINT32                        NumRangesInNewTable = 1);
            // �����е�RootTable�����Descriptor Range
            void AddDescriptorRanges(UINT32 RootTableInd, UINT32 NumExtraRanges = 1);

            template <typename TOperation>
            void ProcessRootDescriptors(TOperation) const;
        	
            template <typename TOperation>
            void ProcessRootTables(TOperation) const;

            bool   operator==(const RootParamsManager& RootParams) const;
            size_t GetHash() const;

        private:
            std::vector<RootParameter> m_RootTables;
            std::vector<RootParameter> m_RootDescriptors; //                  <-----
        };//                                                                  |
        //                                                                    |
        //                                                                    |
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pd3d12RootSignature;//  |
        //                                                                    |                        
        // RootParameter����Root View��Root Table���ֱ�洢������Vector�У�--------
        // ����RootParameterʱ������Shader��������˳�����δ���RootIndexҲ�Ͱ�������˳��
        // CBV����ΪRootView�������İ��ո���Ƶ�ʷ��飬Static��Mutable��Dynamic�ֱ𱣴�Ϊ����Root Table����ͬ��Shader�ֿ����棬����Root Table�������:Shader���� x 3
        // �����m_SrvCbvUavRootTablesMap�洢����ָ��Shader�׶κ�ָ��Shader Variable���͵�Root Table�������m_RootTables�е�����������Root Index��
        // �����ж�ĳ��Shader��ĳ��Variable Type��RootTable�Ƿ��Ѿ�����������Ѿ��������������Root Table�����Descriptor Range�����û�д������ʹ���һ���µ�Root Table
        RootParamsManager m_RootParams;

        RenderDevice* m_RenderDevice;

        // ��¼ÿ��Variable���͵�����RootTable��Descriptor��������
        std::array<UINT32, SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES> m_NumDescriptorInRootTable = {};
        // ��¼ÿ��Variable���͵�����RootDescriptor������
        std::array<UINT32, SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES> m_NumRootDescriptor = {};

        static constexpr UINT8 InvalidRootTableIndex = static_cast<UINT8>(-1);

        std::array<UINT8, SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES * MAX_SHADERS_IN_PIPELINE> m_SrvCbvUavRootTablesMap = {};
	};

    template <typename TOperation>
    void RootSignature::RootParamsManager::ProcessRootDescriptors(TOperation Operation) const
    {
    	for(UINT32 i = 0;i < m_RootDescriptors.size();++i)
    	{
            auto& rootView = m_RootDescriptors[i];
            Operation(rootView);
    	}
    }

    template <typename TOperation>
    void RootSignature::RootParamsManager::ProcessRootTables(TOperation Operation) const
    {
    	for(UINT32 i = 0;i < m_RootTables.size();++i)
    	{
            auto& rootTable = m_RootTables[i];
            Operation(rootTable);
    	}
    }
}

