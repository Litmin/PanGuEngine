
#include "BRDF.hlsli"
#include "PBR_Common.hlsli"

#ifndef OPTIMIZE_SAMPLES
#   define OPTIMIZE_SAMPLES 1
#endif

// Static
cbuffer cbPerObject
{
    float4x4 ObjectToWorld;
    float4x4 WorldToObject;
};

// Static
cbuffer cbPass
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float4x4 gShadowViewProj;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
};

// Static
cbuffer cbPrefilterEnv
{
    float    g_Roughness;
    float    g_EnvMapDim;
    uint     g_NumSamples;
    float    padding0;
};

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

TextureCube EnvironmentMap;

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
    float3 ObjectPos : POSITION;
};

// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
float3 PrefilterEnvMap(float Roughness, float3 R)
{
    float3 N = R;
    float3 V = R;
    float3 PrefilteredColor = float3(0.0, 0.0, 0.0);
    float TotalWeight = 0.0;
    for (uint i = 0u; i < g_NumSamples; i++)
    {
        float2 Xi = Hammersley2D(i, g_NumSamples);
        float3 H = ImportanceSampleGGX(Xi, Roughness, N);
        float3 L = 2.0 * dot(V, H) * H - V;
        float NoL = clamp(dot(N, L), 0.0, 1.0);
        float VoH = clamp(dot(V, H), 0.0, 1.0);
        if (NoL > 0.0 && VoH > 0.0)
        {
#if OPTIMIZE_SAMPLES
            // https://placeholderart.wordpress.com/2015/07/28/implementation-notes-runtime-environment-map-filtering-for-image-based-lighting/

            float NoH = clamp(dot(N, H), 0.0, 1.0);

            // Probability Distribution Function
            float pdf = max(D_GGX(Roughness, NoH) * NoH / (4.0 * VoH), 0.0001);
            // Slid angle of current smple
            float OmegaS = 1.0 / (float(g_NumSamples) * pdf);
            // Solid angle of 1 pixel across all cube faces
            float OmegaP = 4.0 * PI / (6.0 * g_EnvMapDim * g_EnvMapDim);
            // Do not apply mip bias as this produces result that are not cosistent with the reference
            float MipLevel = (Roughness == 0.0) ? 0.0 : max(0.5 * log2(OmegaS / OmegaP), 0.0);
#else
            float MipLevel = 0.0;
#endif
            PrefilteredColor += EnvironmentMap.SampleLevel(gsamLinearWrap, L, MipLevel).rgb * NoL;
            TotalWeight += NoL;
        }
    }
    return PrefilteredColor / TotalWeight;
}

VertexOut VS(VertexIn IN)
{
    VertexOut o;

    o.ObjectPos = IN.Position;

    o.Position = mul(float4(IN.Position, 1.0), gViewProj);

    return o;
}

float4 PS(VertexOut IN) : SV_Target
{
    float3 R = normalize(IN.ObjectPos);
    return float4(PrefilterEnvMap(g_Roughness, R), 0);
}