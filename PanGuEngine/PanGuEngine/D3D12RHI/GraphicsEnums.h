#pragma once

namespace RHI
{
	/**
	* 描述了资源可以绑定到管线的哪个阶段
	* [D3D11_BIND_FLAG]: https://msdn.microsoft.com/en-us/library/windows/desktop/ff476085(v=vs.85).aspx
	*/
	enum BIND_FLAGS
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
	enum USAGE
	{
		// 只能被GPU读，不能被GPU写，CPU不能读写。该类型资源在创建时就要初始化。
		// 对应D3D的D3D11_USAGE_IMMUTABLE,OpenGL的GL_STATIC_DRAW
		USAGE_IMMUTABLE,	

		// GPU可以读写，CPU可以偶尔写入。
		// 对应D3D的D3D11_USAGE_DEFAULT,OpenGL的GL_DYNAMIC_DRAW
		USAGE_DEFAULT,		

		// GPU可读，CPU至少可以每帧写入一次。
		// 对应D3D的D3D11_USAGE_DYNAMIC,OpenGL的GL_STREAM_DRAW
		USAGE_DYNAMIC,

		// 从GPU向CPU传递数据。
		// 对应D3D的D3D11_USAGE_STAGING,OpenGL的GL_DYNAMIC_READ
		USAGE_STAGING
	};

	/**
	* 资源Map时的CPU访问权限
	* 只有USAGE_DYNAMIC的资源可以被Map
	*/
	enum CPU_ACCESS_FLAGS
	{
		CPU_ACCESS_NONE = 0x00,
		CPU_ACCESS_READ = 0x01,
		CPU_ACCESS_WRITE = 0x02
	};

	/**
	* 定义Buffer的访问模式
	* [D3D11_RESOURCE_MISC_FLAG]: https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_resource_misc_flag
	*/
	enum BUFFER_MODE
	{
		BUFFER_MODE_UNDEFINED = 0,
		BUFFER_MODE_FORMATTED,
		BUFFER_MODE_STRUCTURED,
		BUFFER_MODE_RAW,
		BUFFER_MODE_NUM_MODES
	};

	/**
	* Buffer view的类型
	*/
	enum BUFFER_VIEW_TYPE
	{
		BUFFER_VIEW_UNDEFINED = 0,
		BUFFER_VIEW_CONSTANT_BUFFER,
		BUFFER_VIEW_SHADER_RESOURCE,
		BUFFER_VIEW_UNORDERED_ACCESS,
		BUFFER_VIEW_NUM_VIEWS
	};

	/**
	* 描述Value Type
	* BufferView使用VALUE_TYPE描述Formatted Buffer
	* DrawAttribs用来描述Index的类型
	*/
	enum VALUE_TYPE
	{
		VT_UNDEFINED = 0,
		VT_INT8,
		VT_INT16,
		VT_INT32,
		VT_UINT8,
		VT_UINT16,
		VT_UINT32,
		VT_FLOAT16,
		VT_FLOAT32,
		VT_NUM_TYPES
	};

	enum RESOURCE_DIMENSION
	{
		RESOURCE_DIM_UNDEFINED = 0,		///< Texture type undefined
		RESOURCE_DIM_BUFFER,            ///< Buffer
		RESOURCE_DIM_TEX_1D,            ///< One-dimensional texture
		RESOURCE_DIM_TEX_1D_ARRAY,      ///< One-dimensional texture array
		RESOURCE_DIM_TEX_2D,            ///< Two-dimensional texture
		RESOURCE_DIM_TEX_2D_ARRAY,      ///< Two-dimensional texture array
		RESOURCE_DIM_TEX_3D,            ///< Three-dimensional texture
		RESOURCE_DIM_TEX_CUBE,          ///< Cube-map texture
		RESOURCE_DIM_TEX_CUBE_ARRAY,    ///< Cube-map array texture
		RESOURCE_DIM_NUM_DIMENSIONS     ///< Helper value that stores the total number of texture types in the enumeration
	};

	enum MISC_TEXTURE_FLAGS
	{
		MISC_TEXTURE_FLAG_NONE = 0x00,

