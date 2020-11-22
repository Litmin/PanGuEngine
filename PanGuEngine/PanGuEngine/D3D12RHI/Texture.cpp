#include "pch.h"
#include "Texture.h"
#include "RenderDevice.h"

namespace RHI
{
	Texture::Texture(RenderDevice* renderDevice, 
					 const TextureDesc& texDesc, 
					 const TextureData* initData) :
		m_RenderDevice{renderDevice},
		m_Desc{texDesc}
	{
		D3D12_RESOURCE_DESC Desc = GetD3D12TextureDesc();

		if (m_Desc.Usage == USAGE_IMMUTABLE || m_Desc.Usage == USAGE_DEFAULT || m_Desc.Usage == USAGE_DYNAMIC)
		{
			D3D12_HEAP_PROPERTIES HeapProps = {};

			HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
			HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			HeapProps.CreationNodeMask = 1;
			HeapProps.VisibleNodeMask = 1;

			// 初始状态以及是否要初始化,静态Texture需要在创建时使用UploadBuffer初始化数据，因为CPU不能写入数据
			bool bInitializeTexture = (initData != nullptr && initData->SubResources != nullptr && initData->NumSubresources > 0);
			auto InitialState = bInitializeTexture ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON;
			SetState(InitialState);

			ThrowIfFailed(m_RenderDevice->GetD3D12Device()->CreateCommittedResource(
				&HeapProps, D3D12_HEAP_FLAG_NONE, &Desc, InitialState, &m_Desc.ClearValue, IID_PPV_ARGS(&m_D3d12Resource)));

			// 设置Buffer名字，方便调试
			if (!m_Desc.Name.empty())
				m_D3d12Resource->SetName(m_Desc.Name.c_str());

			// TODO: Initial Texture
			if (bInitializeTexture)
			{

			}

		}
		// TODO: Staging Texture
	}

	Texture::Texture(RenderDevice* renderDevice, 
					 const TextureDesc& texDesc, 
					 D3D12_RESOURCE_STATES initialState, 
					 ID3D12Resource* pTexture) :
		m_RenderDevice{renderDevice},
		m_Desc{texDesc}
	{
		m_D3d12Resource = pTexture;
		SetState(initialState);
	}

	Texture::~Texture()
	{
		m_RenderDevice->SafeReleaseDeviceObject(std::move(m_D3d12Resource));
	}

	std::unique_ptr<TextureView> Texture::CreateView(const TextureViewDesc& viewDesc)
	{
		return std::unique_ptr<TextureView>();
	}

	TextureView* Texture::GetDefaultView(TEXTURE_VIEW_TYPE viewType)
	{
		return nullptr;
	}

	void Texture::CreateDefaultViews()
	{

	}

	D3D12_RESOURCE_DESC Texture::GetD3D12TextureDesc() const
	{
		D3D12_RESOURCE_DESC Desc = {};

		Desc.Alignment = 0;
		if (m_Desc.Type == RESOURCE_DIM_TEX_1D_ARRAY || m_Desc.Type == RESOURCE_DIM_TEX_2D_ARRAY || m_Desc.Type == RESOURCE_DIM_TEX_CUBE || m_Desc.Type == RESOURCE_DIM_TEX_CUBE_ARRAY)
			Desc.DepthOrArraySize = (UINT16)m_Desc.ArraySize;
		else if (m_Desc.Type == RESOURCE_DIM_TEX_3D)
			Desc.DepthOrArraySize = (UINT16)m_Desc.Depth;
		else
			Desc.DepthOrArraySize = 1;

		if (m_Desc.Type == RESOURCE_DIM_TEX_1D || m_Desc.Type == RESOURCE_DIM_TEX_1D_ARRAY)
			Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		else if (m_Desc.Type == RESOURCE_DIM_TEX_2D || m_Desc.Type == RESOURCE_DIM_TEX_2D_ARRAY || m_Desc.Type == RESOURCE_DIM_TEX_CUBE || m_Desc.Type == RESOURCE_DIM_TEX_CUBE_ARRAY)
			Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		else if (m_Desc.Type == RESOURCE_DIM_TEX_3D)
			Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		else
		{
			LOG_ERROR("Unknown texture type");
		}

		Desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		if (m_Desc.BindFlag & BIND_RENDER_TARGET)
			Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		if (m_Desc.BindFlag & BIND_DEPTH_STENCIL)
			Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		if ((m_Desc.BindFlag & BIND_UNORDERED_ACCESS) || (m_Desc.MiscFlags & MISC_TEXTURE_FLAG_GENERATE_MIPS))
			Desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		if ((m_Desc.BindFlag & BIND_SHADER_RESOURCE) == 0 && (m_Desc.BindFlag & BIND_DEPTH_STENCIL) != 0)
			Desc.Flags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

		auto Format = m_Desc.Format;
		if (Format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB && (Desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS))
			Desc.Format = DXGI_FORMAT_R8G8B8A8_TYPELESS;
		else
			Desc.Format = Format;

		Desc.Height = UINT{ m_Desc.Height };
		Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		Desc.MipLevels = static_cast<UINT16>(m_Desc.MipLevels);
		Desc.SampleDesc.Count = m_Desc.SampleCount;
		Desc.SampleDesc.Quality = 0;
		Desc.Width = UINT64{ m_Desc.Width };

		return Desc;
	}
}
