#pragma once

#ifndef __MYFILE__
#define __MYFILE__

#include <iostream>
#include <cstdarg>
#include <time.h>
#include "..\QuickInfos\AdditionalFunctions.h"
#include "..\QuickInfos\Trackable.hpp"

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef _ACCESS_RIGHTS
#define _ACCESS_RIGHTS
#define _ACCESS_RIGHTS_READ 0x01
#define _ACCESS_RIGHTS_BINARY 0x02
#define _ACCESS_RIGHTS_PLUS 0x04
#define _ACCESS_RIGHTS_WRITE 0x08
#define _ACCESS_RIGHTS_APPEND 0x10
#endif

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#pragma warning(disable:4996)

class CMyFile
{
	private:
		FILE *handle;
		wchar_t wPath[MAX_PATH];
		char cPath[MAX_PATH];
		wchar_t accessRights[3];
		bool immediateSafe;
		bool unicode;
	public:
		CMyFile();
		CMyFile(const char *path, const char *accessRights);
		CMyFile(const wchar_t *path, const wchar_t *accessRights);
		~CMyFile();

		/*
		bool operator+(wchar_t* const pathAndRightsSeparatedBySemicolon) {
			std::wstring tokenizer = std::wstring(pathAndRightsSeparatedBySemicolon);

			DWORD tokenPos = tokenizer.find(L";");

			wcscpy(this->wPath, tokenizer.substr(0, tokenPos == -1 ? tokenizer.length() : tokenPos).c_str());
			wcscpy(this->accessRights, tokenPos == -1 ? L"rb" : tokenizer.substr(tokenPos + 1).c_str());

			return this->openFile(this->wPath, this->accessRights);
		}
		*/

		bool operator=(const CMyFile& file) {
			wcscpy(accessRights, file.accessRights);
			if (wcslen(file.wPath) > 3) {
				memset(cPath, 0x00, MAX_PATH);
				wcscpy(wPath, file.wPath);
				handle = nullptr;
			}
			if (strlen(file.cPath)>3) {
				memset(wPath, 0x00, MAX_PATH*sizeof(wchar_t));
				strcpy(cPath, file.cPath); 
				char tmp[3] = { 0x00 };
				int idx = 0;
				while (this->accessRights[idx] && idx < 3) {
					tmp[idx] = this->accessRights[idx] & 0xFF;
					idx++;
				}
				handle = nullptr;
			}
			immediateSafe = file.immediateSafe;
			unicode = file.unicode;
			return true;
		}


		//Functions
		bool openFile(const char *, const char *);
		bool openFile(const wchar_t *, const wchar_t *);
		void clear();

		bool getLine(char *ptr, DWORD ptrLen, DWORD line = 0);
		bool getLine(wchar_t *ptr, DWORD ptrLen, DWORD line = 0);

		template<class _Ty> const _Ty read(const DWORD bytesToSkipAfterReading = 0x00) {
			if(bytesToSkipAfterReading>0)
				this->skip(bytesToSkipAfterReading);
			_Ty res = 0x00;
			fread(&res, sizeof(_Ty), 1, this->handle);
			return _Ty(res);
		}
		template<class _TyLen, class _Ty> _TyLen readStringT( _Ty* buf ) {
			_TyLen len = 0x00;
			fread(&len, sizeof(_TyLen), 1, this->handle);
			fread(buf, 1, len, this->handle);
			buf[len] = 0x00;
			return len;
		}

		template<class _Ty> BYTE readString( _Ty* buf ) {
			BYTE len = 0x00;
			fread(&len, sizeof(BYTE), 1, this->handle);
			fread(buf, 1, len, this->handle);
			buf[len] = 0x00;
			return len;
		}
		template<class _TyLen, class _Ty> _TyLen readString( _TyLen len, _Ty* buf ) {
			_TyLen readBytes = fread(buf, 1, len, this->handle);
			buf[len] = 0x00;
			return readBytes;
		}
		template<class _TyLen> void readAndAlloc(Trackable<char>& data, const _TyLen length) {
			char* tmpData = new char[length];
			this->readString(length, tmpData);
			data.init(tmpData, length);

			delete[] tmpData;
			tmpData = nullptr;
		}
		__inline DWORD skip(const DWORD len) {
			DWORD previousPos = ftell(this->handle);
			fseek(this->handle, previousPos + len, SEEK_SET);
			return static_cast<DWORD>(ftell(this->handle) - previousPos);
		}
		__inline void setPosition(const DWORD pos) {
			fseek(this->handle, pos, SEEK_SET);
		}

