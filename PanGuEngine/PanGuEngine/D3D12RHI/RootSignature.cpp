#include "pch.h"
#include "RootSignature.h"
#include "RenderDevice.h"

using namespace Microsoft::WRL;

namespace RHI
{
	D3D12_SHADER_VISIBILITY GetShaderVisibility(SHADER_TYPE ShaderType)
	{
		switch (ShaderType)
		{
		case SHADER_TYPE_VERTEX:        
			return D3D12_SHADER_VISIBILITY_VERTEX;
		case SHADER_TYPE_PIXEL:     
			return D3D12_SHADER_VISIBILITY_PIXEL;
		case SHADER_TYPE_GEOMETRY:      
			return D3D12_SHADER_VISIBILITY_GEOMETRY;
		case SHADER_TYPE_HULL:          
			return D3D12_SHADER_VISIBILITY_HULL;
		case SHADER_TYPE_DOMAIN:        
			return D3D12_SHADER_VISIBILITY_DOMAIN;
		case SHADER_TYPE_COMPUTE:       
			return D3D12_SHADER_VISIBILITY_ALL;
#   ifdef D3D12_H_HAS_MESH_SHADER
		case SHADER_TYPE_AMPLIFICATION: 
			return D3D12_SHADER_VISIBILITY_AMPLIFICATION;
		case SHADER_TYPE_MESH:   
			return D3D12_SHADER_VISIBILITY_MESH;
#   endif
		default: 
			LOG_ERROR("Unknown shader type"); 
			return D3D12_SHADER_VISIBILITY_ALL;
		}
	}


	// <------------------RootParamsManager------------------------------>
	void RootSignature::RootParamsManager::AddRootView(D3D12_ROOT_PARAMETER_TYPE ParameterType, 
													   UINT32 RootIndex, 
													   UINT Register, 
													   D3D12_SHADER_VISIBILITY Visibility, 
													   SHADER_RESOURCE_VARIABLE_TYPE VarType)
	{
		m_RootViews.emplace_back(ParameterType, RootIndex, Register, 0u/*Register Space*/, Visibility, VarType);
	}

	void RootSignature::RootParamsManager::AddRootTable(UINT32 RootIndex, 
														D3D12_SHADER_VISIBILITY Visibility, 
														SHADER_RESOURCE_VARIABLE_TYPE VarType, 
														UINT32 NumRangesInNewTable)
	{
		m_RootTables.emplace_back(D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, RootIndex, NumRangesInNewTable, Visibility, VarType);
	}

	void RootSignature::RootParamsManager::AddDescriptorRanges(UINT32 RootTableInd, UINT32 NumExtraRanges)
	{
		assert(RootTableInd < m_RootTables.size());
		m_RootTables[RootTableInd].AddDescriptorRanges(NumExtraRanges);
	}

	bool RootSignature::RootParamsManager::operator==(const RootParamsManager& RootParams) const
	{
		// 比较Root Table和Root View的数量
		if (m_RootTables.size() != RootParams.m_RootTables.size() ||
			m_RootViews.size() != RootParams.m_RootViews.size())
			return false;

		// 比较Root View
		for (UINT32 i = 0; i < m_RootViews.size(); ++i)
		{
			const auto& rootView0 = GetRootView(i);
			const auto& rootView1 = RootParams.GetRootView(i);
			if (rootView0 != rootView1)
				return false;
		}

		// 比较Root Table
		for (UINT32 i = 0; i < m_RootTables.size(); ++i)
		{
			const auto& rootTable0 = GetRootTable(i);
			const auto& rootTable1 = RootParams.GetRootTable(i);
			if (rootTable0 != rootTable1)
				return false;
		}

		return true;
	}

	size_t RootSignature::RootParamsManager::GetHash() const
	{
		size_t hash = ComputeHash(m_RootTables.size(), m_RootViews.size());
		for (UINT32 i = 0; i < m_RootViews.size(); ++i)
			HashCombine(hash, GetRootView(i).GetHash());

		for (UINT32 i = 0; i < m_RootTables.size(); ++i)
			HashCombine(hash, GetRootTable(i).GetHash());

		return hash;
	}
	// <------------------RootParamsManager------------------------------>


	// <--------------------RootSignature-------------------------------->
	RootSignature::RootSignature()
	{
		m_SrvCbvUavRootTablesMap.fill(InvalidRootTableIndex);
	}

	// TODO:RootSignature是否要SafeRelease
	RootSignature::~RootSignature()
	{
	}