		/// Allow automatic mipmap generation with ITextureView::GenerateMips()
		/// \note A texture must be created with BIND_RENDER_TARGET bind flag
		MISC_TEXTURE_FLAG_GENERATE_MIPS = 0x01
	};

	enum TEXTURE_VIEW_TYPE
	{
		/// Undefined view type
		TEXTURE_VIEW_UNDEFINED = 0,

		/// A texture view will define a shader resource view that will be used 
		/// as the source for the shader read operations
		TEXTURE_VIEW_SHADER_RESOURCE,

		/// A texture view will define a render target view that will be used
		/// as the target for rendering operations
		TEXTURE_VIEW_RENDER_TARGET,

		/// A texture view will define a depth stencil view that will be used
		/// as the target for rendering operations
		TEXTURE_VIEW_DEPTH_STENCIL,

		/// A texture view will define an unordered access view that will be used
		/// for unordered read/write operations from the shaders
		TEXTURE_VIEW_UNORDERED_ACCESS,

		/// Helper value that stores that total number of texture views
		TEXTURE_VIEW_NUM_VIEWS
	};

	/// Describes allowed unordered access view mode
	enum UAV_ACCESS_FLAG
	{
		/// Access mode is unspecified
		UAV_ACCESS_UNSPECIFIED = 0x00,

		/// Allow read operations on the UAV
		UAV_ACCESS_FLAG_READ = 0x01,

		/// Allow write operations on the UAV
		UAV_ACCESS_FLAG_WRITE = 0x02,

		/// Allow read and write operations on the UAV
		UAV_ACCESS_FLAG_READ_WRITE = UAV_ACCESS_FLAG_READ | UAV_ACCESS_FLAG_WRITE
	};

		/// Texture view flags
	enum TEXTURE_VIEW_FLAGS
	{
		/// No flags
		TEXTURE_VIEW_FLAG_NONE = 0x00,

		/// Allow automatic mipmap generation for this view.
		/// This flag is only allowed for TEXTURE_VIEW_SHADER_RESOURCE view type.
		/// The texture must be created with MISC_TEXTURE_FLAG_GENERATE_MIPS flag.
		TEXTURE_VIEW_FLAG_ALLOW_MIP_MAP_GENERATION = 0x01,
	};

	/// This enumeration defines filter type. It is used by SamplerDesc structure to define min, mag and mip filters.
	/// \note On D3D11, comparison filters only work with textures that have the following formats: 
	/// R32_FLOAT_X8X24_TYPELESS, R32_FLOAT, R24_UNORM_X8_TYPELESS, R16_UNORM.
	enum FILTER_TYPE
	{
		FILTER_TYPE_UNKNOWN = 0,           ///< Unknown filter type
		FILTER_TYPE_POINT,                  ///< Point filtering
		FILTER_TYPE_LINEAR,                 ///< Linear filtering
		FILTER_TYPE_ANISOTROPIC,            ///< Anisotropic filtering
		FILTER_TYPE_COMPARISON_POINT,       ///< Comparison-point filtering
		FILTER_TYPE_COMPARISON_LINEAR,      ///< Comparison-linear filtering
		FILTER_TYPE_COMPARISON_ANISOTROPIC, ///< Comparison-anisotropic filtering
		FILTER_TYPE_MINIMUM_POINT,          ///< Minimum-point filtering (DX12 only)
		FILTER_TYPE_MINIMUM_LINEAR,         ///< Minimum-linear filtering (DX12 only)
		FILTER_TYPE_MINIMUM_ANISOTROPIC,    ///< Minimum-anisotropic filtering (DX12 only)
		FILTER_TYPE_MAXIMUM_POINT,          ///< Maximum-point filtering (DX12 only)
		FILTER_TYPE_MAXIMUM_LINEAR,         ///< Maximum-linear filtering (DX12 only)
		FILTER_TYPE_MAXIMUM_ANISOTROPIC,    ///< Maximum-anisotropic filtering (DX12 only)
		FILTER_TYPE_NUM_FILTERS             ///< Helper value that stores the total number of filter types in the enumeration
	};

