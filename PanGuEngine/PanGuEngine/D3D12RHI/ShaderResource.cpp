#include "pch.h"
#include "ShaderResource.h"
#include "Shader.h"

using namespace std;
using namespace Microsoft::WRL;

namespace RHI
{
	ShaderResource::ShaderResource(ID3DBlob* pShaderBytecode, 
								   const ShaderDesc& shaderDesc, 
								   const char* combinedSamplerSuffix) :
		m_ShaderType{shaderDesc.ShaderType}
	{

		// 使用反射来获取这个Shader需要绑定的资源
		ComPtr<ID3D12ShaderReflection> pShaderReflection;
		ThrowIfFailed(D3DReflect(pShaderBytecode->GetBufferPointer(),
							 pShaderBytecode->GetBufferSize(), 
							 __uuidof(pShaderReflection), reinterpret_cast<void**>(pShaderReflection.Get())));// 可能有问题

		D3D12_SHADER_DESC DXshaderDesc = {};
		pShaderReflection->GetDesc(&DXshaderDesc);

		m_ShaderVersion = DXshaderDesc.Version;

		// 记录Shader使用的每个资源
		UINT skipCount = 1;
		for (UINT i = 0; i < DXshaderDesc.BoundResources; i += skipCount)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindingDesc = {};
			pShaderReflection->GetResourceBindingDesc(i, &bindingDesc);

			string name = bindingDesc.Name;

			skipCount = 1;

			UINT bindCount = bindingDesc.BindCount;

			// 处理数组
			// Shader Model 5_0和之前的版本中，每个数组资源是分开列出来的。
			// 比如，Shader中定义了下面这个纹理数组：
			// 
			//		Texture2D<float3> g_tex2DDiffuse[4];
			//
			// Shader反射系统就会使用以下名字的四个资源列举出来：
			// "g_tex2DDiffuse[0]"
			// "g_tex2DDiffuse[1]"
			// "g_tex2DDiffuse[2]"
			// "g_tex2DDiffuse[3]"
			//
			// 如果数组资源的其中一些元素没有被Shader使用，就不会被列出来
			auto openBracketPos = name.find('[');
			if (-1 != openBracketPos)
			{
				assert((bindCount == 1) && "When array elements are enumerated individually, BindCount is expected to always be 1");

				// Name == "g_tex2DDiffuse[0]"
				//                        ^
				//                   OpenBracketPos
				// 名字去掉括号
				name.erase(openBracketPos, name.length() - openBracketPos);
				// Name == "g_tex2DDiffuse"

				for (UINT j = i + 1; j < DXshaderDesc.BoundResources; ++j)
				{
					D3D12_SHADER_INPUT_BIND_DESC arrayElementBindingDesc = {};
					pShaderReflection->GetResourceBindingDesc(j, &arrayElementBindingDesc);

					// strncmp：相等返回0
					if (strncmp(name.c_str(), arrayElementBindingDesc.Name, openBracketPos) == 0 && arrayElementBindingDesc.Name[openBracketPos] == '[')
					{
						// 字符串转int，字符串的数字后面可以包含其他字符，不影响结果，该函数不会抛出异常
						UINT index = atoi(arrayElementBindingDesc.Name + openBracketPos + 1);
						bindCount = std::max(bindCount, index + 1);

						++skipCount;
					}
					// 数组结束
					else
					{
						break;
					}
				}
			}

			// SIT: Shader Input Type
			switch (bindingDesc.Type)
			{
			case D3D_SIT_CBUFFER:
				m_CBs.emplace_back(name, bindingDesc.BindPoint, bindCount, bindingDesc.Type, bindingDesc.Dimension, ShaderResourceAttribs::InvalidSamplerId);
				break;
			case D3D_SIT_TEXTURE:
				if (bindingDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
				{
					m_BufferSRVs.emplace_back(name, bindingDesc.BindPoint, bindCount, bindingDesc.Type, bindingDesc.Dimension, ShaderResourceAttribs::InvalidSamplerId);
				}
				else
				{
					m_TextureSRVs.emplace_back(name, bindingDesc.BindPoint, bindCount, bindingDesc.Type, bindingDesc.Dimension, ShaderResourceAttribs::InvalidSamplerId);
				}
				break;
			case D3D_SIT_SAMPLER:
				m_Samplers.emplace_back(name, bindingDesc.BindPoint, bindCount, bindingDesc.Type, bindingDesc.Dimension, ShaderResourceAttribs::InvalidSamplerId);
				break;
			case D3D_SIT_UAV_RWTYPED:
				if (bindingDesc.Dimension == D3D_SRV_DIMENSION_BUFFER)
				{
					m_BufferUAVs.emplace_back(name, bindingDesc.BindPoint, bindCount, bindingDesc.Type, bindingDesc.Dimension, ShaderResourceAttribs::InvalidSamplerId);
				}
				else
				{
					m_TextureUAVs.emplace_back(name, bindingDesc.BindPoint, bindCount, bindingDesc.Type, bindingDesc.Dimension, ShaderResourceAttribs::InvalidSamplerId);
				}
				break;
			case D3D_SIT_STRUCTURED:
				m_BufferSRVs.emplace_back(name, bindingDesc.BindPoint, bindCount, bindingDesc.Type, bindingDesc.Dimension, ShaderResourceAttribs::InvalidSamplerId);
				break;
			case D3D_SIT_UAV_RWSTRUCTURED:
				m_BufferUAVs.emplace_back(name, bindingDesc.BindPoint, bindCount, bindingDesc.Type, bindingDesc.Dimension, ShaderResourceAttribs::InvalidSamplerId);
				break;
			case D3D_SIT_BYTEADDRESS:
				m_BufferSRVs.emplace_back(name, bindingDesc.BindPoint, bindCount, bindingDesc.Type, bindingDesc.Dimension, ShaderResourceAttribs::InvalidSamplerId);
				break;
			case D3D_SIT_UAV_RWBYTEADDRESS:
				m_BufferUAVs.emplace_back(name, bindingDesc.BindPoint, bindCount, bindingDesc.Type, bindingDesc.Dimension, ShaderResourceAttribs::InvalidSamplerId);
				break;
			default:
				LOG_ERROR("Not Supported Resource Type.");
				break;
			}
		}

		// TODO:实现跟Texture关联的Sampler
		// 给Texture SRV分配Sampler Id
		if (combinedSamplerSuffix != nullptr)
		{

		}
	}

