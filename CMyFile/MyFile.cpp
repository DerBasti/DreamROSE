#include "MyFile.h"
#include "..\QuickInfos\QuickInfoFuncDefs.h"
#include "..\QuickInfos\QuickInfo.h"
#include <fstream>

#define ATTACH_TIME_VIA(func) SYSTEMTIME st; GetSystemTime(&st); \
		func('[', handle); \
		func((st.wHour / 10) + 0x30, handle); \
		func((st.wHour % 10) + 0x30, handle); \
		func(':', handle); \
		func((st.wMinute / 10) + 0x30, handle); \
		func((st.wMinute % 10) + 0x30, handle); \
		func(':', handle); \
		func((st.wSecond / 10) + 0x30, handle); \
		func((st.wSecond % 10) + 0x30, handle); \
		func('.', handle); \
		func((st.wMilliseconds / 100 % 10) + 0x30, handle); \
		func((st.wMilliseconds % 100 / 10) + 0x30, handle); \
		func((st.wMilliseconds % 10) + 0x30, handle); \
		func(']', handle); \
		func(' ', handle);

#pragma warning(disable:4996)

void logging(CMyFile logFile, const char *func) {
#ifdef _MY_DEBUG
	if (GetLastError() != 0) {
		logFile.putStringWithVar(L"%s returned an error: (0x%x)\n", func, GetLastError());
		SetLastError(0);
	}
	else {
		logFile.putStringWithVar(L"%s was successful\n", func);
	}
#endif
}

void logging(CMyFile logFile, const wchar_t *func) {
#ifdef _MY_DEBUG
	if (GetLastError() != 0) {
		logFile.putStringWithVar(L"%w returned an error: 0x%x\n", func, GetLastError());
		SetLastError(0);
	}
	else {
		logFile.putStringWithVar(L"%w was successful\n", func);
	}
#endif
}

CMyFile::CMyFile() {
	this->immediateSafe = false;
	memset(this->accessRights, 0x00, 3*sizeof(wchar_t));
	this->unicode = false;
	this->handle = nullptr;
}

CMyFile::CMyFile( const char *path, const char *accessRights)
{
	this->openFile(path, accessRights);
}

CMyFile::CMyFile(const wchar_t *path, const wchar_t *accessRights) {
	int idx = 0;
	wcscpy(this->accessRights, accessRights);
	this->unicode = true;
	handle = _wfopen(path, accessRights);

	//ERROR_FILE_ALREADY_EXISTS
	if (GetLastError() == 0xb7) {
		SetLastError(0);
	}
	wcscpy(this->wPath, path);
	memset(this->cPath, 0x00, MAX_PATH);
}

CMyFile::~CMyFile()
{
	if (this->handle)
		fclose(handle);
}

bool CMyFile::openFile(const char *path, const char *accessRights) {
	if (handle) {
		fclose(handle);
		this->handle = nullptr;
	}
	this->unicode = false;
	memset(this->accessRights, 0x00, 3 * sizeof(wchar_t));
	int idx = 0;
	while (*accessRights) {
		this->accessRights[idx] = *accessRights & 0xFF;
		*accessRights++;
		idx++;
	}
	//ERROR_FILE_ALREADY_EXISTS
	if (GetLastError() == 0xb7) {
		SetLastError(0);
	}
	if (path[0] == '.' && path[1] == '\\') {
		std::string workPath = "";
		QuickInfo::getPath(&workPath);
		workPath += &path[1];
		strcpy(this->cPath, workPath.c_str());
	}
	else {
		strcpy(this->cPath, path);
	}
	memset(this->wPath, 0x00, sizeof(wchar_t)*MAX_PATH);

	char access[4] = { this->accessRights[0] & 0xFF, this->accessRights[1] & 0xFF, this->accessRights[2] & 0xFF, 0x00 };
	this->handle = fopen(this->cPath, access);
	return this->handle != nullptr;
}


bool CMyFile::openFile(const wchar_t *path, const wchar_t *accessRights) {
	if (handle) {
		fclose(handle);
		this->handle = nullptr;
	}
	for (int i = 0; i < 3; i++) {
		this->accessRights[i] = 0x00;
	}
	this->unicode = true;
	this->immediateSafe = immediateSafe;
	wcscpy(this->accessRights, accessRights);
	wcscpy(this->wPath, path);

	//ERROR_FILE_ALREADY_EXISTS
	if (GetLastError() == 0xb7) {
		SetLastError(0);
	}
	return (handle != nullptr);
}

void CMyFile::clear() {
	if (this->handle)
		this->close();
	if (this->unicode) {
		std::wfstream f(this->wPath, std::ios::out | std::ios::trunc);
		f.close();
	}
	else {
		std::fstream f(this->cPath, std::ios::out | std::ios::trunc);
		f.close();
	}
}

