#include "pch.h"
#include "GeometryFactory.h"
#include "GeometryGenerator.h"

using namespace DirectX;

void GeometryFactory::CreateBox(float width, float height, float depth, std::uint32_t numSubdivisions, 
	UINT& vertexCount, std::vector<DirectX::XMFLOAT3>& positions, std::vector<DirectX::XMFLOAT4>& colors, 
	UINT& indexCount, std::vector<uint16_t>& indices)
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(width, height, depth);

	XMFLOAT4 color(1.0f, 0.0f, 0.0f, 1.0f);

	auto& boxVertices = box.Vertices;
	vertexCount = box.Vertices.size();
	for (auto& vertex : boxVertices)
	{
		positions.push_back(vertex.Position);
		colors.push_back(color);
	}


	indexCount = box.Indices32.size();
	auto& boxIndices = box.GetIndices16();
	for (auto& index : indices)
	{
		indices.push_back(index);
	}
}
