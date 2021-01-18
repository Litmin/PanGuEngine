#pragma once

namespace RHI 
{
    inline bool IsAllowedType(SHADER_RESOURCE_VARIABLE_TYPE varType, UINT32 allowedTypeBits) noexcept
    {
        return ((1 << varType) & allowedTypeBits) != 0;
    }

    inline UINT32 GetAllowedTypeBits(const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes, UINT32 allowedTypeNum) noexcept
    {
        if (allowedVarTypes == nullptr)
            return 0xFFFFFFFF;

        UINT32 AllowedTypeBits = 0;
        for (UINT32 i = 0; i < allowedTypeNum; ++i)
            AllowedTypeBits |= 1 << allowedVarTypes[i];
        return AllowedTypeBits;
    }
}