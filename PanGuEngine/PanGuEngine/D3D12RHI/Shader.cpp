#include "pch.h"
#include "Shader.h"

using namespace std;
using namespace Microsoft::WRL;

namespace RHI
{
    // ����Shader
    // Shader Profile
    string GetHLSLProfileString(SHADER_TYPE shaderType, ShaderModel sm)
    {
        string strShaderProfile;
        switch (shaderType)
        {
        case SHADER_TYPE_VERTEX:        strShaderProfile = "vs"; break;
        case SHADER_TYPE_PIXEL:         strShaderProfile = "ps"; break;
        case SHADER_TYPE_GEOMETRY:      strShaderProfile = "gs"; break;
        case SHADER_TYPE_HULL:          strShaderProfile = "hs"; break;
        case SHADER_TYPE_DOMAIN:        strShaderProfile = "ds"; break;
        case SHADER_TYPE_COMPUTE:       strShaderProfile = "cs"; break;
        case SHADER_TYPE_AMPLIFICATION: strShaderProfile = "as"; break;
        case SHADER_TYPE_MESH:          strShaderProfile = "ms"; break;
        default: LOG_ERROR("Unknown shader type");
        }

        strShaderProfile += "_";
        strShaderProfile += std::to_string(sm.Major);
        strShaderProfile += "_";
        strShaderProfile += std::to_string(sm.Minor);

        return strShaderProfile;
    }

    // ����Shader�ĺ�
    const ShaderMacro VSMacros[] = { {"VERTEX_SHADER", "1"}, {} };
    const ShaderMacro PSMacros[] = { {"FRAGMENT_SHADER", "1"}, {"PIXEL_SHADER", "1"}, {} };
    const ShaderMacro GSMacros[] = { {"GEOMETRY_SHADER", "1"}, {} };
    const ShaderMacro HSMacros[] = { {"TESS_CONTROL_SHADER", "1"}, {"HULL_SHADER", "1"}, {} };
    const ShaderMacro DSMacros[] = { {"TESS_EVALUATION_SHADER", "1"}, {"DOMAIN_SHADER", "1"}, {} };
    const ShaderMacro CSMacros[] = { {"COMPUTE_SHADER", "1"}, {} };
    const ShaderMacro ASMacros[] = { {"TASK_SHADER", "1"}, {"AMPLIFICATION_SHADER", "1"}, {} };
    const ShaderMacro MSMacros[] = { {"MESH_SHADER", "1"}, {} };

    const ShaderMacro* GetShaderTypeMacros(SHADER_TYPE Type)
    {
        switch (Type)
        {
        case SHADER_TYPE_VERTEX:        return VSMacros;
        case SHADER_TYPE_PIXEL:         return PSMacros;
        case SHADER_TYPE_GEOMETRY:      return GSMacros;
        case SHADER_TYPE_HULL:          return HSMacros;
        case SHADER_TYPE_DOMAIN:        return DSMacros;
        case SHADER_TYPE_COMPUTE:       return CSMacros;
        case SHADER_TYPE_AMPLIFICATION: return ASMacros;
        case SHADER_TYPE_MESH:          return MSMacros;
        default:
            return nullptr;
        }
    }

    void AppendShaderMacros(string& source, const ShaderMacro* macros)
    {
        if (macros == nullptr)
            return;

        for (auto* pMacro = macros; !pMacro->Name.empty() && !pMacro->Definition.empty(); ++pMacro)
        {
            source += "#define ";
            source += pMacro->Name;
            source += ' ';
            source += pMacro->Definition;
            source += "\n";
        }
    }

    // ����Shader��Դ��
    string BuildHLSLSourceString(const ShaderCreateInfo& shaderCI)
    {
        string HLSLSource;

        // ���Shader Type��
        auto shaderTypeMacros = GetShaderTypeMacros(shaderCI.Desc.ShaderType);
        AppendShaderMacros(HLSLSource, shaderTypeMacros);

        // ����Զ���ĺ�
        if (shaderCI.Macros != nullptr)
        {
            HLSLSource += '\n';
            AppendShaderMacros(HLSLSource, shaderCI.Macros);
        }

        // ���Shader����
        // TODO:��ȡShader�ļ�


        return HLSLSource;
    }


    Shader::Shader(const ShaderCreateInfo& shaderCI) :
        m_Desc{shaderCI.Desc}
    {
        // ����Shader
        string strShaderProfile = GetHLSLProfileString(shaderCI.Desc.ShaderType, shaderCI.SM);

        UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
        compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        // ֱ�Ӵ�Shader�ļ�����
        ComPtr<ID3DBlob> errors;
        HRESULT hr = S_OK;

        hr = D3DCompileFromFile(shaderCI.FilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, 
            shaderCI.entryPoint.c_str(), strShaderProfile.c_str(), compileFlags, 0, &m_ShaderByteCode, &errors);

        if (errors != nullptr)
            LOG_ERROR((char*)errors->GetBufferPointer());

        ThrowIfFailed(hr);

        // ʹ��Shader����ϵͳ�ռ���Shader�õ�����Դ
        m_ShaderResource = make_unique<const ShaderResource>(m_ShaderByteCode.Get(), shaderCI.Desc);
    }
}