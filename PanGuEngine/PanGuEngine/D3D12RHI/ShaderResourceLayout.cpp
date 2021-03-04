#include "pch.h"
#include "ShaderResourceLayout.h"
#include "TextureView.h"
#include "BufferView.h"
#include "Buffer.h"
#include "ShaderResourceBindingUtility.h"
#include "RootSignature.h"

namespace RHI
{
	

	ShaderResourceLayout::ShaderResourceLayout(ID3D12Device* pd3d12Device,
											   PIPELINE_TYPE pipelineType,
											   const ShaderVariableConfig& shaderVariableConfig, 
											   const ShaderResource* shaderResource,
											   RootSignature* rootSignature) :
		m_D3D12Device(pd3d12Device)
	{
		// 把ShaderResource中的每个资源都通过RootSignature确定RootIndex和OffsetFromTableStart，然后存储下来
		auto AddResource = [&](const ShaderResourceAttribs& Attribs,
							   BindingResourceType              ResType,
							   SHADER_RESOURCE_VARIABLE_TYPE   VarType) //
		{
			assert(rootSignature != nullptr);
			
			UINT32 RootIndex = Resource::InvalidRootIndex;
			UINT32 Offset = Resource::InvalidOffset;

			D3D12_DESCRIPTOR_RANGE_TYPE DescriptorRangeType = GetDescriptorRangeType(ResType);
			SHADER_TYPE shaderType = shaderResource->GetShaderType();
			
			// 按照ShaderResource中的顺序添加到RootSignature中，并分配RootIndex和Offset
			rootSignature->AllocateResourceSlot(shaderType, pipelineType, Attribs, VarType, DescriptorRangeType, RootIndex, Offset);

			m_SrvCbvUavs[VarType].emplace_back(*this, Attribs, VarType, ResType, RootIndex, Offset);
		};

		shaderResource->ProcessResources(
			[&](const ShaderResourceAttribs& CB, UINT32)
		{
			auto VarType = shaderResource->FindVariableType(CB, shaderVariableConfig);
			AddResource(CB, BindingResourceType::CBV, VarType);
		},
			[&](const ShaderResourceAttribs& TexSRV, UINT32)
		{
			auto VarType = shaderResource->FindVariableType(TexSRV, shaderVariableConfig);
			AddResource(TexSRV, BindingResourceType::TexSRV, VarType);
		},
			[&](const ShaderResourceAttribs& TexUAV, UINT32)
		{
			auto VarType = shaderResource->FindVariableType(TexUAV, shaderVariableConfig);
			AddResource(TexUAV, BindingResourceType::TexUAV, VarType);
		},
			[&](const ShaderResourceAttribs& BufSRV, UINT32)
		{
			auto VarType = shaderResource->FindVariableType(BufSRV, shaderVariableConfig);
			AddResource(BufSRV, BindingResourceType::BufSRV, VarType);
		},
			[&](const ShaderResourceAttribs& BufUAV, UINT32)
		{
			auto VarType = shaderResource->FindVariableType(BufUAV, shaderVariableConfig);
			AddResource(BufUAV, BindingResourceType::BufUAV, VarType);
		}
		);
	}

	bool ShaderResourceLayout::Resource::IsBound(UINT32 arrayIndex, const ShaderResourceCache& resourceCache) const
	{
		if (ResourceType == BindingResourceType::CBV)
		{
			const ShaderResourceCache::RootDescriptor& rootDescriptor = resourceCache.GetRootDescriptor(RootIndex);
			if (rootDescriptor.ConstantBuffer != nullptr)
				return true;
		}
		else
		{
			const ShaderResourceCache::RootTable& rootTable = resourceCache.GetRootTable(RootIndex);
			if (OffsetFromTableStart + arrayIndex < rootTable.Descriptors.size())
			{
				if (rootTable.Descriptors[OffsetFromTableStart + arrayIndex] != nullptr)
					return true;
			}
		}

		return false;
	}

	// 绑定Constant Buffer时只需要持有GpuBuffer对象，提交资源时只需要Buffer的GPU地址
	void ShaderResourceLayout::Resource::BindResource(std::shared_ptr<GpuBuffer> buffer, UINT32 arrayIndex, ShaderResourceCache& resourceCache) const
	{
		// 只有Constant Buffer作为Root Descriptor绑定！！！
		assert(ResourceType == BindingResourceType::CBV);

		ShaderResourceCache::RootDescriptor& rootDescriptor = resourceCache.GetRootDescriptor(RootIndex);

		if (VariableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC && rootDescriptor.ConstantBuffer != nullptr)
		{
			// 如果已经绑定了资源就不更新，除非是Dynamic资源
			return;
		}

		rootDescriptor.ConstantBuffer = buffer;
	}

	// 绑定Root Table中的Descriptor时，需要把资源在CPUDescriptorHeap中的Descriptor拷贝到ShaderResourceCache中的GPUDescriptorHeap中
	void ShaderResourceLayout::Resource::BindResource(std::shared_ptr<GpuResourceDescriptor> descriptor, UINT32 arrayIndex, ShaderResourceCache& resourceCache) const
	{
		ShaderResourceCache::RootTable& rootTable = resourceCache.GetRootTable(RootIndex);

		// 如果已经绑定了资源就不更新，除非是Dynamic资源
		if (VariableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC && rootTable.Descriptors[OffsetFromTableStart + arrayIndex] != nullptr)
		{
			return;
		}
		// 保存descriptor对象的引用
		rootTable.Descriptors[OffsetFromTableStart + arrayIndex] = descriptor;

		// Static、Mutable的资源会Copy到ShaderResourceCache的GPUDescriptorHeap中，Dynamic资源每帧动态分配，只需要记录资源在CPUDescriptorHeap中的Descriptor
		D3D12_CPU_DESCRIPTOR_HANDLE shaderVisibleHeapCPUDescriptorHandle = resourceCache.
			GetShaderVisibleTableCPUDescriptorHandle<D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV>(RootIndex, OffsetFromTableStart + arrayIndex);

		if (shaderVisibleHeapCPUDescriptorHandle.ptr != 0)
		{
			ID3D12Device* d3d12Device = ParentResLayout.m_D3D12Device;
			d3d12Device->CopyDescriptorsSimple(1,
				shaderVisibleHeapCPUDescriptorHandle,
				descriptor->CPUDescriptorHandle,
				D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
	}
}