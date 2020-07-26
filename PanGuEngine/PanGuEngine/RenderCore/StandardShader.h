#pragma once
#include "Shader.h"

class StandardShader : public Shader
{
private:
	void BindShaderFilePath() override;
	void BindShaderParam() override;
};
