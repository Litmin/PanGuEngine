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
    float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut VS(VertexIn IN)
{
    VertexOut o;

    // Transform to homogeneous clip space.
    float4 posW = mul(float4(IN.PosL, 1.0f), gWorld);
    o.PosH = mul(posW, gViewProj);

    // Just pass vertex color into the pixel shader.
    o.Color = IN.Color;

    return o;
}

float4 PS(VertexOut IN) : SV_Target
{
    return IN.Color;
}