#include "pch.h"
#include "Debug.h"

using namespace std;

string Debug::m_Message = "";

void Debug::Log(LOG_LEVEL logLevel,
				const char* function,
				const char* file,
				int line,
				const std::string& message)
{
	switch (logLevel)
	{
	case LOG_LEVEL::LOG_LEVEL_INFO:
		m_Message += "[INFO]: ";
		break;
	case LOG_LEVEL::LOG_LEVEL_WARNING:
		m_Message += "[WARNING]: ";
		break;
	case LOG_LEVEL::LOG_LEVEL_ERROR:
		m_Message += "[ERROR]: ";
		break;
	default:
		break;
	}

	m_Message += message;
	m_Message += " In ";
	m_Message += function;
	m_Message += " (";
	m_Message += file;
	m_Message += "):";
	m_Message += std::to_string(line);
	m_Message += "\n";

	OutputDebugStringA(m_Message.c_str());

	m_Message.clear();
}