	bool ShaderResource::IsCompatibleWith(const ShaderResource& shaderResource) const
	{
		if (GetCBNum() != shaderResource.GetCBNum() ||
			GetTexSRVNum() != shaderResource.GetTexSRVNum() ||
			GetTexUAVNum() != shaderResource.GetTexUAVNum() ||
			GetBufSRVNum() != shaderResource.GetBufSRVNum() ||
			GetBufUAVNum() != shaderResource.GetBufUAVNum() ||
			GetSamplerNum() != shaderResource.GetSamplerNum())
			return false;

		bool isCompatible = true;
		ProcessResources(
			[&](const ShaderResourceAttribs& cb, UINT32 i)
		{
			if (!cb.IsCompatibleWith(shaderResource.GetCB(i)))
				isCompatible = false;
		},
			[&](const ShaderResourceAttribs& sampler, UINT32 i)
		{
			if (!sampler.IsCompatibleWith(shaderResource.GetSampler(i)))
				isCompatible = false;
		},
			[&](const ShaderResourceAttribs& texSRV, UINT32 i)
		{
			if (!texSRV.IsCompatibleWith(shaderResource.GetTexSRV(i)))
				isCompatible = false;
		},
			[&](const ShaderResourceAttribs& texUAV, UINT32 i)
		{
			if (!texUAV.IsCompatibleWith(shaderResource.GetTexUAV(i)))
				isCompatible = false;
		},
			[&](const ShaderResourceAttribs& bufSRV, UINT32 i)
		{
			if (!bufSRV.IsCompatibleWith(shaderResource.GetBufSRV(i)))
				isCompatible = false;
		},
			[&](const ShaderResourceAttribs& bufUAV, UINT32 i)
		{
			if (!bufUAV.IsCompatibleWith(shaderResource.GetBufUAV(i)))
				isCompatible = false;
		}
		);

		return isCompatible;
	}

	size_t ShaderResource::GetHash() const
	{
		size_t hash = ComputeHash(GetCBNum(), GetTexSRVNum(), GetTexUAVNum(), GetBufSRVNum(), GetBufUAVNum(), GetSamplerNum());

		for (UINT32 i = 0; i < m_CBs.size(); ++i)
		{
			const auto& cb = GetCB(i);
			HashCombine(hash, cb);
		}

		for (UINT32 i = 0; i < m_TextureSRVs.size(); ++i)
		{
			const auto& texSRV = GetTexSRV(i);
			HashCombine(hash, texSRV);
		}

		for (UINT32 i = 0; i < m_TextureUAVs.size(); ++i)
		{
			const auto& texUAV = GetTexUAV(i);
			HashCombine(hash, texUAV);
		}

		for (UINT32 i = 0; i < m_BufferSRVs.size(); ++i)
		{
			const auto& bufSRV = GetBufSRV(i);
			HashCombine(hash, bufSRV);
		}

		for (UINT32 i = 0; i < m_BufferUAVs.size(); ++i)
		{
			const auto& bufUAV = GetBufUAV(i);
			HashCombine(hash, bufUAV);
		}

		for (UINT32 i = 0; i < m_Samplers.size(); ++i)
		{
			const auto& sampler = GetSampler(i);
			HashCombine(hash, sampler);
		}

		return hash;
	}
}