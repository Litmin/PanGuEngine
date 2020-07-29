#pragma once
#include <vector>

class GeometryFactory
{
public:
	static void CreateBox(float width, float height, float depth, std::uint32_t numSubdivisions,
		UINT& vertexCount, std::vector<DirectX::XMFLOAT3>& positions, std::vector<DirectX::XMFLOAT4>& colors, 
		UINT& indexCount, std::vector<uint16_t>& indices);

private:

};
