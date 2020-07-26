#include "pch.h"
#include "PipelineStateDesc.h"

bool PipelineStateDesc::operator==(const PipelineStateDesc& other) const
{
	return other.shaderPtr == shaderPtr 
		&& other.meshLayoutIndex == meshLayoutIndex 
		&& other.topology == topology;
}
