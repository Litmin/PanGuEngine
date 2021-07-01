

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

SamplerState gsamPointWrap        : register(s0);
SamplerState gsamPointClamp       : register(s1);
SamplerState gsamLinearWrap       : register(s2);
SamplerState gsamLinearClamp      : register(s3);
SamplerState gsamAnisotropicWrap  : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

TextureCube Skybox;

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


VertexOut VS(VertexIn IN)
{
    VertexOut o;

    o.ObjectPos = IN.Position;

    o.Position = mul(float4(IN.Position + gEyePosW, 1.0), gViewProj).xyww;

    return o;
}

float4 PS(VertexOut IN) : SV_Target
{
    return pow(Skybox.Sample(gsamLinearWrap, IN.ObjectPos), 1/2.2);
}