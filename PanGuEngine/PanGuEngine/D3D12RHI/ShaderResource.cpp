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

		// ʹ�÷�������ȡ���Shader��Ҫ�󶨵���Դ
		ComPtr<ID3D12ShaderReflection> pShaderReflection;
		ThrowIfFailed(D3DReflect(pShaderBytecode->GetBufferPointer(),
							 pShaderBytecode->GetBufferSize(), 
							 __uuidof(pShaderReflection), reinterpret_cast<void**>(pShaderReflection.Get())));// ����������

		D3D12_SHADER_DESC DXshaderDesc = {};
		pShaderReflection->GetDesc(&DXshaderDesc);

		m_ShaderVersion = DXshaderDesc.Version;

		// ��¼Shaderʹ�õ�ÿ����Դ
		UINT skipCount = 1;
		for (UINT i = 0; i < DXshaderDesc.BoundResources; i += skipCount)
		{
			D3D12_SHADER_INPUT_BIND_DESC bindingDesc = {};
			pShaderReflection->GetResourceBindingDesc(i, &bindingDesc);

			string name = bindingDesc.Name;

			skipCount = 1;

			UINT bindCount = bindingDesc.BindCount;

			// ��������
			// Shader Model 5_0��֮ǰ�İ汾�У�ÿ��������Դ�Ƿֿ��г����ġ�
			// ���磬Shader�ж�������������������飺
			// 
			//		Texture2D<float3> g_tex2DDiffuse[4];
			//
			// Shader����ϵͳ�ͻ�ʹ���������ֵ��ĸ���Դ�оٳ�����
			// "g_tex2DDiffuse[0]"
			// "g_tex2DDiffuse[1]"
			// "g_tex2DDiffuse[2]"
			// "g_tex2DDiffuse[3]"
			//
			// ���������Դ������һЩԪ��û�б�Shaderʹ�ã��Ͳ��ᱻ�г���
			auto openBracketPos = name.find('[');
			if (-1 != openBracketPos)
			{
				assert((bindCount == 1) && "When array elements are enumerated individually, BindCount is expected to always be 1");

				// Name == "g_tex2DDiffuse[0]"
				//                        ^
				//                   OpenBracketPos
				// ����ȥ������
				name.erase(openBracketPos, name.length() - openBracketPos);
				// Name == "g_tex2DDiffuse"

				for (UINT j = i + 1; j < DXshaderDesc.BoundResources; ++j)
				{
					D3D12_SHADER_INPUT_BIND_DESC arrayElementBindingDesc = {};
					pShaderReflection->GetResourceBindingDesc(j, &arrayElementBindingDesc);

					// strncmp����ȷ���0
					if (strncmp(name.c_str(), arrayElementBindingDesc.Name, openBracketPos) == 0 && arrayElementBindingDesc.Name[openBracketPos] == '[')
					{
						// �ַ���תint���ַ��������ֺ�����԰��������ַ�����Ӱ�������ú��������׳��쳣
						UINT index = atoi(arrayElementBindingDesc.Name + openBracketPos + 1);
						bindCount = std::max(bindCount, index + 1);

						++skipCount;
					}
					// �������
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

		// TODO:ʵ�ָ�Texture������Sampler
		// ��Texture SRV����Sampler Id
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