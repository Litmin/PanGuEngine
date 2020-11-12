#pragma once

namespace RHI 
{
    /**
    * Direct3D 12中把所有的GPU资源都用ID3D12Resource表示，因为所有资源本质上都是GPU中的一段内存，
    * 不同类型的资源用不同的Descriptor来描述
    */
    class D3D12ResourceBase
    {
    public:
        D3D12ResourceBase()
        {}

        ID3D12Resource* GetD3D12Resource() { return m_D3d12Resource.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D12Resource> m_D3d12Resource;
    };

}