	// 完成Root Signature的构造，创建Direct3D 12的Root Signature
	void RootSignature::Finalize(ID3D12Device* pd3d12Device)
	{
		// 统计Root Table中Descriptor的数量
		for (UINT32 i = 0; i < m_RootParams.GetRootTableNum(); ++i)
		{
			const auto& RootTbl = m_RootParams.GetRootTable(i);
			const auto& d3d12RootParam = static_cast<const D3D12_ROOT_PARAMETER&>(RootTbl);
			assert(d3d12RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);

			auto TableSize = RootTbl.GetDescriptorTableSize();
			assert(d3d12RootParam.DescriptorTable.NumDescriptorRanges > 0 && TableSize > 0 && "Unexpected empty descriptor table");
			auto VarType = RootTbl.GetShaderVariableType();
			m_TotalSrvCbvUavSlots[VarType] += TableSize;
		}

		// 统计Root View的数量
		for (UINT32 i = 0; i < m_RootParams.GetRootViewNum(); ++i)
		{
			const auto& RootView = m_RootParams.GetRootView(i);
			++m_TotalRootViews[RootView.GetShaderVariableType()];
		}

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		// RootParameter的总数
		auto TotalParams = m_RootParams.GetRootTableNum() + m_RootParams.GetRootViewNum();

		std::vector<D3D12_ROOT_PARAMETER> D3D12Parameters(TotalParams, D3D12_ROOT_PARAMETER{});
		// 设置每个Root Paramter
		for (UINT32 i = 0; i < m_RootParams.GetRootTableNum(); ++i)
		{
			const auto& RootTable = m_RootParams.GetRootTable(i);
			const D3D12_ROOT_PARAMETER& SrcParam = RootTable;	// 隐式转换
			assert(SrcParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && SrcParam.DescriptorTable.NumDescriptorRanges > 0 && "Non-empty descriptor table is expected");
			D3D12Parameters[RootTable.GetRootIndex()] = SrcParam;
		}
		for (UINT32 i = 0; i < m_RootParams.GetRootViewNum(); ++i)
		{
			const auto& RootView = m_RootParams.GetRootView(i);
			const D3D12_ROOT_PARAMETER& SrcParam = RootView;
			assert(SrcParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV && "Root CBV is expected");
			D3D12Parameters[RootView.GetRootIndex()] = SrcParam;
		}

		rootSignatureDesc.NumParameters = static_cast<UINT>(D3D12Parameters.size());
		rootSignatureDesc.pParameters = D3D12Parameters.size() ? D3D12Parameters.data() : nullptr;

		// TODO: Static Sampler!!!!!!

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		HRESULT           hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);

