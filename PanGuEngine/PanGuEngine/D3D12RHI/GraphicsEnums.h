#pragma once

/**
* 描述了资源可以绑定到管线的哪个阶段 
* [D3D11_BIND_FLAG]: https://msdn.microsoft.com/en-us/library/windows/desktop/ff476085(v=vs.85).aspx
*/
enum class PGBIND_FLAGS
{
	BIND_NONE = 0x0L,				// 未定义
	BIND_VERTEX_BUFFER = 0x1L,		// Vertex Buffer
	BIND_INDEX_BUFFER = 0x2L,		// Index Buffer
	BIND_CONSTANT_BUFFER = 0x4L,	// Constant Buffer,不能和其他BIND_FLAG组合
	BIND_SHADER_RESOURCE = 0x8L,	// Buffer或Texture作为Shader Resource
	BIND_STREAM_OUTPUT = 0x10L,		// Stream Output
	BIND_RENDER_TARGET = 0x20L,		// Render Target
	BIND_DEPTH_STENCIL = 0x40L,		// Depth Stencil
	BIND_UNORDERED_ACCESS = 0x80L,	// Buffer或Texture作为Unordered Access
	BIND_INDIRECT_DRAW_AGRS = 0x100L// Indirect Draw命令的参数Buffer
};


/**
* 描述资源的Usage
* [D3D11_USAGE]: https://msdn.microsoft.com/en-us/library/windows/desktop/ff476259(v=vs.85).aspx
*/
enum class USAGE
{
	USAGE_IMMUTABLE,	// 只能被GPU读，不能被GPU写，CPU不能读写。该类型资源在创建时就要初始化。
	USAGE_DEFAULT,
	USAGE_DYNAMIC,
	USSAGE_STAGING
};

/**
* 资源Map时的CPU访问权限
* 只有USAGE_DYNAMIC的资源可以被Map
*/
enum class CPU_ACCESS_FLAGS
{
	CPU_ACCESS_NONE		= 0x00,
	CPU_ACCESS_READ		= 0x01,
	CPU_ACCESS_WRITE	= 0x02
};

/**
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

/// Buffer view type

/// This enumeration describes allowed view types for a buffer view. It is used by BufferViewDesc
/// structure.
enum class BUFFER_VIEW_TYPE
{
	BUFFER_VIEW_UNDEFINED = 0,
	BUFFER_VIEW_SHADER_RESOURCE,
	BUFFER_VIEW_UNORDERED_ACCESS,
	BUFFER_VIEW_NUM_VIEWS
};
