#pragma once

/*
常量缓冲区还有个要求，就是大小必为硬件的最小分配空间（256B）的整数倍。(TODO: 每个Element256字节对齐还是整个Constant Buffer256字节对齐？？？)
在HLSL中，常量缓冲区会以补齐填充（padding）的方式，将其中的元素都包装为4D向量，但一个单独的元素却不能被分开并横跨两个4D向量。

cbuffer cb : register(b0)
{
    float3 Pos;
    float3 Dir;
};

可能会认为Pack成这样
vector1： (Pos.x, Pos.y, Pos.z, Dir.x,)
vector2 : (Dir.y, Dir.z, empty, empty)

然而，这种方法却把dir元素分置在两个4D向量之中，这在HLSL的规则中是不允许的。因此必须按下列方式封装在着色器内存中：
vector1: (Pos.x, Pos.y, Pos.z, empty)
vector2: (Dir.x, Dir.y, Dir.z, empty)

如果没有留意这些封装规则，而盲目调用memcpy函数以复制字节的方式将数据写入常量缓冲区里，那么将以上面所述的第一种错误情景而告终，而常量数据也将被错误的赋值为：
vector1： (Pos.x, Pos.y, Pos.z, Dir.x,)
vector2 : (Dir.y, Dir.z, empty, empty)

如此说来，我们必须依照HLSL的封装规则以“填充”变量的方式来定义C++结构体，以此使其中的数据元素可正确复制给HLSL中的常量。

所以，我们必须用显式填充的方法来定义HLSL和C++中的结构体！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！


数组的处理方式与上文所述不同。数组中的每个元素都会被存于一个具有4个分量的向量之中:
float2 TexOffsets[8];

这个数组相当于：
float4 TexOffsets[8];

这浪费了很多存储空间，可以通过强制类型转换以及额外的地址计算指令来更有效利用内存
float4 array[4];
static float2 aggressivePackArray[8] = (float2[8])array;
*/

struct PerDrawConstants
{
    DirectX::XMFLOAT4X4 ObjectToWorld = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 WorldToObject = MathHelper::Identity4x4();
};


struct PerPassConstants
{
    DirectX::XMFLOAT4X4 View = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvView = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 Proj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 ViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT4X4 InvViewProj = MathHelper::Identity4x4();
    DirectX::XMFLOAT3 EyePosW = { 0.0f, 0.0f, 0.0f };
    float cbPerObjectPad1 = 0.0f;
    DirectX::XMFLOAT2 RenderTargetSize = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 InvRenderTargetSize = { 0.0f, 0.0f };
    float NearZ = 0.0f;
    float FarZ = 0.0f;
    float TotalTime = 0.0f;
    float DeltaTime = 0.0f;
};


// 目前只实现一个方向光
struct LightConstants
{
    DirectX::XMFLOAT3 LightDir = { 0.0f, 0.0f, 0.0f };
    float LightIntensity = 1.0f;
    DirectX::XMFLOAT3 LightColor = { 0.0f, 0.0f, 0.0f };
};

struct MaterialConstants
{
    float AmbientStrength;
};

