#include "pch.h"
#include "GraphicContext.h"
#include "SceneManager.h"
using namespace std;

// 不要再头文件中定义常量,编译器会认为它们是不同的符号常量，为每个编译单元分别分配存储空间
const int gNumFrameResources = 3;

GraphicContext::GraphicContext(
	ID3D12Device* device, 
	ID3D12GraphicsCommandList* commandList,
	ID3D12CommandAllocator* commandAllocator,
	ID3D12CommandQueue* commandQueue,
	ID3D12Fence* fence,
	UINT CbvSrvUavDescriptorSize) :
	m_Device(device),
	m_CommandList(commandList),
	m_CommandAllocator(commandAllocator),
	m_CommandQueue(commandQueue),
	m_Fence(fence),
	m_CbvSrvUavDescriptorSize(CbvSrvUavDescriptorSize)
{
}

GraphicContext::~GraphicContext()
{
}

void GraphicContext::ResetCommandList()
{
	m_CommandList->Reset(m_CommandAllocator, nullptr);
}

void GraphicContext::ExecuteCommandList()
{
	ThrowIfFailed(m_CommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_CommandList };
	m_CommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
}

void GraphicContext::Update()
{
	// Cycle through the circular frame resource array.
	m_CurrFrameResourceIndex = (m_CurrFrameResourceIndex + 1) % gNumFrameResources;
	m_CurrFrameResource = m_FrameResources[m_CurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (m_CurrFrameResource->Fence != 0 && m_Fence->GetCompletedValue() < m_CurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(m_Fence->SetEventOnCompletion(m_CurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

UINT GraphicContext::GetInputLayoutIndex(
	bool hasColor, 
	bool hasNormal, 
	bool hasTangent, 
	bool hasuv0, 
	bool hasuv1, 
	bool hasuv2, 
	bool hasuv3)
{
	USHORT mask = 0;
	if (hasColor)
		mask |= (1 << 0);
	if (hasNormal)
		mask |= (1 << 1);
	if (hasTangent)
		mask |= (1 << 2);
	if (hasuv0)
		mask |= (1 << 3);
	if (hasuv1)
		mask |= (1 << 4);
	if (hasuv2)
		mask |= (1 << 5);
	if (hasuv3)
		mask |= (1 << 6);

	// TODO:Why&&
	auto&& ite = m_InputLayoutMap.find(mask);
	if (ite == m_InputLayoutMap.end())
	{
		unique_ptr<vector<D3D12_INPUT_ELEMENT_DESC>> desc = make_unique<vector<D3D12_INPUT_ELEMENT_DESC>>();
		GenerateInputElementDesc(*desc.get(), hasColor, hasNormal, hasTangent, hasuv0, hasuv1, hasuv2, hasuv3);

		UINT index = m_InputLayouts.size();
		m_InputLayoutMap[mask] = index;

		m_InputLayouts.push_back(move(desc));
		return index;
	}

	return ite->second;
}

std::vector<D3D12_INPUT_ELEMENT_DESC>* GraphicContext::GetInputElementDesc(UINT index)
{
	return m_InputLayouts[index].get();
}

void GraphicContext::GenerateInputElementDesc(
	std::vector<D3D12_INPUT_ELEMENT_DESC>& desc, 
	bool hasColor, 
	bool hasNormal, 
	bool hasTangent, 
	bool hasuv0, 
	bool hasuv1, 
	bool hasuv2, 
	bool hasuv3)
{
	desc.reserve(8);
	desc.push_back(
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA , 0 }
	);
	UINT offset = 12;

	if (hasColor)
	{
		desc.push_back(
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 16;
	}
	else
	{
		desc.push_back(
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
	if (hasNormal)
	{
		desc.push_back(
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 12;
	}
	else
	{
		desc.push_back(
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
	if (hasTangent)
	{
		desc.push_back(
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 16;
	}
	else
	{
		desc.push_back(
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
	if (hasuv0)
	{
		desc.push_back(
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 8;
	}
	else
	{
		desc.push_back(
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
	if (hasuv1)
	{
		desc.push_back(
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 8;
	}
	else
	{
		desc.push_back(
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
	if (hasuv2)
	{
		desc.push_back(
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 8;
	}
	else
	{
		desc.push_back(
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
	if (hasuv3)
	{
		desc.push_back(
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT, 0, offset, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
		offset += 8;
	}
	else
	{
		desc.push_back(
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		);
	}
}

	ID3D12PipelineState* GraphicContext::GetPSO(const RendererStateDesc& rendererStateDesc, RTStateDesc& rtStateDesc)
	{
		auto&& ite = m_PSOs.find(pair<RTStateDesc, RendererStateDesc>(rtStateDesc, rendererStateDesc));
		if (ite == m_PSOs.end())
		{
			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

			// Input Layout
			std::vector<D3D12_INPUT_ELEMENT_DESC>* inputLayout = GetInputElementDesc(rendererStateDesc.inputLayoutIndex);
			psoDesc.InputLayout = { inputLayout->data(), (UINT)inputLayout->size() };
			// Shader
			rendererStateDesc.shaderPtr->SetPSODesc(&psoDesc);
			// Render Target
			psoDesc.NumRenderTargets = rtStateDesc.rtCount;
			memcpy(&psoDesc.RTVFormats, rtStateDesc.rtFormat, rtStateDesc.rtCount * sizeof(DXGI_FORMAT));
			psoDesc.DSVFormat = rtStateDesc.depthFormat;
			// Other
			psoDesc.SampleMask = UINT_MAX;
			psoDesc.SampleDesc.Count = 1;
			psoDesc.SampleDesc.Quality = 0;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

			Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
			ThrowIfFailed(m_Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso)));
			m_PSOs.insert_or_assign(pair<RTStateDesc, RendererStateDesc>(rtStateDesc, rendererStateDesc), pso);
			return pso.Get();
		}
		return ite->second.Get();
	}

	void GraphicContext::BuildFrameResource()
	{
		m_FrameResources.clear();

		//UINT rendererCount = SceneManager::GetSingleton().GetRendererCount();
		//for (int i = 0; i < gNumFrameResources; ++i)
		//{
		//	m_FrameResources.push_back(make_unique<FrameResource>(m_Device, 1, rendererCount));
		//}
	}
