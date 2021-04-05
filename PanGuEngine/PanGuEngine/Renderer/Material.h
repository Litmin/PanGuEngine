#pragma once
#include <unordered_map>

namespace RHI
{
    class GpuTexture2D;

    class ShaderResourceBinding;
}


//**********************************************
// 描述一个表面的属性:
//		使用的Shader
//		Shader参数值
//**********************************************
struct Material
{
    enum ALPHA_MODE
    {
        ALPHAMODE_OPAQUE,
        ALPHAMODE_MASK,
        ALPHAMODE_BLEND
    };
    ALPHA_MODE AlphaMode = ALPHAMODE_OPAQUE;

    bool DoubleSided = false;

    float  MetallicFactor = 1.0f;
    float  RoughnessFactor = 1.0f;
    DirectX::XMFLOAT4 BaseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT4 EmissiveFactor = { 1.0f, 1.0f, 1.0f, 1.0f };

    std::shared_ptr<RHI::GpuTexture2D> pBaseColorTexture;
    std::shared_ptr<RHI::GpuTexture2D> pMetallicRoughnessTexture;
    std::shared_ptr<RHI::GpuTexture2D> pNormalTexture;
    std::shared_ptr<RHI::GpuTexture2D> pOcclusionTexture;
    std::shared_ptr<RHI::GpuTexture2D> pEmissiveTexture;

    std::unique_ptr<RHI::ShaderResourceBinding> m_SRB;
};
