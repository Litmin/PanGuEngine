#include "pch.h"
#include "PipelineState.h"

namespace RHI
{
	PipelineState::PipelineState(RenderDevice* renderDevice, const PipelineStateDesc& desc) :
		m_RenderDevice{renderDevice},
		m_Desc{desc}
	{
		if (m_Desc.PipelineType == PIPELINE_TYPE_GRAPHIC)
		{
			m_VertexShader = m_Desc.GraphicsPipeline.VertexShader;
			m_PixelShader = m_Desc.GraphicsPipeline.PixelShader;
			m_GeometryShader = m_Desc.GraphicsPipeline.GeometryShader;
			m_HullShader = m_Desc.GraphicsPipeline.HullShader;
			m_DomainShader = m_Desc.GraphicsPipeline.DomainShader;
			
			m_RenderPass = m_Desc.GraphicsPipeline.pRenderPass;
			// TODO: …Ë÷√RenderPass

		}


		// TODO: Compute Pipeline
		// TODO: Mesh Pipeline
	}
}

