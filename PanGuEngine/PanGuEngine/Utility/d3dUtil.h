#pragma once

//using namespace DirectX;
using Microsoft::WRL::ComPtr;

class d3dUtil
{
public:
	static Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
		const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);
};

