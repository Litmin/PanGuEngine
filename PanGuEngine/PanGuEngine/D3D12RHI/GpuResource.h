#pragma once

namespace RHI 
{
    /**
    * GPU��������Դ�Ļ���
    * GPU��Դ������
    *   GpuBuffer��StructuredBuffer��ByteAddressBuffer��IndirectArgsBuffer��ReadbackBuffer��TypedBuffer
    *   PixelBuffer��ColorBuffer��DepthBuffer
    *   UploadBuffer
    *   Texture��
    * GpuResource�̳�enable_shared_from_this����Ϊ���ڲ��ᴴ��GpuResourceView��GpuResourceView��ӵ��GpuResource������Ȩ������֤ʹ��GpuResourceViewʱ��GpuResource�����ͷ�
    */
    class GpuResource : std::enable_shared_from_this<GpuResource>
    {
    public:

    protected:

    };

}