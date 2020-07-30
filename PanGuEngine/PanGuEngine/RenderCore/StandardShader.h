#pragma once
#include "Shader.h"

class StandardShader : public Shader
{
public:
	StandardShader(ID3D12Device* device);
	virtual ~StandardShader() = default;
private:
	void BindShaderFilePath() override;
	void BindShaderParam() override;
};
