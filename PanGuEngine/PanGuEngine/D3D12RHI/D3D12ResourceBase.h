#pragma once


/**
* Direct3D 12�а����е�GPU��Դ����ID3D12Resource��ʾ����Ϊ������Դ�����϶���GPU�е�һ���ڴ棬
* ��ͬ���͵���Դ�ò�ͬ��Descriptor������
*/
class D3D12ResourceBase
{
public:

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_D3d12Resource;

};

