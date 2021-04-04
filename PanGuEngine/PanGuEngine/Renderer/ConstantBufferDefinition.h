#pragma once

/*
�������������и�Ҫ�󣬾��Ǵ�С��ΪӲ������С����ռ䣨256B������������(TODO: ÿ��Element256�ֽڶ��뻹������Constant Buffer256�ֽڶ��룿����)
��HLSL�У��������������Բ�����䣨padding���ķ�ʽ�������е�Ԫ�ض���װΪ4D��������һ��������Ԫ��ȴ���ܱ��ֿ����������4D������

cbuffer cb : register(b0)
{
    float3 Pos;
    float3 Dir;
};

���ܻ���ΪPack������
vector1�� (Pos.x, Pos.y, Pos.z, Dir.x,)
vector2 : (Dir.y, Dir.z, empty, empty)

Ȼ�������ַ���ȴ��dirԪ�ط���������4D����֮�У�����HLSL�Ĺ������ǲ�����ġ���˱��밴���з�ʽ��װ����ɫ���ڴ��У�
vector1: (Pos.x, Pos.y, Pos.z, empty)
vector2: (Dir.x, Dir.y, Dir.z, empty)

���û��������Щ��װ���򣬶�äĿ����memcpy�����Ը����ֽڵķ�ʽ������д�볣�����������ô�������������ĵ�һ�ִ����龰�����գ�����������Ҳ��������ĸ�ֵΪ��
vector1�� (Pos.x, Pos.y, Pos.z, Dir.x,)
vector2 : (Dir.y, Dir.z, empty, empty)

���˵�������Ǳ�������HLSL�ķ�װ�����ԡ���䡱�����ķ�ʽ������C++�ṹ�壬�Դ�ʹ���е�����Ԫ�ؿ���ȷ���Ƹ�HLSL�еĳ�����

���ԣ����Ǳ�������ʽ���ķ���������HLSL��C++�еĽṹ�壡��������������������������������������������������������������������������������������������������������


����Ĵ���ʽ������������ͬ�������е�ÿ��Ԫ�ض��ᱻ����һ������4������������֮��:
float2 TexOffsets[8];

��������൱�ڣ�
float4 TexOffsets[8];

���˷��˺ܶ�洢�ռ䣬����ͨ��ǿ������ת���Լ�����ĵ�ַ����ָ��������Ч�����ڴ�
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


// Ŀǰֻʵ��һ�������
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

