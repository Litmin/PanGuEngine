
#include "BRDF.hlsli"
#include "PBR_Common.hlsli"


struct VertexIn
{
    float3 Position  : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float2 uv : TEXCOORD0;
};

struct VertexOut
{
    float4 Position  : SV_POSITION;
    float2 UV : TEXCOORD0;
};

#define NUM_SAMPLES 512u

float2 IntegrateBRDF(float Roughness, float NoV, uint NumSamples)
{
    float3 V;
    V.x = sqrt(1.0 - NoV * NoV); // sin
    V.y = 0.0;
    V.z = NoV; // cos
    const float3 N = float3(0.0, 0.0, 1.0);
    float A = 0.0;
    float B = 0.0;
    for (uint i = 0u; i < NumSamples; i++)
    {
        float2 Xi = Hammersley2D(i, NumSamples);
        float3 H = ImportanceSampleGGX(Xi, Roughness, N);
        float3 L = 2.0 * dot(V, H) * H - V;
        float NoL = saturate(L.z);
        float NoH = saturate(H.z);
        float VoH = saturate(dot(V, H));
        if (NoL > 0.0)
        {
            // https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
            // Also see eq. 92 in  https://google.github.io/filament/Filament.md.html#lighting/imagebasedlights
            // Note that VoH / (NormalDistribution_GGX(H,Roughness) * NoH) term comes from importance sampling
            float G_Vis = 4.0 * SmithGGXVisibilityCorrelated(NoL, NoV, Roughness) * VoH * NoL / NoH;
            float Fc = pow(1.0 - VoH, 5.0);
            A += (1.0 - Fc) * G_Vis;
            B += Fc * G_Vis;
        }
    }
    return float2(A, B) / float(NumSamples);
}

VertexOut VS(VertexIn IN)
{
    VertexOut o;

    o.Position = float4(IN.Position, 1.0);
    o.UV = IN.uv;

    return o;
}

float4 PS(VertexOut IN) : SV_Target
{
    float NdotV = IN.UV.x;
    float linearRoughness = IN.UV.y;
    return float4(IntegrateBRDF(linearRoughness, NdotV, NUM_SAMPLES), 0, 0);
}