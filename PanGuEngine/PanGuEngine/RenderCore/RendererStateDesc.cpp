#include "pch.h"
#include "RendererStateDesc.h"

bool RendererStateDesc::operator==(const RendererStateDesc& other) const
{
	return other.shaderPtr == shaderPtr 
		&& other.inputLayoutIndex == inputLayoutIndex 
		&& other.topology == topology;
}

bool RTStateDesc::operator==(const RTStateDesc& other) const
{
	if (other.depthFormat != depthFormat
		|| other.rtCount != rtCount)
		return false;

	for (UINT i = 0; i < rtCount; ++i)
	{
		if (other.rtFormat[i] != rtFormat[i])
			return false;
	}

	return true;
}
