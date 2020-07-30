#pragma once
#include "UploadBuffer.h"
#include "Material.h"

struct ObjectConstants
{
    DirectX::XMFLOAT4X4 World = MathHelper::Identity4x4();
};


struct PassConstants
{
    // Camera Constant
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


struct FrameResource
{
public:
    FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
    FrameResource(const FrameResource& rhs) = delete;
    FrameResource& operator=(const FrameResource& rhs) = delete;
    ~FrameResource();


    // GPU��������������ResetAllocator������ÿһ֡����Ҫ������CommandAllocator
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CmdListAlloc;

    // GPU��������������ǲ��ܸ���Constant Buffer������ÿ֡Ҳ����һ��
    std::unique_ptr<UploadBuffer<PassConstants>> m_PassCB = nullptr;
    // ���ʳ�����������С�ǲ��̶��ģ������˲�����Ҫ����
    std::unique_ptr<UploadBuffer<ObjectConstants>> m_ObjectCB = nullptr;

    // ÿһ֡��Fence��������鵱ǰ֡�Ƿ�û������
    UINT64 Fence = 0;
};

