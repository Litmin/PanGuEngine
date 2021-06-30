#include "pch.h"
#include "GeometryFactory.h"
#include "GeometryGenerator.h"
#include "Renderer/Mesh.h"

using namespace DirectX;


std::shared_ptr<Mesh> GeometryFactory::CreateBox(float width, float height, float depth, std::uint32_t numSubdivisions)
{
	UINT vertexCount, indexCount;
	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT4> colors;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT4> tangents;
	std::vector<XMFLOAT2> uvs;
	std::vector<UINT32> indices;

	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(width, height, depth, numSubdivisions);

	XMFLOAT4 color[6] = { {1.0f, 0.0f, 0.0f, 1.0f},
						  {0.0f, 1.0f, 0.0f, 1.0f},
						  {0.0f, 0.0f, 1.0f, 1.0f},
						  {1.0f, 1.0f, 0.0f, 1.0f},
						  {1.0f, 0.0f, 1.0f, 1.0f},
						  {0.0f, 1.0f, 1.0f, 1.0f} };

	XMFLOAT4 gold = { 0.9843f, 0.73725f, 0.0196f, 1.0f };


	auto& boxVertices = box.Vertices;
	vertexCount = box.Vertices.size();
	UINT8 i = 0;
	for (auto& vertex : boxVertices)
	{
		positions.push_back(vertex.Position);
		//colors.push_back(color[i % 6]);
		colors.push_back(gold);
		normals.push_back(vertex.Normal);
		tangents.push_back(DirectX::XMFLOAT4(vertex.TangentU.x, vertex.TangentU.y, vertex.TangentU.z, 1.0f));
		uvs.push_back(vertex.TexC);
		i++;
	}


	indexCount = box.Indices32.size();
	auto& boxIndices = box.Indices32;
	for (auto& index : boxIndices)
	{
		indices.push_back(index);
	}

	std::shared_ptr<Mesh> boxMesh = std::make_shared<Mesh>(vertexCount, (const float*)positions.data(), nullptr,
		(const float*)normals.data(), nullptr, (const float*)uvs.data(), nullptr, nullptr, nullptr,
		indexCount, boxIndices.data());

	return boxMesh;
}

std::shared_ptr<Mesh> GeometryFactory::CreateSphere(float radius, UINT32 sliceCount, UINT32 stackCount)
{
	UINT vertexCount, indexCount;
	std::vector<XMFLOAT3> positions;
	std::vector<XMFLOAT4> colors;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT4> tangents;
	std::vector<XMFLOAT2> uvs;
	std::vector<UINT32> indices;

	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateSphere(radius, sliceCount, stackCount);

	XMFLOAT4 color[6] = { {1.0f, 0.0f, 0.0f, 1.0f},
						  {0.0f, 1.0f, 0.0f, 1.0f},
						  {0.0f, 0.0f, 1.0f, 1.0f},
						  {1.0f, 1.0f, 0.0f, 1.0f},
						  {1.0f, 0.0f, 1.0f, 1.0f},
						  {0.0f, 1.0f, 1.0f, 1.0f} };

	XMFLOAT4 gold = { 0.9843f, 0.73725f, 0.0196f, 1.0f };


	auto& boxVertices = box.Vertices;
	vertexCount = box.Vertices.size();
	UINT8 i = 0;
	for (auto& vertex : boxVertices)
	{
		positions.push_back(vertex.Position);
		//colors.push_back(color[i % 6]);
		colors.push_back(gold);
		normals.push_back(vertex.Normal);
		tangents.push_back(DirectX::XMFLOAT4(vertex.TangentU.x, vertex.TangentU.y, vertex.TangentU.z, 1.0f));
		uvs.push_back(vertex.TexC);
		i++;
	}


	indexCount = box.Indices32.size();
	auto& boxIndices = box.Indices32;
	for (auto& index : boxIndices)
	{
		indices.push_back(index);
	}

	std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>(vertexCount, (const float*)positions.data(), nullptr,
		(const float*)normals.data(), nullptr, (const float*)uvs.data(), nullptr, nullptr, nullptr,
		indexCount, boxIndices.data());

	return sphereMesh;
}
