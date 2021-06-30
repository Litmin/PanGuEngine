#pragma once
#include <vector>

class Mesh;

class GeometryFactory
{
public:
	static std::shared_ptr<Mesh> CreateBox(float width, float height, float depth, std::uint32_t numSubdivisions);

	static std::shared_ptr<Mesh> CreateSphere(float radius, UINT32 sliceCount, UINT32 stackCount);

};
