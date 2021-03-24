#include "pch.h"
#include "Mesh.h"
#include "GraphicContext.h"

using namespace DirectX;

// TODO:�޸Ĵ���Ķ��㲼��
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

	ULONGLONG bufferSize = m_VertexBufferByteSize + m_IndexBufferByteSize;
	char* buffer = new char[bufferSize];
	std::unique_ptr<char> dataPtrGuard(buffer);	// �����ͷ�bufferָ����ڴ�

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

	m_VertexBufferGPU = d3dUtil::CreateDefaultBuffer(GraphicContext::GetSingleton().Device(),
		GraphicContext::GetSingleton().CommandList(), buffer, bufferSize, m_UploadBuffer);

	m_VertexBufferView.BufferLocation = m_VertexBufferGPU->GetGPUVirtualAddress();
	m_VertexBufferView.StrideInBytes = m_VertexByteStride;
	m_VertexBufferView.SizeInBytes = m_VertexBufferByteSize;
	m_IndexBufferView.BufferLocation = m_VertexBufferGPU->GetGPUVirtualAddress() + m_VertexBufferByteSize;
	m_IndexBufferView.Format = m_IndexFormat;
	m_IndexBufferView.SizeInBytes = m_IndexBufferByteSize;

	// TODO:�ύCommandList��ͬ��CPU��GPU���ͷ�UploadBuffer
}

Mesh::~Mesh()
{
}

void Mesh::UpdateMeshData()
{

}

