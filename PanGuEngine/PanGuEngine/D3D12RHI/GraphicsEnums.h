#pragma once

namespace RHI
{
	/**
	* ��������Դ���԰󶨵����ߵ��ĸ��׶�
	* [D3D11_BIND_FLAG]: https://msdn.microsoft.com/en-us/library/windows/desktop/ff476085(v=vs.85).aspx
	*/
	enum class BIND_FLAGS
	{
		BIND_NONE = 0x0L,				// δ����
		BIND_VERTEX_BUFFER = 0x1L,		// Vertex Buffer
		BIND_INDEX_BUFFER = 0x2L,		// Index Buffer
		BIND_CONSTANT_BUFFER = 0x4L,	// Constant Buffer,���ܺ�����BIND_FLAG���
		BIND_SHADER_RESOURCE = 0x8L,	// Buffer��Texture��ΪShader Resource
		BIND_STREAM_OUTPUT = 0x10L,		// Stream Output
		BIND_RENDER_TARGET = 0x20L,		// Render Target
		BIND_DEPTH_STENCIL = 0x40L,		// Depth Stencil
		BIND_UNORDERED_ACCESS = 0x80L,	// Buffer��Texture��ΪUnordered Access
		BIND_INDIRECT_DRAW_AGRS = 0x100L// Indirect Draw����Ĳ���Buffer
	};


	/**
	* ������Դ��Usage
	* [D3D11_USAGE]: https://msdn.microsoft.com/en-us/library/windows/desktop/ff476259(v=vs.85).aspx
	*/
	enum class USAGE
	{
		// ֻ�ܱ�GPU�������ܱ�GPUд��CPU���ܶ�д����������Դ�ڴ���ʱ��Ҫ��ʼ����
		// ��ӦD3D��D3D11_USAGE_IMMUTABLE,OpenGL��GL_STATIC_DRAW
		USAGE_IMMUTABLE,	

		// GPU���Զ�д��CPU����ż��д�롣
		// ��ӦD3D��D3D11_USAGE_DEFAULT,OpenGL��GL_DYNAMIC_DRAW
		USAGE_DEFAULT,		

		// GPU�ɶ���CPU���ٿ���ÿ֡д��һ�Ρ�
		// ��ӦD3D��D3D11_USAGE_DYNAMIC,OpenGL��GL_STREAM_DRAW
		USAGE_DYNAMIC,

		// ��GPU��CPU�������ݡ�
		// ��ӦD3D��D3D11_USAGE_STAGING,OpenGL��GL_DYNAMIC_READ
		USSAGE_STAGING
	};

	/**
	* ��ԴMapʱ��CPU����Ȩ��
	* ֻ��USAGE_DYNAMIC����Դ���Ա�Map
	*/
	enum class CPU_ACCESS_FLAGS
	{
		CPU_ACCESS_NONE = 0x00,
		CPU_ACCESS_READ = 0x01,
		CPU_ACCESS_WRITE = 0x02
	};

	/**
	* ����Buffer�ķ���ģʽ
	* [D3D11_RESOURCE_MISC_FLAG]: https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_resource_misc_flag
	*/
	enum class BUFFER_MODE
	{
		BUFFER_MODE_UNDEFINED = 0,
		BUFFER_MODE_FORMATTED,
		BUFFER_MODE_STRUCTURED,
		BUFFER_MODE_RAW,
		BUFFER_MODE_NUM_MODES
	};

	/**
	* Buffer view������
	*/
	enum class BUFFER_VIEW_TYPE
	{
		BUFFER_VIEW_UNDEFINED = 0,
		BUFFER_VIEW_SHADER_RESOURCE,
		BUFFER_VIEW_UNORDERED_ACCESS,
		BUFFER_VIEW_NUM_VIEWS
	};

}