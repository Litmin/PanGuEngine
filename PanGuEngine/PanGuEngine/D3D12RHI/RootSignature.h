#pragma once
#include "Constant.h"
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
        RootSignature();

        // ���Root Signature�Ĺ��죬����Direct3D 12��Root Signature
        void Finalize(ID3D12Device* pd3d12Device);

        ID3D12RootSignature* GetD3D12RootSignature() const { return m_pd3d12RootSignature.Get(); }

        // ��ʼ��SRB��Cache����Ϊֻ��RootSignature֪���㹻����Ϣ����ʼ��ÿ��Shader
        void InitResourceCacheForSRB(RenderDevice* RenderDevice, ShaderResourceCache& ResourceCache) const;

        // ΪShader�е�ÿ��ShaderResource����һ������֮��
        void AllocateResourceSlot(SHADER_TYPE                     ShaderType,
                                  PIPELINE_TYPE                   PipelineType,
                                  const ShaderResourceAttribs& ShaderResAttribs,
                                  SHADER_RESOURCE_VARIABLE_TYPE   VariableType,
                                  D3D12_DESCRIPTOR_RANGE_TYPE     RangeType,
                                  UINT32& RootIndex,
                                  UINT32& OffsetFromTableStart);

	private:
        // ����Resource Cache����Ҫ��Table�������Լ�ÿ��Table��Descriptor��������Root View����ֻ��һ��Descriptor��Table
        std::vector<UINT32> GetCacheTableSizes() const;


        // �ڲ�Ƕ���࣬��������RootParam
        class RootParamsManager
        {
        public:
            RootParamsManager() = default;

            RootParamsManager(const RootParamsManager&) = delete;
            RootParamsManager& operator=(const RootParamsManager&) = delete;
            RootParamsManager(RootParamsManager&&) = delete;
            RootParamsManager& operator=(RootParamsManager&&) = delete;

            ~RootParamsManager() = default;

            UINT32 GetRootTableNum() const { return m_RootTables.size(); }
            UINT32 GetRootViewNum() const { return m_RootViews.size(); }

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

            const RootParameter& GetRootView(UINT32 viewIndex) const
            {
                assert(viewIndex < m_RootViews.size());
                return m_RootViews[viewIndex];
            }

            RootParameter& GetRootView(UINT32 viewIndex)
            {
                assert(viewIndex < m_RootViews.size());
                return m_RootViews[viewIndex];
            }

            // ���һ���µ�RootView
            void AddRootView(D3D12_ROOT_PARAMETER_TYPE     ParameterType,
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
            void ProcessRootTables(TOperation) const;

            bool   operator==(const RootParamsManager& RootParams) const;
            size_t GetHash() const;

        private:
            UINT32 m_TotalDescriptorRanges = 0;
            std::vector<RootParameter> m_RootTables;
            std::vector<RootParameter> m_RootViews; //                  <-----
        };//                                                                  |
        //                                                                    |
        //                                                                    |
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pd3d12RootSignature;//  |
        //                                                                    |                        
        // RootParameter����Root View��Root Table���ֱ�洢������Vector�У�--------
        // ����RootParameterʱ������Shader��������˳�����δ���RootIndexҲ�Ͱ�������˳��
        // Root Table�У�Static��Mutable��Դ�Ƿ���һ��Root Table�еģ�
        // �����m_SrvCbvUavRootTablesMap�洢����ָ��Shader�׶κ�ָ��Shader Variable���͵�Root Table�������m_RootTables�е�����������Root Index��
        //
        RootParamsManager m_RootParams;

        // Slot: ���
        std::array<UINT32, SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES> m_TotalSrvCbvUavSlots = {};
        std::array<UINT32, SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES> m_TotalRootViews = {};

        static constexpr UINT8 InvalidRootTableIndex = static_cast<UINT8>(-1);

        // The array below contains array index of a CBV/SRV/UAV root table
        // in m_RootParams (NOT the Root Index!), for every variable type
        // (static, mutable, dynamic) and every shader type,
        // or -1, if the table is not yet assigned to the combination
        // ������洢��ÿ��Shader��ÿ��Shader Variable Type �� CBVSRVUAV Root Table��m_RootParams�е�����������RootIndex��
        std::array<UINT8, SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES * MAX_SHADERS_IN_PIPELINE> m_SrvCbvUavRootTablesMap = {};
	};
}

