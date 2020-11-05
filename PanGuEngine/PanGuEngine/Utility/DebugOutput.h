#pragma once

enum DEBUG_MESSAGE_SEVERITY
{
    DEBUG_MESSAGE_SEVERITY_INFO = 0,
    DEBUG_MESSAGE_SEVERITY_WARNING,
    DEBUG_MESSAGE_SEVERITY_ERROR,
    DEBUG_MESSAGE_SEVERITY_FATAL_ERROR
};

template <typename SSType, typename ArgType>
void FormatStrSS(SSType& ss, const ArgType& Arg)
{
    ss << Arg;
}

template <typename SSType, typename FirstArgType, typename... RestArgsType>
void FormatStrSS(SSType& ss, const FirstArgType& FirstArg, const RestArgsType&... RestArgs)
{
    FormatStrSS(ss, FirstArg);
    FormatStrSS(ss, RestArgs...); // recursive call using pack expansion syntax
}

template <typename... RestArgsType>
std::string FormatString(const RestArgsType&... Args)
{
    std::stringstream ss;
    FormatStrSS(ss, Args...);
    return ss.str();
}


std::string FormatDebugMessage(DEBUG_MESSAGE_SEVERITY Severity,
    const char* Message,
    const char* Function, // type of __FUNCTION__
    const char* File,     // type of __FILE__
    int                    Line)
{
    std::stringstream msg_ss;

    static const char* const strSeverities[] = { "Info", "Warning", "ERROR", "CRITICAL ERROR" };
    const auto* MessageSevery = strSeverities[static_cast<int>(Severity)];

    msg_ss << "Diligent Engine: " << MessageSevery;
    if (Function != nullptr || File != nullptr)
    {
        msg_ss << " in ";
        if (Function != nullptr)
        {
            msg_ss << Function << "()";
            if (File != nullptr)
                msg_ss << " (";
        }

        if (File != nullptr)
        {
            msg_ss << File << ", " << Line << ')';
        }
    }
    msg_ss << ": " << Message << '\n';

    return msg_ss.str();
}

void OutputDebugMessage(DEBUG_MESSAGE_SEVERITY Severity, const char* Message, const char* Function, const char* File, int Line)
{
    auto msg = FormatDebugMessage(Severity, Message, Function, File, Line);
    OutputDebugStringA(msg.c_str());

    if (Severity == DEBUG_MESSAGE_SEVERITY_ERROR || Severity == DEBUG_MESSAGE_SEVERITY_FATAL_ERROR)
        std::cerr << msg;
    else
        std::cout << msg;
}

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