bool CMyFile::reopen() {
	if (handle) 
		this->close();
	if (this->unicode && wcslen(this->wPath) > 4) {
		handle = _wfopen(this->wPath, accessRights);
	}
	if (!this->unicode && strlen(this->cPath) > 4) {
		char tmp[4] = { 0x00 };
		int idx = 0;
		while (this->accessRights[idx] && idx<3) {
			tmp[idx] = this->accessRights[idx] & 0xFF;
			idx++;
		}
		handle = fopen(this->cPath, tmp);
	}

	//ERROR_FILE_ALREADY_EXISTS
	if (GetLastError() == 0xb7) {
		SetLastError(0);
	}
	return (handle != nullptr);
}


bool CMyFile::getLine(char *ptr, DWORD ptrLen, DWORD line) {
	if (this->accessRights[0] == 'w' || this->accessRights[0] == 'a' || ptr == nullptr)
		return false;

	if (this->reopen()) {
		fseek(this->handle, 0, SEEK_END);
		DWORD maxLen = (DWORD)ftell(this->handle);
		rewind(this->handle);

		//make sure we don't get a loop without any breaks
		DWORD curLine = 0; DWORD curChar = 0;
		for (unsigned int i = 0; i < maxLen; i++) {
#pragma region UNICODE_PART
			if (curLine == line) {
				if (this->unicode) {
					for (unsigned int j = i, k = 0; j < maxLen; j++, k++) {
						if (this->accessRights[1] != 'b') //non binary
							ptr[k] = fgetwc(this->handle) & 0xFF;
						else
							ptr[k] = fgetc(this->handle);
						if (ptr[k] == 0x0A || ptr[k] == 0x0D) {
							ptr[k] = 0x00;
							return true;
						}
					}
#pragma endregion
#pragma region ANSI_PART
				}
				else { //ANSI
					for (unsigned int j = i, k = 0; j < maxLen; j++, k++) {
						ptr[k] = fgetc(this->handle);
						if (ptr[k] == 0x0A || ptr[k] == 0x0D) {
							ptr[k] = 0x00;
							return true;
						}
					}
				}
			}
			else {
				if (this->unicode) {
					if (this->accessRights[1] != 'b')
						curChar = fgetwc(this->handle);
					else
						curChar = fgetc(this->handle);
				}
				else
					curChar = fgetc(this->handle);
			}
			if (curChar == 0x0A) {
				curLine++; curChar = 0x00;
			}
			if (curLine > line)
				break;
#pragma endregion
		}
	}
	this->close();
	return false;
}

bool CMyFile::getLine(wchar_t *ptr, DWORD ptrLen, DWORD line) {
	if (this->accessRights[0] == 'w' || this->accessRights[0] == 'a' || ptr == nullptr)
		return false;

	if (this->reopen()) {
		fseek(this->handle, 0, SEEK_END);
		DWORD maxLen = (DWORD)ftell(this->handle);
		rewind(this->handle);

		//make sure we don't get a loop without any breaks
		DWORD curLine = 0; DWORD curChar = 0;
		for (unsigned int i = 0; i < maxLen;i++) {
#pragma region UNICODE_PART
			if (curLine == line) { //if the current line equals the one we're wanting
				if (this->unicode) {
					for (unsigned int j = i, k = 0; j < maxLen; j++, k++) {
						if (this->accessRights[1] != 'b')
							ptr[k] = fgetwc(this->handle);
						else
							ptr[k] = fgetc(this->handle);
						if (ptr[k] == 0x0A || ptr[k] == 0x0D) {
							ptr[k] = 0x00;
							return true;
						}
					}
				}
#pragma endregion
#pragma region ANSI_PART
				else { //ANSI
					for (unsigned int j = i, k = 0; j < maxLen; j++, k++) {
						ptr[k] = fgetc(this->handle);
						if (ptr[k] == 0x0A || ptr[k] == 0x0D) {
							ptr[k] = 0x00;
							return true;
						}
					}
				}
#pragma endregion
			}
			else {
				if (this->unicode) {
					if (this->accessRights[1] != 'b')
						curChar = fgetwc(this->handle);
					else
						curChar = fgetc(this->handle);
				}
				else
					curChar = fgetc(this->handle);
			}
			if (curChar == 0x0A) {
				curLine++; curChar = 0x00;
			}
			if (curLine > line)
				break;
		}
	}
	this->close();
	return false;
}

