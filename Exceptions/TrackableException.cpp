#include "..\QuickInfos\QuickInfo.h"

CTraceableException::CTraceableException(const char* file, int line, const char *msg, ...) {
	ArgConverterA(result, msg);
	char num[0x10] = { 0x00 }; _itoa_s(line, num, 10);
	this->errorMsg = "Exception: ";
	this->errorMsg = this->errorMsg.append(result).append("\n\nStacktrace:\n");
	QuickInfo::getCallStack(this->errorMsg);
}

CTraceableException::CTraceableException(const char* file, int line, std::exception& ex) {
	char num[0x10] = { 0x00 }; _itoa_s(line, num, 10);
	this->errorMsg = ex.what();
	std::string fileStr = file; fileStr = fileStr.substr(fileStr.find_last_of("\\") + 1);
	this->errorMsg = this->errorMsg.append(fileStr);
	this->errorMsg = this->errorMsg.append("(Line: ").append(num).append(")\n");
}

const char* CTraceableException::what() const throw() {
	return this->errorMsg.c_str();
}