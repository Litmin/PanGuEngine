
#ifndef __BRDF_HLSLI__
#define __BRDF_HLSLI__

#ifndef PI
#   define  PI 3.141592653589793
#endif

struct SurfaceInfo
{
    float  PerceptualRoughness;     // Roughness的平方，为了效果更线性
    float3 F0;                      // 0度入射角的反射率
    float3 F90;                     // Grazing Angle的反射率
    float3 DiffuseColor;            // 漫反射颜色，也就是光线折射进物体内部被吸收的比例
};

// 对于金属，BaseColor表示F0；对于非金属，BaseColor表示漫反射颜色
// PhysicalDesc.g是Roughness;PhysicalDesc.b是Metallic
SurfaceInfo GetSurfaceInfo(float4 BaseColor, float4 PhysicalDesc)
{
    SurfaceInfo surfaceInfo;

    // 非金属的F0用一个常量0.04
    float3 noMetalF0 = float3(0.04, 0.04, 0.04);


}




// Lambertian Diffuse
float3 LambertianDiffuse(float3 DiffuseColor)
{
    return DiffuseColor / PI;
}

// Schlick Fresnel Reflection
float3 F_Schlick(float VdotH, float3 F0, float3 F90)
{
    return F0 + (F90 - F0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

// [Beckmann 1963, "The scattering of electromagnetic waves from rough surfaces"]
float D_Beckmann(float a2, float NoH)
{
	float NoH2 = NoH * NoH;
	return exp((NoH2 - 1) / (a2 * NoH2)) / (PI * a2 * NoH2 * NoH2);
}

// GGX尾巴长 效果好！！！！！！
// GGX / Trowbridge-Reitz
// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
float D_GGX(float a2, float NoH)
{
	float d = (NoH * a2 - NoH) * NoH + 1;	// 2 mad
	return a2 / (PI * d * d);					// 4 mul, 1 rcp
}




#endif



