#include <exception>
#include <string>
#include <Windows.h>
#include <stdio.h>

#include "..\QuickInfos\StackWalker.h"
#include "..\QuickInfos\QuickInfoFuncDefs.h"

class CTraceableException : public std::exception {
	private:
		std::string errorMsg; 
	public:
		CTraceableException(const char* file, int line, const char *msg, ...);
		CTraceableException(const char* file, int line, std::exception& ex);
		~CTraceableException() {
			//nothing to do here.
		}

		virtual const char* what() const throw();

		virtual const char* traceableWhat(const char* file, unsigned int line) throw() {
			char num[0x10] = { 0x00 }; _itoa_s(line, num, 10);
			std::string fileStr = file; fileStr = fileStr.substr(fileStr.find_last_of("\\") + 1);
			this->errorMsg = this->errorMsg.append(fileStr);
			this->errorMsg = this->errorMsg.append("(Line: ").append(num).append(")\n");
			return this->errorMsg.c_str();
		}
		
		CTraceableException& operator=(const CTraceableException& rhs) {
			this->errorMsg = rhs.errorMsg;
			return (*this);
		}
};

#ifndef TraceableException
#define TraceableException(msg_or_previousException) CTraceableException( __FILE__, __LINE__, msg_or_previousException );
#define TraceableExceptionARGS(msg_with_vaArgs, ...) CTraceableException( __FILE__, __LINE__, msg_with_vaArgs, __VA_ARGS__); 
#endif

#define traceableWhat() traceableWhat(__FILE__, __LINE__)