	/// [D3D12_TEXTURE_ADDRESS_MODE]: https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_texture_address_mode
	/// Defines a technique for resolving texture coordinates that are outside of 
	/// the boundaries of a texture. The enumeration generally mirrors [D3D11_TEXTURE_ADDRESS_MODE][]/[D3D12_TEXTURE_ADDRESS_MODE][] enumeration. 
	/// It is used by SamplerDesc structure to define the address mode for U,V and W texture coordinates.
	enum TEXTURE_ADDRESS_MODE
	{
		/// Unknown mode
		TEXTURE_ADDRESS_UNKNOWN = 0,

		/// Tile the texture at every integer junction. \n
		/// Direct3D Counterpart: D3D11_TEXTURE_ADDRESS_WRAP/D3D12_TEXTURE_ADDRESS_MODE_WRAP. OpenGL counterpart: GL_REPEAT
		TEXTURE_ADDRESS_WRAP = 1,

		/// Flip the texture at every integer junction. \n
		/// Direct3D Counterpart: D3D11_TEXTURE_ADDRESS_MIRROR/D3D12_TEXTURE_ADDRESS_MODE_MIRROR. OpenGL counterpart: GL_MIRRORED_REPEAT
		TEXTURE_ADDRESS_MIRROR = 2,

		/// Texture coordinates outside the range [0.0, 1.0] are set to the 
		/// texture color at 0.0 or 1.0, respectively. \n
		/// Direct3D Counterpart: D3D11_TEXTURE_ADDRESS_CLAMP/D3D12_TEXTURE_ADDRESS_MODE_CLAMP. OpenGL counterpart: GL_CLAMP_TO_EDGE
		TEXTURE_ADDRESS_CLAMP = 3,

		/// Texture coordinates outside the range [0.0, 1.0] are set to the border color specified
		/// specified in SamplerDesc structure. \n
		/// Direct3D Counterpart: D3D11_TEXTURE_ADDRESS_BORDER/D3D12_TEXTURE_ADDRESS_MODE_BORDER. OpenGL counterpart: GL_CLAMP_TO_BORDER
		TEXTURE_ADDRESS_BORDER = 4,

		/// Similar to TEXTURE_ADDRESS_MIRROR and TEXTURE_ADDRESS_CLAMP. Takes the absolute 
		/// value of the texture coordinate (thus, mirroring around 0), and then clamps to 
		/// the maximum value. \n
		/// Direct3D Counterpart: D3D11_TEXTURE_ADDRESS_MIRROR_ONCE/D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE. OpenGL counterpart: GL_MIRROR_CLAMP_TO_EDGE
		/// \note GL_MIRROR_CLAMP_TO_EDGE is only available in OpenGL4.4+, and is not available until at least OpenGLES3.1
		TEXTURE_ADDRESS_MIRROR_ONCE = 5,

		/// Helper value that stores the total number of texture address modes in the enumeration
		TEXTURE_ADDRESS_NUM_MODES
	};

	/// [D3D12_COMPARISON_FUNC]: https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_comparison_func
	/// This enumeartion defines a comparison function. It generally mirrors [D3D11_COMPARISON_FUNC]/[D3D12_COMPARISON_FUNC] enum and is used by
	/// - SamplerDesc to define a comparison function if one of the comparison mode filters is used
	/// - StencilOpDesc to define a stencil function
	/// - DepthStencilStateDesc to define a depth function
	enum COMPARISON_FUNCTION
	{
		/// Unknown comparison function
		COMPARISON_FUNC_UNKNOWN = 0,

		/// Comparison never passes. \n
		/// Direct3D counterpart: D3D11_COMPARISON_NEVER/D3D12_COMPARISON_FUNC_NEVER. OpenGL counterpart: GL_NEVER.
		COMPARISON_FUNC_NEVER,

		/// Comparison passes if the source data is less than the destination data.\n
		/// Direct3D counterpart: D3D11_COMPARISON_LESS/D3D12_COMPARISON_FUNC_LESS. OpenGL counterpart: GL_LESS.
		COMPARISON_FUNC_LESS,

		/// Comparison passes if the source data is equal to the destination data.\n
		/// Direct3D counterpart: D3D11_COMPARISON_EQUAL/D3D12_COMPARISON_FUNC_EQUAL. OpenGL counterpart: GL_EQUAL.
		COMPARISON_FUNC_EQUAL,

