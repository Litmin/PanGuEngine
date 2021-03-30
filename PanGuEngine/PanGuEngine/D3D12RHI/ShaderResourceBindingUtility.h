#pragma once

namespace RHI 
{
    bool IsAllowedType(SHADER_RESOURCE_VARIABLE_TYPE varType, UINT32 allowedTypeBits) noexcept;

    UINT32 GetAllowedTypeBits(const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes, UINT32 allowedTypeNum) noexcept;

    // �ж�Shader Type��Pipeline Type�Ƿ���ݣ�����Compute Pipeline�Ͳ�����VS��PS
    bool IsConsistentShaderType(SHADER_TYPE ShaderType, PIPELINE_TYPE PipelineType);

    // ��Shader Type��Ϊ����������Root Tableʱ��ʹ��
    INT32 GetShaderTypePipelineIndex(SHADER_TYPE ShaderType, PIPELINE_TYPE PipelineType);

    // ��ShaderVariableConfig���ҳ�ĳ��ShaderResource��Variable Type��Static��Mutable��Dynamic��
    SHADER_RESOURCE_VARIABLE_TYPE GetShaderVariableType(SHADER_TYPE shaderType, 
                                                        const std::string& name, 
                                                        const struct ShaderVariableConfig& shaderVariableConfig);

    // CachedResourceType to D3D12_DESCRIPTOR_RANGE_TYPE
    D3D12_DESCRIPTOR_RANGE_TYPE GetDescriptorRangeType(BindingResourceType cachedResType);

    D3D12_SHADER_VISIBILITY GetShaderVisibility(SHADER_TYPE ShaderType);
}