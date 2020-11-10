#pragma once

#include "pch.h"
#include <fstream>

class Debug
{
public:
    
    static void Log();
    static void LogWarning();
    static void LogError();


private:

};

template <typename... ArgsType>
void LogError(bool IsFatal, const char* Function, const char* FullFilePath, int Line, const ArgsType&... Args)
{
    std::string FileName(FullFilePath);

    auto LastSlashPos = FileName.find_last_of("/\\");
    if (LastSlashPos != std::string::npos)
        FileName.erase(0, LastSlashPos + 1);
    auto Msg = FormatString(Args...);
       
    OutputDebugMessage(IsFatal ? DEBUG_MESSAGE_SEVERITY_FATAL_ERROR : DEBUG_MESSAGE_SEVERITY_ERROR, Msg.c_str(), Function, FileName.c_str(), Line);
}

#define LOG_ERROR_AND_THROW(...)                                                                      \
    do                                                                                                \
    {                                                                                                 \
        Diligent::LogError<true>(/*IsFatal=*/false, __FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (false)