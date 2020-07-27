#include "pch.h"
#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount, UINT materialCount)
{
    ThrowIfFailed(device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(m_CmdListAlloc.GetAddressOf())));

    //  FrameCB = std::make_unique<UploadBuffer<FrameConstants>>(device, 1, true);
    m_PassCB = std::make_unique<UploadBuffer<PassConstants>>(device, passCount, true);
    //m_PBRMaterialCB = std::make_unique<UploadBuffer<PBRMaterialConstant>>(device, materialCount, true);
    m_ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(device, objectCount, true);
}

FrameResource::~FrameResource()
{

}