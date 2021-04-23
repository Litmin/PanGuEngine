

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
cbuffer cbLight
{
    float3 LightDir;
    float LightIntensity;
    float3 LightColor;
}

// Mutable
cbuffer cbMaterial
{
    float4 BaseColorFactor;
    float4 EmissiveFactor;
    float  MetallicFactor;
    float  RoughnessFactor;
}

Texture2D BaseColorTex;
Texture2D EmissiveTex;

Texture2D ShadowMap;


SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);
SamplerComparisonState gsamShadow : register(s6);

struct VertexIn
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float4 Tangent  : TANGENT;
    float2 uv       : TEXCOORD0;
};

struct VertexOut
{
    float4 Position     : SV_POSITION;
    float3 WorldNormal  : NORMAL;
    float3 ViewDir      : VIEW;
    float2 uv           : TEXCOORD0;
    float4 ShadowPos    : TEXCOORD1;
};

// 变换法线要使用逆转置矩阵
float3 ObjectToWorldNormal(float3 normal)
{
    return normalize(mul((float3x3)WorldToObject, normal));
}

VertexOut VS(VertexIn IN)
{
    VertexOut o;

    float4 worldPos = mul(float4(IN.Position, 1.0f), ObjectToWorld);
    o.Position = mul(worldPos, gViewProj);
    o.WorldNormal = ObjectToWorldNormal(IN.Normal);
    o.ViewDir = gEyePosW - worldPos.xyz;
    o.uv = IN.uv;

    o.ShadowPos = mul(worldPos, gShadowViewProj);

    return o;
}

float PCSS(float2 shadowMapUV, float curDepth)
{
    float shadowFactor = 1.0;
    const float lightSize = 10.0;
    uint width, height, numMips;
    ShadowMap.GetDimensions(0, width, height, numMips);
    float dx = 1.0f / (float)width;

    // 1.Blocker Search 计算Blocker的平均深度
    float blockerDepth = 0.0;
    float blockerNum = 0;
    for (int i = -3; i <= 3; ++i)
    {
        for (int j = -3; j <= 3; ++j)
        {
            float shadowMapDepth = ShadowMap.Sample(gsamLinearWrap, shadowMapUV + dx * 5 * float2(i, j)).r;
            if (curDepth > shadowMapDepth)
            {
                blockerDepth += shadowMapDepth;
                blockerNum += 1.0;
            }
        }
    }

    if (blockerNum >= 1.0)
    {
        blockerDepth /= blockerNum;

        // 2.计算Filter大小
        float penumbra = (curDepth - blockerDepth) * lightSize / blockerDepth;

        // 3.PCF
        shadowFactor = 0.0;
        for (int i = -3; i <= 3; ++i)
        {
            for (int j = -3; j <= 3; ++j)
            {
                // 使用硬件支持的PCF, LevelZero表示mip0
                shadowFactor += ShadowMap.SampleCmpLevelZero(gsamShadow, shadowMapUV + dx * penumbra * float2(i, j), curDepth).r;
            }
        }
        shadowFactor /= 49.0;
    }

    return shadowFactor;
}

float4 PS(VertexOut IN) : SV_Target
{
    float4 baseColor = BaseColorTex.Sample(gsamLinearWrap, IN.uv);
    float4 emissiveColor = EmissiveTex.Sample(gsamLinearWrap, IN.uv);

    float4 col = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    float3 lightDir = -normalize(LightDir);

    // 环境光
    float3 ambient = 0.2f * LightColor;
    // 漫反射
    float3 diffuse = max(dot(lightDir, IN.WorldNormal), 0.0f) * LightColor;
    // 高光
    float3 specular = pow(max(dot(IN.WorldNormal, normalize(IN.ViewDir)), 0.0f), 32.0f) * LightColor;

    // Shadow Map
    float3 shadowPos = IN.ShadowPos.xyz / IN.ShadowPos.w;
    float2 shadowMapUV = shadowPos.xy * 0.5 + 0.5;
    shadowMapUV.y = 1 - shadowMapUV.y;
    float curDepth = shadowPos.z;

    // PCSS
    //float shadowFactor = PCSS(shadowMapUV, curDepth);

    // 使用硬件支持的PCF
    float depthInShadowMap = ShadowMap.Sample(gsamLinearWrap, shadowMapUV).r;
    // Bias,使用硬件Bias
    //depthInShadowMap += 0.01f;
    float shadowFactor = 1.0f;
    if (curDepth > depthInShadowMap)
        shadowFactor = 0.0f;

    col.rgb = (ambient + (diffuse + specular) * shadowFactor) * baseColor.rgb + emissiveColor.rgb;

    return col;
}