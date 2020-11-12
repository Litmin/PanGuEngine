#pragma once

#include <fstream>

enum class LOG_LEVEL
{
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR
};

class Debug
{
public:
    
    static void Log(LOG_LEVEL logLevel,
                    const char* function, 
                    const char* file, 
                    int line, 
                    const std::string& message);


private:
    static std::string m_Message;
};

// 宏定义中使用do while可以保证多行语句的宏在任何情况下都是正确的，例如没有括号的if语句
#define LOG(message)                                                                      \
    do                                                                                    \
    {                                                                                     \
        Debug::Log(LOG_LEVEL::LOG_LEVEL_INFO, __FUNCTION__, __FILE__, __LINE__, message); \
    } while(false)

#define LOG_WARNING(message)                                                                      \
    do                                                                                    \
    {                                                                                     \
        Debug::Log(LOG_LEVEL::LOG_LEVEL_WARNING, __FUNCTION__, __FILE__, __LINE__, message); \
    } while(false)

#define LOG_ERROR(message)                                                                      \
    do                                                                                    \
    {                                                                                     \
        Debug::Log(LOG_LEVEL::LOG_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, message); \
    } while(false)