#include "pch.h"
#include "GpuResource.h"
#include "RenderDevice.h"


namespace RHI
{

	GpuResource::~GpuResource()
	{
		RenderDevice::GetSingleton().SafeReleaseDeviceObject(m_pResource);
	}

}
