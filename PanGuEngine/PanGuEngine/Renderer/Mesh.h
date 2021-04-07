#pragma once
#include "D3D12RHI/GpuBuffer.h"

class Mesh
{
public:
	Mesh(
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
		UINT32* indices);
	~Mesh();

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const { return m_VertexBufferView; }
	D3D12_INDEX_BUFFER_VIEW IndexBufferView()const { return m_IndexBufferView; }
	UINT VertexCount() const { return m_VertexCount; }
	UINT IndexCount() const { return m_IndexCount; }
	UINT GetLayoutIndex() { return m_LayoutIndex; }

private:

	std::shared_ptr<RHI::GpuDefaultBuffer> m_VertexBuffer = nullptr;
	std::shared_ptr<RHI::GpuDefaultBuffer> m_IndexBuffer = nullptr;

	UINT m_VertexByteStride = 0;
	ULONG m_VertexBufferByteSize = 0;
	DXGI_FORMAT m_IndexFormat = DXGI_FORMAT_R32_UINT;
	UINT m_IndexBufferByteSize = 0;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;

	UINT m_VertexCount = 0;
	UINT m_IndexCount = 0;

	UINT m_LayoutIndex;
};