		const DWORD getTotalSize() {
			DWORD curPos = ftell(this->handle);
			fseek(this->handle, 0, SEEK_END);
			DWORD result = ftell(this->handle);
			fseek(this->handle, curPos, SEEK_SET);
			return result;
		}

		bool writePlain(void *, unsigned int, unsigned int);
		bool putString(const char *, bool attachTime = false);
		bool putString(std::string cStr, bool attachTime = false) { return putString(cStr.c_str(), attachTime); }
		bool putString(const wchar_t *, bool attachTime = false);
		bool putString(std::wstring wStr, bool attachTime = false) { return putString(wStr.c_str(), attachTime); }
		bool putStringWithVarOnly(const char*, ...);
		bool putStringWithVarOnly(const wchar_t*, ...);
		bool putStringWithVarAndTime(const char*, ...);
		bool putStringWithVarAndTime(const wchar_t*, ...);

		//For debug purposes
		__forceinline bool debugPutStringA(const char*, unsigned int, const char *, bool attachTime = false);
		__forceinline bool debugPutStringA(const char*, unsigned int, std::string cStr, bool attachTime = false);
		__forceinline bool debugPutStringW(const wchar_t*, unsigned int, const wchar_t *, bool attachTime = false);
		__forceinline bool debugPutStringW(const wchar_t*, unsigned int, std::wstring wStr, bool attachTime = false);
		__forceinline bool debugPutStringWithVarA(const char*, unsigned int, const char* fmt, ...);
		__forceinline bool debugPutStringWithVarW(const wchar_t*, unsigned int, const wchar_t* fmt, ...);

		wchar_t *getPathW() { return this->wPath; }
		char *getPathA() { return this->cPath; }
		bool isUnicode() { return this->unicode; }
		bool exists();
		bool reopen();
		bool close();
};

class CMyBufferedReader {
	private:
		char *buffer;
		DWORD totalSize;
		DWORD offset;
	public:
		CMyBufferedReader(char* newBuffer, const DWORD newTotalSize) {
			this->buffer = newBuffer;
			this->totalSize = newTotalSize;
		}
		template<class _Ty> __inline _Ty read() {
			if (std::is_arithmetic<_Ty>::value && this->offset < this->totalSize) {
				_Ty value = *(reinterpret_cast<_Ty*>(&this->buffer[offset]));
				offset += sizeof(_Ty);
				return value;
			}
			return _Ty(0x00);
		}
		std::string readStringZero() {
			std::string res = "";
			DWORD strLen = 0x00;
			while ((this->offset + strLen) < this->totalSize && this->buffer[this->offset + strLen]) {
				res += this->buffer[this->offset + strLen];
				strLen++;
			}
			this->offset += strLen + 1;
			return std::string(res);
		}
		template<class _TyLen, class _Ty> void readString(const _TyLen lengthToRead, _Ty buffer) {
			memcpy(buffer, &this->buffer[offset], lengthToRead);
			this->offset += lengthToRead;
			buffer[lengthToRead] = 0x00; 
		}
		template<class _TyLen, class _Ty> void readStringT(_Ty buffer) {
			_TyLen length = *(reinterpret_cast<_TyLen*>(&this->buffer[this->offset]));
			this->offset += sizeof(_TyLen);
			this->readString(length, buffer);
		}
		template<class _TyLen> void readAndAlloc(Trackable<char>* data, const _TyLen length) {
			std::vector<char> buffer(length+1);
			this->readString(length, buffer.data());
			data->init(buffer.data(), length);
		}
		__inline void skip(const DWORD bytesToSkip) { this->offset += bytesToSkip; }
		__inline void setPosition(const DWORD newPos) { this->offset = newPos; }
		DWORD getTotalSize() { return this->totalSize; }
};

#define putStringWithVar putStringWithVarAndTime

void logging(CMyFile logFile, const char *func);
void logging(CMyFile logFile, const wchar_t *func);

#pragma warning(default:4996)

#endif //__MYFILE__