#pragma once

#ifndef __PRIVATE_LOGGER__
#define __PRIVATE_LOGGER__

#include "D:\Programmieren\QuickInfos\VarsToString"
#include "D:\Programmieren\CMyFile\MyFile.h"

class Logger {
	private:
		CMyFileWriter<char>* file;
	public:
		Logger() {
		}
		~Logger() {
			delete file;
			file = nullptr;
		}
		void setFile(const char* path){ 
			file = new CMyFileWriter<char>(path, true);
		}
		void logDebug(const char* fmt, ...) {
			va_list args; va_start(args, fmt);
			std::string result = QuickInfo::convertVarsToString(fmt, args);
			va_end(args);
			file->putStringWithVar(result.c_str());
		}
};

extern Logger logger;

#endif //__PRIVATE_LOGGER__
