#pragma once
#include <vector>

class GeometryFactory
{
public:
	static void CreateBox(float width, float height, float depth, std::uint32_t numSubdivisions,
		UINT& vertexCount, std::vector<DirectX::XMFLOAT3>& positions, std::vector<DirectX::XMFLOAT4>& colors, 
		std::vector<DirectX::XMFLOAT3>& normals, std::vector<DirectX::XMFLOAT4>& tangents, std::vector<DirectX::XMFLOAT2>& uvs,
		UINT& indexCount, std::vector<UINT32>& indices);

private:

};