bool CMyFile::putString(const wchar_t *str, bool attachTime) {
	if (this->accessRights[0] == 'w' || this->accessRights[0] == 'a') {
		if (!this->reopen())
			return false;
		if (attachTime) {
			if(this->unicode) {
				ATTACH_TIME_VIA(fputwc);
			}
			else {
				ATTACH_TIME_VIA(fputc);
			}
		}
		if(this->unicode) {
			fputws(str, handle);
		}
		else {
			for(unsigned int i=0;i<wcslen(str);i++) {
				if (str[i] & 0xFF00)
					fputc(str[i]>>8&0xFF,handle);
				fputc(str[i]&0xFF,handle);
			}
		}
		this->close();
		return true;
	}
	return false;
}

bool CMyFile::putString(const char *str, bool attachTime) {
	if (this->accessRights[0] == 'w' || this->accessRights[0] == 'a') {
		if (!this->reopen())
			return false;
		if (attachTime) {
			if(this->unicode) {
				ATTACH_TIME_VIA(fputwc);
			} else {	
				ATTACH_TIME_VIA(fputc);
			}
		}
		if(this->unicode) {
			for(unsigned int i=0;i<strlen(str);i++) {
				fputc(0x00 << 8 | str[i], handle);
			}
		} else {
			fputs(str, handle);
		}
		this->close();
		return true;
	}
	return false;
}

bool CMyFile::writePlain(void *ptr, unsigned int sizeOfElement, unsigned int length) {
	if (!handle || !(this->accessRights[0] == 'w' || this->accessRights[0] == 'a'))
		return false;
	return (fwrite(ptr, sizeOfElement, length, handle) > 0);
}

bool CMyFile::putStringWithVarOnly(const char *fmt, ...) {
	if (!(this->accessRights[0] == 'w' || this->accessRights[0] == 'a'))
		return false;
	if (!this->reopen())
		return false;
	if (this->unicode) {
		ArgConverter( wResult, fmt);
		for (unsigned int i = 0; i < wResult.length(); i++) {
			if (this->accessRights[1] != 'b') //non binary
				fputwc(wResult.c_str()[i], handle);
			else {
				fputc(wResult.c_str()[i] >> 8 & 0xFF, handle);
				fputc(wResult.c_str()[i] & 0xFF, handle);
			}
		}
	}
	else {
		ArgConverterA( cResult, fmt);
		for (unsigned int i = 0; i < cResult.length(); i++) {
			fputc(cResult.c_str()[i], handle);
		}
	}
	this->close();
	return true;
}

bool CMyFile::putStringWithVarOnly(const wchar_t *fmt, ...) {
	if (!(this->accessRights[0] == 'w' || this->accessRights[0] == 'a'))
		return false;
	if (!this->reopen())
		return false;
	ArgConverter(wResult, fmt);
	for (unsigned int i = 0; i < wResult.length(); i++)
		fputwc(wResult.c_str()[i], handle);
	this->close();
	return true;
}

bool CMyFile::putStringWithVar(const char *fmt, ...) {
	if (!(this->accessRights[0] == 'w' || this->accessRights[0] == 'a'))
		return false;
	if (!this->reopen())
		return false;
	if (this->unicode) {
		ArgConverter(wResult, fmt);
		ATTACH_TIME_VIA(fputwc);
		for(unsigned int i=0;i<wResult.length();i++) {
			if (this->accessRights[1] != 'b') //non binary
				fputwc(wResult.c_str()[i], handle);
			else {
				fputc(wResult.c_str()[i] >> 8 & 0xFF, handle);
				fputc(wResult.c_str()[i] & 0xFF, handle);
			}
		}
	}
	else {
		ArgConverterA(cResult, fmt);
		for (unsigned int i = 0; i < cResult.length(); i++) {
			fputc(cResult.c_str()[i], handle);
		}
	}
	this->close();
	return true;
}

bool CMyFile::putStringWithVar(const wchar_t *fmt, ...) {
	if (!(this->accessRights[0] == 'w' || this->accessRights[0] == 'a'))
		return false;
	if (!this->reopen())
		return false;
	ArgConverter(wResult, fmt);
	ATTACH_TIME_VIA(fputwc);
	for (unsigned int i = 0; i < wResult.length(); i++) {
		if (this->accessRights[1] != 'b') //non binary
			fputwc(wResult.c_str()[i], handle);
		else {
			fputc(wResult.c_str()[i] >> 8 & 0xFF, handle);
			fputc(wResult.c_str()[i] & 0xFF, handle);
		}
	}
	this->close();
	return true;
}

bool CMyFile::exists() {
	if (wcslen(wPath) == 0 && strlen(cPath) == 0) {
		return false;
	}
	if(this->handle)
		return true;
	//path was already given, let's see whether it actually exists.
	if(this->reopen()) {
		this->close();
		return true;
	}
	return false;
}

bool CMyFile::close() {
	if (handle) {
		fclose(handle);
		this->handle = nullptr;
	}
	return (handle == nullptr);
}
#pragma warning(default:4996)