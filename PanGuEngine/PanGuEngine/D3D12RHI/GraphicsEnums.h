#pragma once

namespace RHI
{
	static constexpr UINT32 MAX_SHADERS_IN_PIPELINE = 5;


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

	enum class BindingResourceType
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