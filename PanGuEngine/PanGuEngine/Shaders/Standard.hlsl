
// Phong Shading Model

// Static
cbuffer cbPerObject : register(b0)
{
    float4x4 ObjectToWorld;
    float4x4 WorldToObject;
};

// Static
cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
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
cbuffer cbLight : register(b2)
{
    float3 LightDir;
    float3 LightColor;
    float LightIntensity;
}

// Mutable
cbuffer cbMaterial : register(b3)
{
    float4 BaseColorFactor;
    float4 EmissiveFactor;
    float  MetallicFactor;
    float  RoughnessFactor;
}

Texture2D BaseColorTex : register(t0);


SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

struct VertexIn
{
    float3 Position  : POSITION;
    //float4 Color : COLOR;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float2 uv : TEXCOORD0;
};

struct VertexOut
{
    float4 Position  : SV_POSITION;
    //float4 Color : COLOR;
    float3 WorldNormal : NORMAL;
    float3 ViewDir : VIEW;
    float2 uv : TEXCOORD0;
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
    //o.Color = IN.Color;
    o.WorldNormal = ObjectToWorldNormal(IN.Normal);
    o.ViewDir = gEyePosW - worldPos.xyz;
    o.uv = IN.uv;

    return o;
}

float4 PS(VertexOut IN) : SV_Target
{
    float4 baseColor = BaseColorTex.Sample(gsamLinearClamp, IN.uv);

    float4 col = float4(0.0f, 0.0f, 0.0f, 1.0f);
    
    float3 lightDir = normalize(LightDir);

    // 环境光
    float3 ambient = 0.05f * LightColor;
    // 漫反射
    float3 diffuse = max(dot(lightDir, IN.WorldNormal), 0.0f) * LightColor;
    // 高光
    float3 specular = pow(max(dot(IN.WorldNormal, normalize(IN.ViewDir)), 0.0f), 32.0f) * LightColor;

    col.rgb = (ambient + diffuse + specular) * baseColor.rgb;

    //col = float4(IN.uv.x, IN.uv.x, IN.uv.x, 1.0f);
    //col = float4(IN.uv.y, IN.uv.y, IN.uv.y, 1.0f);

    return col;
}