#pragma once

#ifndef __ROSE_VFS__
#define __ROSE_VFS__

#include <Windows.h>
#include <string>

#include "..\..\QuickInfos\Trackable.hpp"

typedef DWORD(CALLBACK* OpenVFS_FUNCPTR)(const char*, const char*);
typedef DWORD(CALLBACK* CloseVFS_FUNCPTR)(DWORD fileHandle);
typedef DWORD(CALLBACK* GetVFSCount_FUNCPTR)(DWORD fileHandle);
typedef void(CALLBACK* GetVFSNames_FUNCPTR)(DWORD fileHandle, const char **array, DWORD maxLen);
typedef DWORD(CALLBACK* GetFileCount_FUNCPTR)(DWORD fileHandle, const char*);
typedef void(CALLBACK* GetFileNames_FUNCPTR)(DWORD fileHandle, const char* name, const char **fileNameArray, DWORD, DWORD maxLen);
typedef DWORD(CALLBACK* OpenFile_FUNCPTR)(const char* fileName, DWORD fileHandle);
typedef DWORD(CALLBACK* RemoveFile_FUNCPTR)(DWORD handle, const char* fileName);
typedef void(CALLBACK* CloseFile_FUNCPTR)(DWORD fileHandle);
typedef DWORD(CALLBACK* ReadFile_FUNCPTR)(void* buffer, DWORD size, DWORD count, DWORD fileHandle);
typedef DWORD(CALLBACK* GetFileSize_FUNCPTR)(DWORD fileHandle);

class VFS {
	private:
		HMODULE triggerVFSDLL;
		std::string filePath;
		DWORD vfsHandle;
		DWORD currentFileHandle;

		OpenVFS_FUNCPTR OpenVFS;
		CloseVFS_FUNCPTR CloseVFS;
		GetVFSCount_FUNCPTR GetVFSCount;
		GetVFSNames_FUNCPTR GetVFSNames;
		GetFileCount_FUNCPTR GetFileCount;
		GetFileNames_FUNCPTR GetFileNames;
		OpenFile_FUNCPTR OpenFile;
		RemoveFile_FUNCPTR RemoveFile;
		CloseFile_FUNCPTR CloseFile;
		ReadFile_FUNCPTR ReadFile;
		GetFileSize_FUNCPTR GetFileSize;
	public:
		VFS(const char* gameFolder);
		~VFS();

		DWORD readFile(const char *pathInVFS, char** ppBuffer);
		void readFile(const char *pathInVFS, Trackable<char>& buf);
};

#endif //__ROSE_VFS__