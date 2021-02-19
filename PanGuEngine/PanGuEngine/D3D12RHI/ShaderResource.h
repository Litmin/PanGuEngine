#pragma once
#include "ShaderResourceBindingUtility.h"


namespace RHI 
{
    struct ShaderDesc;
    struct ShaderVariableConfig;

    // ��ʾShader��ʹ�õ�һ����Դ
    struct ShaderResourceAttribs
    {
        // ����������
        const std::string Name; // TODO:����ʹ��char*��StringPool���Ż�
        // �����ļĴ������,����cbuffer cbBuff0 : register(b5);��BindPoint����5
        const UINT16 BindPoint;
        // ���������BindCount��������Ĵ�С�������������BindCount��1
        const UINT16 BindCount;
        
    private:
        //            4               4                 24           
        // bit | 0  1  2  3   |  4  5  6  7  |  8   9  10   ...   31  |   
        //     |              |              |                        |
        //     |  InputType   |   SRV Dim    | SamplerOrTexSRVIdBits  |
        // ��constexpr���ε�const�����ǳ������ʽ����һ������ϵͳ�У����ѷֱ�һ����ʼֵ�Ƿ��ǳ������ʽ��
        // C++11�涨��������������Ϊconstexpr���ͣ������������ͻ���֤������ֵ�Ƿ���һ���������ʽ��
        // ����Ϊconstexpr�ı��������ó������ʽ��ʼ����
        static constexpr const UINT32 ShaderInputTypeBits = 4;
        static constexpr const UINT32 SRVDimBits = 4;
        static constexpr const UINT32 SamplerOrTexSRVIdBits = 24;

        // ��֤ÿ������ռ�õ�Bits�Ƿ��㹻
        static_assert(ShaderInputTypeBits + SRVDimBits + SamplerOrTexSRVIdBits == 32, "�������PackΪ32λ��");

        static_assert(D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER < (1 << ShaderInputTypeBits), "Not enough bits to represent D3D_SHADER_INPUT_TYPE");;
        static_assert(D3D_SRV_DIMENSION_BUFFEREX < (1 << SRVDimBits), "Not enough bits to represent D3D_SRV_DIMENSION");

        // bitfileds,λ��
        // D3D_SHADER_INPUT_TYPE:https://docs.microsoft.com/zh-cn/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_shader_input_type?redirectedfrom=MSDN
        const UINT32 InputType : ShaderInputTypeBits;
        // D3D_SRV_DIMENSION:https://docs.microsoft.com/zh-cn/windows/win32/api/d3dcommon/ne-d3dcommon-d3d_srv_dimension?redirectedfrom=MSDN
        const UINT32 SRVDimension : SRVDimBits;

    public:
        static constexpr const UINT32 InvalidTexSRVId = (1 << SamplerOrTexSRVIdBits) - 1;
        static constexpr const UINT16 InvalidBindPoint = std::numeric_limits<UINT16>::max();
        static constexpr const UINT16 MaxBindPoint = InvalidBindPoint - 1;
        static constexpr const UINT16 MaxBindCount = std::numeric_limits<UINT16>::max();

        ShaderResourceAttribs(const std::string& name, 
                              UINT bindPoint,
                              UINT bindCount,
                              D3D_SHADER_INPUT_TYPE inputType,
                              D3D_SRV_DIMENSION srvDimension) noexcept :
            Name{name},
            BindPoint{static_cast<decltype(BindPoint)>(bindPoint)},
            BindCount{static_cast<decltype(BindCount)>(bindCount)},
            InputType{static_cast<decltype(InputType)>(inputType)},
            SRVDimension{static_cast<decltype(SRVDimension)>(srvDimension)}
        {

        }

        // ɾ����������
        ShaderResourceAttribs(const ShaderResourceAttribs& rhs) = delete;
        ShaderResourceAttribs& operator = (const ShaderResourceAttribs& rhs) = delete;


        D3D_SHADER_INPUT_TYPE GetInputType() const
        {
            return static_cast<D3D_SHADER_INPUT_TYPE>(InputType);
        }

        D3D_SRV_DIMENSION GetSRVDimension() const
        {
            return static_cast<D3D_SRV_DIMENSION>(SRVDimension);
        }

        bool IsValidBindPoint() const
        {
            return BindPoint != InvalidBindPoint;
        }

        bool IsCompatibleWith(const ShaderResourceAttribs& Attribs) const
        {
            return BindPoint == Attribs.BindPoint &&
                BindCount == Attribs.BindCount &&
                InputType == Attribs.InputType &&
                SRVDimension == Attribs.SRVDimension;
        }

        size_t GetHash() const
        {
            return ComputeHash(BindPoint, BindCount, InputType, SRVDimension);
        }

    private:
        friend class ShaderResources;
    };
    // static_assert(sizeof(ShaderResourceAttribs) == sizeof(void*) + sizeof(UINT32) * 2, "Unexpected sizeof(ShaderResourceAttribs)");


