cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

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

struct VertexIn
{
    float3 Position  : POSITION;
    float4 Color : COLOR;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT;
    float2 uv : TEXCOORD0;
};

struct VertexOut
{
    float4 Position  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn IN)
{
    VertexOut o;

    float4 worldPos = mul(float4(IN.Position, 1.0f), gWorld);
    o.Position = mul(worldPos, gViewProj);
    o.Color = IN.Color;

    return o;
}

float4 PS(VertexOut IN) : SV_Target
{
    return IN.Color;
}