#pragma once
#include "Constant.h"

namespace RHI
{
	class RootParameter
	{
	public:
		// 三个构造函数,对应DX12中RootParamater三种类型：Root Constant、Root View、Root Table
        // 最好不存储Root Constant，Root Parameter的总大小有限制，Root Constant会占用大量空间
        // Root Table稍微复杂一点,一个Root Table可以有多个Descriptor Range，一个Descriptor Range其实就是一个相同类型(CBV、SRV、UAV)的Descriptor数组
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

        // 在现有的Root Table中添加Descriptor Range时会调用
        void AddDescriptorRanges(UINT32 addRangesNum)
        {
            m_DescriptorRanges.push_back

            // 添加了新的Descriptor Range后，要重新赋值指针，因为当vector扩容后，存储位置会变化!!!!!!
            m_RootParam.DescriptorTable.pDescriptorRanges = &m_DescriptorRanges[0];

        }

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

        UINT32 GetDescriptorTableSize() const
        {
            assert(m_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && "Incorrect parameter table: descriptor table is expected");
            return m_DescriptorTableSize;
        }

        D3D12_SHADER_VISIBILITY   GetShaderVisibility() const { return m_RootParam.ShaderVisibility; }
        D3D12_ROOT_PARAMETER_TYPE GetParameterType() const { return m_RootParam.ParameterType; }

        UINT32 GetRootIndex() const { return m_RootIndex; }

        // 定义到D3D12_ROOT_PARAMETER的隐式转换
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
		// TODO: 用-1初始化是什么作用？？？
		SHADER_RESOURCE_VARIABLE_TYPE m_ShaderVarType = static_cast<SHADER_RESOURCE_VARIABLE_TYPE>(-1);
		D3D12_ROOT_PARAMETER m_RootParam = {};
		UINT32 m_DescriptorTableSize = 0;
		UINT32 m_RootIndex = static_cast<UINT32>(-1);
        std::vector<D3D12_DESCRIPTOR_RANGE> m_DescriptorRanges;
	};


	class RootSignature
	{
	public:

	private:
        // Slot: 插槽
        std::array<UINT32, SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES> m_TotalSrvCbvUavSlots = {};
        std::array<UINT32, SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES> m_TotalSamplerSlots = {};
        std::array<UINT32, SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES> m_TotalRootViews = {};

        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pd3d12RootSignature;

        // 内部嵌套类，帮助管理RootParam
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

            // 添加一个新的RootView
            void AddRootView(D3D12_ROOT_PARAMETER_TYPE     ParameterType,
                             UINT32                        RootIndex,
                             UINT                          Register,
                             D3D12_SHADER_VISIBILITY       Visibility,
                             SHADER_RESOURCE_VARIABLE_TYPE VarType);
            // 添加一个新的RootTable
            void AddRootTable(UINT32                        RootIndex,
                              D3D12_SHADER_VISIBILITY       Visibility,
                              SHADER_RESOURCE_VARIABLE_TYPE VarType,
                              UINT32                        NumRangesInNewTable = 1);
            // 在现有的RootTable中添加Descriptor Range
            void AddDescriptorRanges(UINT32 RootTableInd, UINT32 NumExtraRanges = 1);

            template <typename TOperation>
            void ProcessRootTables(TOperation) const;

            bool   operator==(const RootParamsManager& RootParams) const;
            size_t GetHash() const;

        private:
            UINT32 m_TotalDescriptorRanges = 0;
            std::vector<RootParameter> m_RootTables;
            std::vector<RootParameter> m_RootViews;
        };

        static constexpr UINT8 InvalidRootTableIndex = static_cast<UINT8>(-1);

        // The array below contains array index of a CBV/SRV/UAV root table
        // in m_RootParams (NOT the Root Index!), for every variable type
        // (static, mutable, dynamic) and every shader type,
        // or -1, if the table is not yet assigned to the combination
        // 该数组存储了每个Shader的每个Shader Variable Type 的 CBVSRVUAV Root Table在m_RootParams中的索引（不是RootIndex）
        std::array<UINT8, SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES * MAX_SHADERS_IN_PIPELINE> m_SrvCbvUavRootTablesMap = {};
        std::array<UINT8, SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES * MAX_SHADERS_IN_PIPELINE> m_SamplerRootTablesMap = {};

        RootParamsManager m_RootParams;
	};
    template<typename TOperation>
    inline void RootSignature::RootParamsManager::ProcessRootTables(TOperation) const
    {
    }
}

