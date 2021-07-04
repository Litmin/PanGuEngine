#include "pch.h"
#include "Mesh.h"
#include "VertexFactory.h"

using namespace RHI;

// TODO:修改传入的顶点布局
// 使用array of structure,每个顶点个各种属性连续放在一起
Mesh::Mesh(
	UINT vertexCount, 
	const float* positions, 
	const float* colors, 
	const float* normals, 
	const float* tangents, 
	const float* uv0, 
	const float* uv1, 
	const float* uv2, 
	const float* uv3,
	UINT indexCount,
	UINT32* indices) :
	m_VertexCount(vertexCount),
	m_IndexCount(indexCount)
{
	m_LayoutIndex = VertexFactory::GetSingleton().GetInputLayoutIndex(
		colors != nullptr,
		normals != nullptr,
		tangents != nullptr,
		uv0 != nullptr,
		uv1 != nullptr,
		uv2 != nullptr,
		uv3 != nullptr
	);

	m_VertexByteStride = 0;
	if (positions) m_VertexByteStride += 12;
	if (colors) m_VertexByteStride += 16;
	if (normals) m_VertexByteStride += 12;
	if (tangents) m_VertexByteStride += 16;
	if (uv0) m_VertexByteStride += 8;
	if (uv1) m_VertexByteStride += 8;
	if (uv2) m_VertexByteStride += 8;
	if (uv3) m_VertexByteStride += 8;

	m_VertexBufferByteSize = m_VertexByteStride * vertexCount;
	
	m_IndexBufferByteSize = indexCount * sizeof(UINT32);

	ULONGLONG bufferSize = (ULONGLONG)m_VertexBufferByteSize + m_IndexBufferByteSize;
	char* buffer = new char[bufferSize];
	std::unique_ptr<char> dataPtrGuard(buffer);	// 用来释放buffer的内存

	UINT offset = 0;
	auto vertBufferCopy = [&](char* buffer, const char* ptr, UINT size, UINT& offset) -> void
	{
		if (ptr)
		{
			for (UINT i = 0; i < vertexCount; ++i)
			{
				memcpy(buffer + i * m_VertexByteStride + offset, ptr + size * i, size);
			}
			offset += size;
		}
	};
	vertBufferCopy(buffer, reinterpret_cast<const char*>(positions), 12, offset);
	vertBufferCopy(buffer, reinterpret_cast<const char*>(colors), 16, offset);
	vertBufferCopy(buffer, reinterpret_cast<const char*>(normals), 12, offset);
	vertBufferCopy(buffer, reinterpret_cast<const char*>(tangents), 16, offset);
	vertBufferCopy(buffer, reinterpret_cast<const char*>(uv0), 8, offset);
	vertBufferCopy(buffer, reinterpret_cast<const char*>(uv1), 8, offset);
	vertBufferCopy(buffer, reinterpret_cast<const char*>(uv2), 8, offset);
	vertBufferCopy(buffer, reinterpret_cast<const char*>(uv3), 8, offset);

	char* indexBufferStart = buffer + m_VertexBufferByteSize;
	memcpy(indexBufferStart, indices, indexCount * sizeof(UINT32));

	m_VertexBuffer = std::make_shared<GpuDefaultBuffer>(vertexCount, m_VertexByteStride, buffer);
	m_IndexBuffer = std::make_shared<GpuDefaultBuffer>(indexCount, sizeof(UINT32), indexBufferStart);
	m_VertexBufferView = m_VertexBuffer->CreateVBV();
	m_IndexBufferView = m_IndexBuffer->CreateIBV();
}

Mesh::~Mesh()
{

}