		/// Comparison passes if the source data is less than or equal to the destination data.\n
		/// Direct3D counterpart: D3D11_COMPARISON_LESS_EQUAL/D3D12_COMPARISON_FUNC_LESS_EQUAL. OpenGL counterpart: GL_LEQUAL.
		COMPARISON_FUNC_LESS_EQUAL,

		/// Comparison passes if the source data is greater than the destination data.\n
		/// Direct3D counterpart: 3D11_COMPARISON_GREATER/D3D12_COMPARISON_FUNC_GREATER. OpenGL counterpart: GL_GREATER.
		COMPARISON_FUNC_GREATER,

		/// Comparison passes if the source data is not equal to the destination data.\n
		/// Direct3D counterpart: D3D11_COMPARISON_NOT_EQUAL/D3D12_COMPARISON_FUNC_NOT_EQUAL. OpenGL counterpart: GL_NOTEQUAL.
		COMPARISON_FUNC_NOT_EQUAL,

		/// Comparison passes if the source data is greater than or equal to the destination data.\n
		/// Direct3D counterpart: D3D11_COMPARISON_GREATER_EQUAL/D3D12_COMPARISON_FUNC_GREATER_EQUAL. OpenGL counterpart: GL_GEQUAL.
		COMPARISON_FUNC_GREATER_EQUAL,

		/// Comparison always passes. \n
		/// Direct3D counterpart: D3D11_COMPARISON_ALWAYS/D3D12_COMPARISON_FUNC_ALWAYS. OpenGL counterpart: GL_ALWAYS.
		COMPARISON_FUNC_ALWAYS,

		/// Helper value that stores the total number of comparison functions in the enumeration
		COMPARISON_FUNC_NUM_FUNCTIONS
	};

	enum SHADER_TYPE
	{
		SHADER_TYPE_UNKNOWN = 0x000,        ///< Unknown shader type
		SHADER_TYPE_VERTEX = 0x001,         ///< Vertex shader
		SHADER_TYPE_PIXEL = 0x002,          ///< Pixel (fragment) shader
		SHADER_TYPE_GEOMETRY = 0x004,       ///< Geometry shader
		SHADER_TYPE_HULL = 0x008,           ///< Hull (tessellation control) shader
		SHADER_TYPE_DOMAIN = 0x010,         ///< Domain (tessellation evaluation) shader
		SHADER_TYPE_COMPUTE = 0x020,        ///< Compute shader
		SHADER_TYPE_AMPLIFICATION = 0x040,  ///< Amplification (task) shader
		SHADER_TYPE_MESH = 0x080,           ///< Mesh shader
		SHADER_TYPE_LAST = SHADER_TYPE_MESH
	};

	// 管线类型：Graphic、Compute、Mesh
	enum PIPELINE_TYPE
	{
		PIPELINE_TYPE_GRAPHIC,
		PIPELINE_TYPE_COMPUTE,
		PIPELINE_TYPE_MESH	// 19年刚出的新管线
	};

	// 按照更新频率，划分Shader变量
	enum SHADER_RESOURCE_VARIABLE_TYPE
	{
		/// Shader resource bound to the variable is the same for all SRB instances.
		/// It must be set *once* directly through Pipeline State object.
		SHADER_RESOURCE_VARIABLE_TYPE_STATIC = 0,

		/// Shader resource bound to the variable is specific to the shader resource binding
		/// instance (see Diligent::IShaderResourceBinding). It must be set *once* through
		/// Diligent::IShaderResourceBinding interface. It cannot be set through Diligent::IPipelineState
		/// interface and cannot be change once bound.
		SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,

		/// Shader variable binding is dynamic. It can be set multiple times for every instance of shader resource
		/// binding (see Diligent::IShaderResourceBinding). It cannot be set through Diligent::IPipelineState interface.
		SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC,

		/// Total number of shader variable types
		SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES
	};

	enum class CachedResourceType
	{
		Unknown = -1,
		CBV = 0,
		TexSRV,
		BufSRV,
		TexUAV,
		BufUAV,
		Sampler,
		NumTypes
	};
}