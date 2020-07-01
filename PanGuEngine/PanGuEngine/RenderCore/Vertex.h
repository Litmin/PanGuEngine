#pragma once
#include "pch.h"

struct Vertex
{
    Vertex(){}
    Vertex(
        const DirectX::XMFLOAT4& color,
        const DirectX::XMFLOAT4& tangentU,
        const DirectX::XMFLOAT3& position,
        const DirectX::XMFLOAT3& normal,
        const DirectX::XMFLOAT2& uv0,
        const DirectX::XMFLOAT2& uv1) :
        Color(color),
        TangentU(tangentU),
        Position(position),
        Normal(normal),
        UV0(uv0),
        UV1(uv1){}

    static Vertex Builder(DirectX::XMFLOAT3& position, DirectX::XMFLOAT4& color)
    {
        return Vertex(static_cast<DirectX::XMFLOAT4>(DirectX::Colors::Transparent), 
            DirectX::XMFLOAT4(0, 0, 0, 0), position, DirectX::XMFLOAT3(0, 0, 0),
            DirectX::XMFLOAT2(0, 0), DirectX::XMFLOAT2(0, 0));
    }

    DirectX::XMFLOAT4 Color;
    DirectX::XMFLOAT4 TangentU;
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT3 Normal;
    DirectX::XMFLOAT2 UV0;
    DirectX::XMFLOAT2 UV1;
};

