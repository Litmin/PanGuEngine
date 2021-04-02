#include "pch.h"
#include "Mesh.h"
#include "GraphicContext.h"

using namespace RHI;

// TODO:修改传入的顶点布局
Mesh::Mesh(
	UINT vertexCount, 
	DirectX::XMFLOAT3* positions, 
	DirectX::XMFLOAT4* colors, 
	DirectX::XMFLOAT3* normals, 
	DirectX::XMFLOAT4* tangents, 
	DirectX::XMFLOAT2* uv0, 
	DirectX::XMFLOAT2* uv1, 
	DirectX::XMFLOAT2* uv2, 
	DirectX::XMFLOAT2* uv3,
	UINT indexCount,
	uint16_t* indices) :
	m_VertexCount(vertexCount),
	m_IndexCount(indexCount)
{
	m_LayoutIndex = GraphicContext::GetSingleton().GetInputLayoutIndex(
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
	
	m_IndexBufferByteSize = indexCount * sizeof(std::uint16_t);

	ULONGLONG bufferSize = (ULONGLONG)m_VertexBufferByteSize + m_IndexBufferByteSize;
	char* buffer = new char[bufferSize];
	std::unique_ptr<char> dataPtrGuard(buffer);	// 用来释放buffer的内存

	UINT offset = 0;
	auto vertBufferCopy = [&](char* buffer, char* ptr, UINT size, UINT& offset) -> void
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
	vertBufferCopy(buffer, reinterpret_cast<char*>(positions), 12, offset);
	vertBufferCopy(buffer, reinterpret_cast<char*>(colors), 16, offset);
	vertBufferCopy(buffer, reinterpret_cast<char*>(normals), 12, offset);
	vertBufferCopy(buffer, reinterpret_cast<char*>(tangents), 16, offset);
	vertBufferCopy(buffer, reinterpret_cast<char*>(uv0), 8, offset);
	vertBufferCopy(buffer, reinterpret_cast<char*>(uv1), 8, offset);
	vertBufferCopy(buffer, reinterpret_cast<char*>(uv2), 8, offset);
	vertBufferCopy(buffer, reinterpret_cast<char*>(uv3), 8, offset);

	char* indexBufferStart = buffer + m_VertexBufferByteSize;
	memcpy(indexBufferStart, indices, indexCount * sizeof(std::uint16_t));

	m_VertexBuffer = std::make_shared<GpuDefaultBuffer>(vertexCount, m_VertexByteStride, buffer);
	m_IndexBuffer = std::make_shared<GpuDefaultBuffer>(indexCount, sizeof(std::uint16_t), indexBufferStart);
	m_VertexBufferView = m_VertexBuffer->CreateVBV();
	m_IndexBufferView = m_IndexBuffer->CreateIBV();
}

Mesh::~Mesh()
{
}


