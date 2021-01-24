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

    // 判断Shader Type和Pipeline Type是否兼容，比如Compute Pipeline就不能有VS、PS
    bool IsConsistentShaderType(SHADER_TYPE ShaderType, PIPELINE_TYPE PipelineType)
    {
        static_assert(SHADER_TYPE_LAST == 0x080, "Please update the switch below to handle the new shader type");
        switch (PipelineType)
        {
        case PIPELINE_TYPE_GRAPHIC:
            return ShaderType == SHADER_TYPE_VERTEX ||
                ShaderType == SHADER_TYPE_HULL ||
                ShaderType == SHADER_TYPE_DOMAIN ||
                ShaderType == SHADER_TYPE_GEOMETRY ||
                ShaderType == SHADER_TYPE_PIXEL;

        case PIPELINE_TYPE_COMPUTE:
            return ShaderType == SHADER_TYPE_COMPUTE;

        case PIPELINE_TYPE_MESH:
            return ShaderType == SHADER_TYPE_AMPLIFICATION ||
                ShaderType == SHADER_TYPE_MESH ||
                ShaderType == SHADER_TYPE_PIXEL;

        default:
            LOG_ERROR("Unexpected pipeline type");
            return false;
        }
    }

    // 把Shader Type作为索引，构造Root Table时会使用
    INT32 GetShaderTypePipelineIndex(SHADER_TYPE ShaderType, PIPELINE_TYPE PipelineType)
    {
        assert(IsConsistentShaderType(ShaderType, PipelineType) && "Shader Type和Pipeline Type不兼容！");
        assert(IsPowerOfTwo(UINT32{ ShaderType }), "Only single shader stage should be provided");

        static_assert(SHADER_TYPE_LAST == 0x080, "Please update the switch below to handle the new shader type");
        switch (ShaderType)
        {
        case SHADER_TYPE_UNKNOWN:
            return -1;

        case SHADER_TYPE_VERTEX:        // Graphics
        case SHADER_TYPE_AMPLIFICATION: // Mesh
        case SHADER_TYPE_COMPUTE:       // Compute
            return 0;

        case SHADER_TYPE_HULL: // Graphics
        case SHADER_TYPE_MESH: // Mesh
            return 1;

        case SHADER_TYPE_DOMAIN: // Graphics
            return 2;

        case SHADER_TYPE_GEOMETRY: // Graphics
            return 3;

        case SHADER_TYPE_PIXEL: // Graphics or Mesh
            return 4;

        default:
            LOG_ERROR("Unexpected shader type (", ShaderType, ")");
            return -1;
        }
    }
}