#pragma once

#define NOMINMAX
#define _WINSOCKAPI_
#include <windows.h>
#include <Commdlg.h>
#include <iostream>
#include <vector>
#include <Psapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "psapi.lib")

namespace QuickInfo {
	#ifndef WINDOW_SEARCH_TYPES
	#define WINDOW_SEARCH_TYPES
	#define WINDOW_SHARP_SEARCH 0x10000000
	#define WINDOW_FUZZY_SEARCH 0x20000000
	#define WINDOW_THREAD_SEARCH 0x40000000
	#define WINDOW_HANDLE_SEARCH 0x80000000
	#endif

#pragma warning(disable:4996)
#ifndef __arg_converterA
	#define __arg_converterA(result, fmt) char* ap;\
		va_start(ap, fmt);\
		unsigned int minLenPos = 0; DWORD minLen[2] = { 0x00 };\
		DWORD dTmp; bool afterComma = false;\
		int iTmp; double fTmp; const char *cTmp = NULL; const wchar_t *wTmp = NULL; void *ptr = NULL;\
		std::string strFmt = "";\
		char a = 0;\
		char numBuf[0x20] = { 0x00 }; wchar_t wNumBuf[0x20] = { 0x00 };\
		while (*fmt) {\
			if (*fmt == '%') {\
				*fmt++;\
		LABEL_CHECK_LIMIT_ASCII:\
				if ( *fmt >= '0' && *fmt <= '9') {\
					while (*fmt >= '0' && *fmt <= '9') {\
						minLen[minLenPos] = minLen[minLenPos] * 10 + *fmt - 0x30;\
						*fmt++;\
					}\
				}\
				if (*fmt == '.') {\
					minLenPos++;\
					*fmt++;\
					goto LABEL_CHECK_LIMIT_ASCII;\
				}\
				switch (*fmt++) {\
					case 0:\
						result += '%'; \
					break;\
					case 'c':\
						a = (char)va_arg(ap, unsigned char);\
						result += a;\
					break;\
					case 'd':\
					case 'i':\
						iTmp = va_arg(ap, int);\
						strFmt = "%";\
						if (minLen[0] > 0) {\
							_itoa_s(minLen[0], numBuf, 10);\
							strFmt += ::std::string(numBuf);\
						}\
						strFmt += "i";\
						sprintf_s(numBuf, 0x20, strFmt.c_str(), iTmp);\
						result += ::std::string(numBuf);\
						minLen[0] = 0x00;\
					break;\
					case 'f':\
						fTmp = va_arg(ap, double);\
						strFmt = "%";\
						if (minLen[0] > 0) {\
							_itoa_s(minLen[0], numBuf, 10);\
							strFmt += ::std::string(numBuf);\
						}\
						if (minLen[1] > 0) {\
							_itoa_s(minLen[1], numBuf, 10);\
							strFmt += strFmt.append(".").append(::std::string(numBuf));\
						}\
						strFmt += "f";\
						sprintf_s(numBuf, 0x20, strFmt.c_str(), fTmp);\
						result += ::std::string(numBuf);\
						minLen[0] = 0x00;\
						minLen[1] = 0x00;\
					break;\
					case 'p':\
						ptr = va_arg(ap, void*);\
						strFmt = "%"; \
						if (minLen[0] > 0) {\
							_itoa_s(minLen[0], numBuf, 10);\
							strFmt += ::std::string(numBuf);\
						}\
						strFmt += "p";\
						sprintf_s(numBuf, 0x20, strFmt.c_str(), reinterpret_cast<unsigned int>(ptr)); \
						result += ::std::string(numBuf);\
						minLen[0] = 0x00;\
					break;\
					case 's':\
						cTmp = va_arg(ap, char*);\
						dTmp = 0x00;\
						while (dTmp < strlen(cTmp)) {\
							result += cTmp[dTmp];\
							dTmp++; \
						}\
						dTmp = 0x00;\
					break;\
					case 'w':\
						wTmp = va_arg(ap, wchar_t*);\
						if (!wTmp) {\
							wTmp = va_arg(ap, const wchar_t*);\
						}\
						while (*wTmp) {\
							unsigned int higherByte = (*wTmp) >> 8 & 0xFF;\
							if (higherByte != 0)\
								result += higherByte;\
							result += (*wTmp & 0xFF);\
							*wTmp++;\
						}\
					break;\
					case 'x':\
						dTmp = va_arg(ap, unsigned int);\
						strFmt = "%";\
						if (minLen[0] > 0) {\
							_itoa_s(minLen[0], numBuf, 10);\
							strFmt += ::std::string(numBuf);\
						}\
						strFmt += "x"; \
						sprintf_s(numBuf, 0x20, strFmt.c_str(), dTmp);\
						result += ::std::string(numBuf);\
					break;\
					default:\
						result += '%';\
						result += *(fmt-1);\
					break;\
				}\
			}\
			else {\
				result += *fmt;\
				*fmt++;\
			}\
			minLenPos = 0; \
		}\
		va_end(ap)\

#endif

#ifndef __arg_converterW
	#define __arg_converterW(result, fmt) \
		::va_list ap; \
		va_start(ap, fmt);\
		::std::wstring wFmt = L""; int minLenPos = 0; DWORD minLen[2] = { 0x00 };\
		DWORD dTmp = 0; \
		int iTmp; double fTmp; char *cTmp = NULL; wchar_t *wTmp = NULL; void *ptr = NULL;\
		wchar_t a = 0; wchar_t wNumBuf[0x20] = { 0x00 };\
		while (*fmt) {\
			if (*fmt == '%') {\
				*fmt++;\
		LABEL_CHECK_LIMIT_UNICODE:\
				if (*fmt >= '0' && *fmt <= '9') {\
					while (*fmt >= '0' && *fmt <= '9') {\
						minLen[minLenPos] = minLen[minLenPos] * 10 + *fmt - 0x30;\
						*fmt++;\
					}\
				}\
				minLenPos = 0;\
				if (*fmt == '.') {\
					minLenPos++;\
					*fmt++;\
					goto LABEL_CHECK_LIMIT_UNICODE;\
				}\
				switch (*fmt++) {\
					case '\0':\
						result += *(fmt-1);\
					break;\
					case 'c':\
						a = (wchar_t)va_arg(ap, unsigned char);\
						result += (wchar_t)a;\
					break;\
					case 'd':\
					case 'i':\
						iTmp = va_arg(ap, int);\
						wFmt = L"%";\
						if (minLen[0] > 0) {\
							_itow_s(minLen[0], wNumBuf, 0x20, 10);\
							wFmt += ::std::wstring(wNumBuf);\
						}\
						wFmt += L"i";\
						wsprintf(wNumBuf, wFmt.c_str(), iTmp);\
						result += ::std::wstring(wNumBuf);\
						minLen[0] = 0x00; \
					break;\
					case 'f':\
						fTmp = va_arg(ap, double);\
						wFmt = L"%";\
						if (minLen[0] > 0) {\
							_itow_s(minLen[0], wNumBuf, 0x20, 10);\
							wFmt += ::std::wstring(wNumBuf);\
						}\
						if (minLen[1] > 0) {\
							_itow_s(minLen[1], wNumBuf, 0x20, 10);\
							wFmt += wFmt.append(L".").append(::std::wstring(wNumBuf));\
						}\
						wFmt += L"f";\
						swprintf_s(wNumBuf, 0x20, wFmt.c_str(), fTmp);\
						result += ::std::wstring(wNumBuf);\
						minLen[0] = 0x00;\
						minLen[1] = 0x00;\
					break;\
					case 'p':\
						ptr = va_arg(ap, void*);\
						wFmt = L"%";\
						if (minLen[0] > 0) {\
							_itow_s(minLen[0], wNumBuf, 0x20, 10);\
							wFmt += ::std::wstring(wNumBuf);\
						}\
						wFmt += L"p";\
						wsprintf(wNumBuf, wFmt.c_str(), reinterpret_cast<unsigned int>(ptr));\
						if(wcslen(wNumBuf)<minLen[0]) {\
							for(unsigned int i=0;i<minLen[0]-wcslen(wNumBuf);i++) {\
								result += '0';\
							}\
						}\
						result += ::std::wstring(wNumBuf);\
						minLen[0] = 0x00;\
					break;\
					case 's':\
						cTmp = va_arg(ap, char*);\
						dTmp = 0x00;\
						while (dTmp < strlen(cTmp)) {\
							result += cTmp[dTmp];\
							dTmp++; \
						}\
						dTmp = 0x00;\
					break;\
					case 'w': \
						wTmp = va_arg(ap, wchar_t*);\
						while (*wTmp) {\
							result += *wTmp;\
							*wTmp++;\
						}\
					break;\
					case 'x': \
						dTmp = va_arg(ap, unsigned int); \
						wFmt = L"%";\
						if (minLen[0] > 0) {\
							_itow_s(minLen[0], wNumBuf, 0x20, 10);\
							wFmt += ::std::wstring(wNumBuf);\
						}\
						wFmt += L"x";\
						wsprintf(wNumBuf, wFmt.c_str(), dTmp);\
						if(wcslen(wNumBuf)<minLen[0]) {\
							for(unsigned int i=0;i<minLen[0]-wcslen(wNumBuf);i++) {\
								result += '0';\
							}\
						}\
						result += ::std::wstring(wNumBuf);\
						minLen[0] = 0x00;\
					break;\
					default:\
						result += '%';\
						result += *(fmt-1);\
					break;\
				}\
			}\
			else {\
				result += *fmt;\
				*fmt++;\
			}\
		}\
		va_end(ap);\

#endif

	#ifndef ArgConverter
	#define ArgConverterA(resultName, fmt) std::string resultName = ""; __arg_converterA(resultName, fmt );
	#define ArgConverterW(resultName, fmt) std::wstring resultName = L""; __arg_converterW(resultName, fmt );
	#define ArgConverter ArgConverterW
	#endif

	#pragma warning(default:4996)
} 
//end namespace QuickInfo