		if (error != nullptr)
		{
			LOG_ERROR((char*)error->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(pd3d12Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_pd3d12RootSignature.GetAddressOf()) ));
	}

	// 初始化SRB的Cache，因为只有RootSignature知道足够的信息，初始化每个Shader
	void RootSignature::InitResourceCacheForSRB(RenderDevice* RenderDevice, ShaderResourceCache& ResourceCache) const
	{
		auto CacheTableSizes = GetCacheTableSizes();

		ResourceCache.Initialize(static_cast<UINT32>(CacheTableSizes.size()), CacheTableSizes.data());

		// 在GPUDescriptorHeap中为Static和Mutable资源分配空间
		UINT32 TotalSrvCbvUavDescriptors =
			m_TotalSrvCbvUavSlots[SHADER_RESOURCE_VARIABLE_TYPE_STATIC] +
			m_TotalSrvCbvUavSlots[SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE];

		DescriptorHeapAllocation CbcSrvUavHeapSpace;
		if (TotalSrvCbvUavDescriptors)
		{
			CbcSrvUavHeapSpace = RenderDevice->AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, TotalSrvCbvUavDescriptors);
			assert(!CbcSrvUavHeapSpace.IsNull() && "Failed to allocate  GPU-visible CBV/SRV/UAV descriptor");
		}

		// 设置每个Root Table在GPUDescriptorHeap中的Offset，Dynamic资源不管，它不在GPUDescriptorHeap中分配空间，
		// RootView也不管，它不需要分配空间!!!!!!
		UINT32 SrvCbvUavTblStartOffset = 0;
		for (UINT32 i = 0; i < m_RootParams.GetRootTableNum(); ++i)
		{
			auto& RootTable = m_RootParams.GetRootTable(i);
			const auto& D3D12RootParam = static_cast<const D3D12_ROOT_PARAMETER&>(RootTable); // 隐式转换
			auto& RootTableCache = ResourceCache.GetRootTable(RootTable.GetRootIndex());

			assert(D3D12RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);

			auto TableSize = RootTable.GetDescriptorTableSize();
			assert(TableSize > 0 && "Unexpected empty descriptor table");

			if (RootTable.GetShaderVariableType() != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
			{
				RootTableCache.m_TableStartOffset = SrvCbvUavTblStartOffset;
				SrvCbvUavTblStartOffset += TableSize;
			}
			else
			{
				assert(RootTableCache.m_TableStartOffset == ShaderResourceCache::InvalidDescriptorOffset);
			}
		}

		ResourceCache.SetDescriptorHeapSpace(std::move(CbcSrvUavHeapSpace));
	}

	// 为Shader中的每个ShaderResource分配一个容身之地
	void RootSignature::AllocateResourceSlot(SHADER_TYPE ShaderType,
											 PIPELINE_TYPE PipelineType, 
											 const ShaderResourceAttribs& ShaderResAttribs, 
											 SHADER_RESOURCE_VARIABLE_TYPE VariableType, 
											 D3D12_DESCRIPTOR_RANGE_TYPE RangeType, 
											 UINT32& RootIndex,				// 输出参数
											 UINT32& OffsetFromTableStart)	// 输出参数
	{
		const auto shaderVisibility = GetShaderVisibility(ShaderType);

		// 分配一个CBV，当作一个Root View
		if (RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_CBV && ShaderResAttribs.BindCount == 1)
		{
			RootIndex = m_RootParams.GetRootTableNum() + m_RootParams.GetRootViewNum();
			OffsetFromTableStart = 0;

			m_RootParams.AddRootView(D3D12_ROOT_PARAMETER_TYPE_CBV, RootIndex, ShaderResAttribs.BindPoint, shaderVisibility, VariableType);
		}
		// 分配一个新的Root Table，或者在现有的Root Table中添加Descriptor
		else
		{
			// 见头文件注释
			const auto ShaderInd = GetShaderTypePipelineIndex(ShaderType, PipelineType);
			// Static、Mutable资源放在同一个Root Table中
			const auto RootTableType = (VariableType == SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC) ? SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC : SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
			// Shader Type和Variable Type共同组成该Table的Index
			const auto TableIndKey = ShaderInd * SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES + RootTableType;
			// m_SrvCbvUavRootTablesMap存储了指定类型的Root Table在m_RootParams.m_RootTables中的索引
			auto& RootTableArrayInd = m_SrvCbvUavRootTablesMap[TableIndKey];
			
			if (RootTableArrayInd == InvalidRootTableIndex)
			{
				// 该Table还没创建，创建一个新的Root Table
				RootIndex = m_RootParams.GetRootTableNum() + m_RootParams.GetRootViewNum();
				RootTableArrayInd = static_cast<UINT8>(m_RootParams.GetRootTableNum());
				// 添加有一个Descriptor Range的Root Table
				m_RootParams.AddRootTable(RootIndex, shaderVisibility, RootTableType, 1);
			}
			else
			{
				// 往现有的Root Table中添加一个Descriptor Range
				m_RootParams.AddDescriptorRanges(RootTableArrayInd, 1);
			}

			// 刚刚创建的或者刚刚使用的Root Table
			auto& RootTable = m_RootParams.GetRootTable(RootTableArrayInd);
			RootIndex = RootTable.GetRootIndex();

			const auto& d3d12RootParam = static_cast<const D3D12_ROOT_PARAMETER&>(RootTable);

			assert(d3d12RootParam.ShaderVisibility == shaderVisibility && "Shader visibility is not correct");

			// Descriptor是紧密排列的，所以要添加的这个Descriptor的Offset就是添加前Descriptor的数量
			// 调用了AddDescriptorRanges时，GetDescriptorTableSize还没有增加！！！！！！
			OffsetFromTableStart = RootTable.GetDescriptorTableSize();


			// 刚刚添加的Descriptor Range的索引
			UINT32 NewDescriptorRangeIndex = d3d12RootParam.DescriptorTable.NumDescriptorRanges - 1;
			RootTable.SetDescriptorRange(NewDescriptorRangeIndex, RangeType, ShaderResAttribs.BindPoint, ShaderResAttribs.BindCount, 0, OffsetFromTableStart);
		}
	}

	// 计算Resource Cache中需要的Table数量，以及每个Table中Descriptor的数量，Root View当成只有一个Descriptor的Table
	std::vector<UINT32> RootSignature::GetCacheTableSizes() const
	{
		std::vector<UINT32> CacheTableSizes(m_RootParams.GetRootTableNum() + m_RootParams.GetRootViewNum(), 0);

		for (UINT32 i = 0; i < m_RootParams.GetRootTableNum(); ++i)
		{
			auto& RootParam = m_RootParams.GetRootTable(i);
			CacheTableSizes[RootParam.GetRootIndex()] = RootParam.GetDescriptorTableSize();
		}

		for (UINT32 i = 0; i < m_RootParams.GetRootViewNum(); ++i)
		{
			auto& RootParam = m_RootParams.GetRootView(i);
			// ShaderResourceCache中把RootView当成只有一个Descriptor的Root Table
			CacheTableSizes[RootParam.GetRootIndex()] = 1;
		}

		return CacheTableSizes;
	}

	// <--------------------RootSignature-------------------------------->

}