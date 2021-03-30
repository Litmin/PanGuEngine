#pragma once

namespace RHI 
{
    bool IsAllowedType(SHADER_RESOURCE_VARIABLE_TYPE varType, UINT32 allowedTypeBits) noexcept;

    UINT32 GetAllowedTypeBits(const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes, UINT32 allowedTypeNum) noexcept;

    // 判断Shader Type和Pipeline Type是否兼容，比如Compute Pipeline就不能有VS、PS
    bool IsConsistentShaderType(SHADER_TYPE ShaderType, PIPELINE_TYPE PipelineType);

    // 把Shader Type作为索引，构造Root Table时会使用
    INT32 GetShaderTypePipelineIndex(SHADER_TYPE ShaderType, PIPELINE_TYPE PipelineType);

    // 从ShaderVariableConfig中找出某个ShaderResource的Variable Type（Static、Mutable、Dynamic）
    SHADER_RESOURCE_VARIABLE_TYPE GetShaderVariableType(SHADER_TYPE shaderType, 
                                                        const std::string& name, 
                                                        const struct ShaderVariableConfig& shaderVariableConfig);

    // CachedResourceType to D3D12_DESCRIPTOR_RANGE_TYPE
    D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(BindingResourceType cachedResType);

    D3D12_SHADER_VISIBILITY GetShaderVisibility(SHADER_TYPE ShaderType);
}