    // ����һ��Shaderʹ�õ�������Դ,ʹ��DX��Shader����ϵͳʵ��
    class ShaderResource
    {
    public:
        // �ӱ������ֽ�������ʼ��Shader Resource
        ShaderResource(ID3DBlob* pShaderBytecode,
                       const ShaderDesc& shaderDesc);

        ShaderResource(const ShaderResource&) = delete;
        ShaderResource(ShaderResource&&) = delete;
        ShaderResource& operator = (const ShaderResource&) = delete;
        ShaderResource& operator = (ShaderResource&&) = delete;

        UINT32 GetCBNum() const noexcept { return m_CBs.size(); }
        UINT32 GetTexSRVNum() const noexcept { return m_TextureSRVs.size(); }
        UINT32 GetTexUAVNum() const noexcept { return m_TextureUAVs.size(); }
        UINT32 GetBufSRVNum() const noexcept { return m_BufferSRVs.size(); }
        UINT32 GetBufUAVNum() const noexcept { return m_BufferUAVs.size(); }

        const ShaderResourceAttribs& GetCB(UINT32 n) const noexcept { assert(n > 0 && n < m_CBs.size()); return m_CBs[n]; }
        const ShaderResourceAttribs& GetTexSRV(UINT32 n) const noexcept { assert(n > 0 && n < m_TextureSRVs.size()); return m_TextureSRVs[n]; }
        const ShaderResourceAttribs& GetTexUAV(UINT32 n) const noexcept { assert(n > 0 && n < m_TextureUAVs.size()); return m_TextureUAVs[n]; }
        const ShaderResourceAttribs& GetBufSRV(UINT32 n) const noexcept { assert(n > 0 && n < m_BufferSRVs.size()); return m_BufferSRVs[n]; }
        const ShaderResourceAttribs& GetBufUAV(UINT32 n) const noexcept { assert(n > 0 && n < m_BufferUAVs.size()); return m_BufferUAVs[n]; }

        SHADER_TYPE GetShaderType() const noexcept { return m_ShaderType; }

        template <typename THandleCB,
                  typename THandleTexSRV,
                  typename THandleTexUAV,
                  typename THandleBufSRV,
                  typename THandleBufUAV>
        void ProcessResources(THandleCB HandleCB,
                              THandleTexSRV HandleTexSRV,
                              THandleTexUAV HandleTexUAV,
                              THandleBufSRV HandleBufSRV,
                              THandleBufUAV HandleBufUAV) const
        {
            for (UINT32 i = 0;i < m_CBs.size();++i)
                HandleCB(GetCB(i), i);

            for (UINT32 i = 0; i < m_TextureSRVs.size(); ++i)
                HandleTexSRV(GetTexSRV(i), i);

            for (UINT32 i = 0; i < m_TextureUAVs.size(); ++i)
                HandleTexUAV(GetTexUAV(i), i);

            for (UINT32 i = 0; i < m_BufferSRVs.size(); ++i)
                HandleBufSRV(GetBufSRV(i), i);

            for (UINT32 i = 0; i < m_BufferUAVs.size(); ++i)
                HandleBufUAV(GetBufUAV(i), i);
        }

        bool IsCompatibleWith(const ShaderResource& shaderResource) const;
        const std::string& GetShaderName() const { return m_ShaderName; }

        size_t GetHash() const;

        // ���ϲ����õ�PSO��ShaderVariableConfig�ҵ���ӦShaderResource��Variable Type��Static��Mutable��Dynamic��
        SHADER_RESOURCE_VARIABLE_TYPE FindVariableType(const ShaderResourceAttribs& ResourceAttribs,
            const ShaderVariableConfig& shaderVariableConfig) const;

        void GetShaderModel(UINT32& Major, UINT32& Minor) const
        {
            Major = (m_ShaderVersion & 0x000000F0) >> 4;
            Minor = (m_ShaderVersion & 0x0000000F);
        }

    private:
        // TODO:����ʹ��һ��Vector���Ż�
    	// ��������Ŀ���ǰ�D3D�в�ͬ������ת��Ϊ�⼸�����ͣ�CBV��TexSRV��TexUAV��BufferSRV��BufferUAV
        std::vector<ShaderResourceAttribs> m_CBs;
        std::vector<ShaderResourceAttribs> m_TextureSRVs;
        std::vector<ShaderResourceAttribs> m_TextureUAVs;
        std::vector<ShaderResourceAttribs> m_BufferSRVs;
        std::vector<ShaderResourceAttribs> m_BufferUAVs;

        const SHADER_TYPE m_ShaderType;

        UINT32 m_ShaderVersion = 0;

        std::string m_ShaderName;
    };
}

namespace std
{
    template<>
    struct hash<RHI::ShaderResourceAttribs>
    {
        size_t operator()(const RHI::ShaderResourceAttribs& attribs) const
        {
            return attribs.GetHash();
        }
    };

    template<>
    struct hash<RHI::ShaderResource>
    {
        size_t operator()(const RHI::ShaderResource& shaderResource) const
        {
            return shaderResource.GetHash();
        }
    };
}