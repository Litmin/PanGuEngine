#include "pch.h"
#include "GpuRenderTextureDepth.h"


namespace RHI
{

	GpuRenderTextureDepth::GpuRenderTextureDepth(UINT32 width, UINT32 height, DXGI_FORMAT format) :
		GpuRenderTexture(width, height, D3D12_RESOURCE_DIMENSION_TEXTURE2D, format)
	{

	}

	std::shared_ptr<RHI::GpuResourceDescriptor> GpuRenderTextureDepth::CreateDSV()
	{

	}

	std::shared_ptr<RHI::GpuResourceDescriptor> GpuRenderTextureDepth::CreateDepthSRV()
	{

	}

	std::shared_ptr<RHI::GpuResourceDescriptor> GpuRenderTextureDepth::CreateStencilSRV()
	{

	}

}
