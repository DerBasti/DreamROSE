#pragma once

#ifndef __PRIVATE_LOGGER__
#define __PRIVATE_LOGGER__

#include "D:\Programmieren\QuickInfos\QuickInfo.h"
#include "D:\Programmieren\CMyFile\MyFile.h"

class Logger {
	private:
		CMyFile file;
	public:
		Logger() {
		}
		~Logger() {
			file.close();
		}
		void setFile(const char* path){ 
			file.openFile(path, "a+");
		}
		void logDebug(const char* fmt, ...) {
			::ArgConverterA(result, fmt);
			file.putStringWithVarAndTime(result.c_str());
		}
};

extern Logger logger;

#endif //__PRIVATE_LOGGER__
