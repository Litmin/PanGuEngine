
#ifndef __BRDF_HLSLI__
#define __BRDF_HLSLI__

#ifndef PI
#   define  PI 3.141592653589793
#endif

struct SurfaceInfo
{
    float  PerceptualRoughness;     // 感知粗糙度，制作人员调的参数，Shader计算用的粗糙度是感知粗糙度的平方，目的是使效果变化更线性
    float3 F0;                      // 0度入射角的反射率
    float3 F90;                     // Grazing Angle的反射率
    float3 DiffuseColor;            // 漫反射颜色，也就是光线折射进物体内部被吸收的比例
};

struct AngularInfo
{
    float NdotL;   // cos angle between normal and light direction
    float NdotV;   // cos angle between normal and view direction
    float NdotH;   // cos angle between normal and half vector
    float LdotH;   // cos angle between light direction and half vector
    float VdotH;   // cos angle between view direction and half vector
};

// 对于金属，BaseColor表示F0；对于非金属，BaseColor表示漫反射颜色
// PhysicalDesc.g是Roughness;PhysicalDesc.b是Metallic
SurfaceInfo GetSurfaceInfo(float4 BaseColor, float4 PhysicalDesc)
{
    SurfaceInfo surfaceInfo;

    float metallic = PhysicalDesc.b;

    // 非金属的F0用一个常量0.04
    float3 noMetalF0 = float3(0.04, 0.04, 0.04);

    surfaceInfo.PerceptualRoughness = clamp(PhysicalDesc.g, 0.0, 1.0);
    
    surfaceInfo.DiffuseColor = BaseColor.rgb * (float3(1.0, 1.0, 1.0) - f0) * (1.0 - Metallic);

    surfaceInfo.F0 = lerp(f0, BaseColor.rgb, metallic);
    float reflectance = max(max(surfaceInfo.F0.r, surfaceInfo.F0.g), surfaceInfo.F0.b);
    // Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
    surfaceInfo.F90 = clamp(reflectance * 50.0, 0.0, 1.0) * float3(1.0, 1.0, 1.0);
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

void BRDF(AngularInfo angularInfo, SurfaceInfo surfaceInfo, out float3 diffuseContrib, out float3 specContrib)
{
    diffuseContrib = float3(0.0, 0.0, 0.0);
    specContrib = float3(0.0, 0.0, 0.0);

    // It is not a mistake that AlphaRoughness = PerceptualRoughness ^ 2 and that
    // SmithGGXVisibilityCorrelated and NormalDistribution_GGX then use a2 = AlphaRoughness ^ 2.
    // See eq. 3 in https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
    float AlphaRoughness = surfaceInfo.PerceptualRoughness * surfaceInfo.PerceptualRoughness;

    float F = F_Schlick(angularInfo.VdotH, surfaceInfo.F0, surfaceInfo.F90);
    float D = D_GGX(AlphaRoughness, angularInfo.NdotH);

    diffuseContrib = (1.0 - F) * LambertianDiffuse(surfaceInfo.DiffuseColor);
    specContrib = F * D;
}


#endif



