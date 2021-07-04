

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

#ifndef NUM_PHI_SAMPLES
#   define NUM_PHI_SAMPLES 64
#endif

#ifndef NUM_THETA_SAMPLES
#   define NUM_THETA_SAMPLES 32
#endif

VertexOut VS(VertexIn IN)
{
    VertexOut o;

    o.ObjectPos = IN.Position;

    o.Position = mul(float4(IN.Position, 1.0), gViewProj);

    return o;
}

float4 PS(VertexOut IN) : SV_Target
{
    const float PI = 3.141592653;

    float3 N = normalize(IN.ObjectPos);     // Y
    // 跟Billboard一样的算法,先有鸡还是先有蛋
    float3 up = float3(0, 1, 0);
    //float3 up = abs(N.y) < 0.999f ? float3(0.0, 1.0, 0.0) : float3(0.0, 0.0, 1.0);
    float3 right = normalize(cross(up, N)); // X
    up = normalize(cross(N, right));        // Z

    float3 irradiance = float3(0, 0, 0);
    float sampleDelta = 0.025;
    float numSamples = 0;

    for (float phi = 0; phi < 2 * PI; phi += sampleDelta)
    {
        for (float theta = 0; theta < 0.5 * PI; theta += sampleDelta)
        {
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // 转到世界空间
            float3 sampleDir = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += EnvironmentMap.Sample(gsamLinearWrap, sampleDir).rgb * sin(theta) * cos(theta);

            numSamples += 1.0;
        }
    }

    return float4(PI * irradiance / numSamples, 1.0);
}