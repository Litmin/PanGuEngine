// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
	#define NOMINMAX
#endif
#include <windows.h>


#include <d3d12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")


#include "Utility/d3dx12.h"
#include "Utility/DxException.h"
#include "Utility/PathUtil.h"
#include "Utility/d3dUtil.h"
#include "Utility/MathHelper.h"
#include "Utility/Singleton.h"

#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <string>
#include <exception>
#include <wrl.h>
#include <shellapi.h>
#include <cassert>
#include <iostream>

#endif //PCH_H
