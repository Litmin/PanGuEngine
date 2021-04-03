#include "pch.h"
#include "GeometryFactory.h"
#include "GeometryGenerator.h"

using namespace DirectX;

void GeometryFactory::CreateBox(float width, float height, float depth, std::uint32_t numSubdivisions, 
	UINT& vertexCount, std::vector<DirectX::XMFLOAT3>& positions, std::vector<DirectX::XMFLOAT4>& colors, 
	std::vector<DirectX::XMFLOAT3>& normals, std::vector<DirectX::XMFLOAT4>& tangents, std::vector<DirectX::XMFLOAT2>& uvs,
	UINT& indexCount, std::vector<uint16_t>& indices)
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(width, height, depth, numSubdivisions);

	XMFLOAT4 color[6] = { {1.0f, 0.0f, 0.0f, 1.0f},
						  {0.0f, 1.0f, 0.0f, 1.0f}, 
						  {0.0f, 0.0f, 1.0f, 1.0f}, 
						  {1.0f, 1.0f, 0.0f, 1.0f},
						  {1.0f, 0.0f, 1.0f, 1.0f}, 
						  {0.0f, 1.0f, 1.0f, 1.0f} };


	auto& boxVertices = box.Vertices;
	vertexCount = box.Vertices.size();
	UINT8 i = 0;
	for (auto& vertex : boxVertices)
	{
		positions.push_back(vertex.Position);
		colors.push_back(color[i % 6]);
		normals.push_back(vertex.Normal);
		tangents.push_back(DirectX::XMFLOAT4(vertex.TangentU.x, vertex.TangentU.y, vertex.TangentU.z, 1.0f));
		uvs.push_back(vertex.TexC);
		i++;
	}


	indexCount = box.Indices32.size();
	auto& boxIndices = box.GetIndices16();
	for (auto& index : boxIndices)
	{
		indices.push_back(index);
	}
}
