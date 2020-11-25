#pragma once
#include "D3D12DeviceObject.h"

namespace RHI 
{
    /**
    * Direct3D 12�а����е�GPU��Դ����ID3D12Resource��ʾ����Ϊ������Դ�����϶���GPU�е�һ���ڴ棬
    * ��ͬ���͵���Դ�ò�ͬ��Descriptor������
    */
    class D3D12ResourceBase : public D3D12DeviceObject
    {
    public:
        ID3D12Resource* GetD3D12Resource() { return m_D3d12Resource.Get(); }

    protected:
        // ������Դʱʹ�øó�Ա��������Դʱʹ��GetD3D12Resource��������������ü����������ڴ�й¶
        Microsoft::WRL::ComPtr<ID3D12Resource> m_D3d12Resource;
    };

}