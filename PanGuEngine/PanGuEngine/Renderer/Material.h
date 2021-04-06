#pragma once
#include <unordered_map>

namespace RHI
{
    class GpuTexture2D;
    class GpuResourceDescriptor;
    class ShaderResourceBinding;
    class PipelineState;
    class GpuDefaultBuffer;
}


//**********************************************
// 描述一个表面的属性:
//		使用的Shader
//		Shader参数值
//**********************************************
class Material
{
public:
    Material(float metallicFactor, float roughnessFactor, DirectX::XMFLOAT4 baseColorFactor, DirectX::XMFLOAT4 emissiveFactor,
             std::shared_ptr<RHI::GpuTexture2D> baseColorTex, std::shared_ptr<RHI::GpuTexture2D> metallicRoughnessTex,
             std::shared_ptr<RHI::GpuTexture2D> normalTex, std::shared_ptr<RHI::GpuTexture2D> occlusionTex, std::shared_ptr<RHI::GpuTexture2D> emissiveTex);

	void CreateSRB(RHI::PipelineState* PSO);
    RHI::ShaderResourceBinding* GetSRB() { assert(m_SRB != nullptr); return m_SRB.get(); }

private:

    enum ALPHA_MODE
    {
        ALPHAMODE_OPAQUE,
        ALPHAMODE_MASK,
        ALPHAMODE_BLEND
    };
    ALPHA_MODE m_AlphaMode = ALPHAMODE_OPAQUE;

    bool m_DoubleSided = false;

    PBRMaterialConstants m_ConstantsData;

    std::shared_ptr<RHI::GpuTexture2D> m_BaseColorTexture;
    std::shared_ptr<RHI::GpuTexture2D> m_MetallicRoughnessTexture;
    std::shared_ptr<RHI::GpuTexture2D> m_NormalTexture;
    std::shared_ptr<RHI::GpuTexture2D> m_OcclusionTexture;
    std::shared_ptr<RHI::GpuTexture2D> m_EmissiveTexture;

	std::shared_ptr<RHI::GpuResourceDescriptor> m_BaseColorTextureDescriptor;
	std::shared_ptr<RHI::GpuResourceDescriptor> m_MetallicRoughnessTextureDescriptor;
	std::shared_ptr<RHI::GpuResourceDescriptor> m_NormalTextureDescriptor;
	std::shared_ptr<RHI::GpuResourceDescriptor> m_OcclusionTextureDescriptor;
	std::shared_ptr<RHI::GpuResourceDescriptor> m_EmissiveTextureDescriptor;

    std::shared_ptr<RHI::GpuDefaultBuffer> m_ConstantBuffer;
    std::unique_ptr<RHI::ShaderResourceBinding> m_SRB = nullptr;

};
