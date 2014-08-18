#pragma once

#ifndef __ROSE_VFS__
#define __ROSE_VFS__

#include <string>

#include "..\..\QuickInfos\Trackable.hpp"

typedef DWORD(__stdcall* OpenVFS_FUNCPTR)(const char*, const char*);
typedef DWORD(__stdcall* CloseVFS_FUNCPTR)(DWORD fileHandle);
typedef DWORD(__stdcall* GetVFSCount_FUNCPTR)(DWORD fileHandle);
typedef void(__stdcall* GetVFSNames_FUNCPTR)(DWORD fileHandle, const char **array, DWORD maxLen);
typedef DWORD(__stdcall* GetFileCount_FUNCPTR)(DWORD fileHandle, const char*);
typedef void(__stdcall* GetFileNames_FUNCPTR)(DWORD fileHandle, const char* name, const char **fileNameArray, DWORD, DWORD maxLen);
typedef DWORD(__stdcall* OpenFile_FUNCPTR)(const char* fileName, DWORD fileHandle);
typedef DWORD(__stdcall* RemoveFile_FUNCPTR)(DWORD handle, const char* fileName);
typedef void(__stdcall* CloseFile_FUNCPTR)(DWORD fileHandle);
typedef DWORD(__stdcall* ReadFile_FUNCPTR)(void* buffer, DWORD size, DWORD count, DWORD fileHandle);
typedef DWORD(__stdcall* GetFileSize_FUNCPTR)(DWORD fileHandle);

struct VFSData {
	Trackable<char> data;
	std::string filePath;
};

class VFS {
	private:

		//Dirty work-around to avoid using <windows.h>; it would lead to several instances of "makro already defined"-situations.
		void* triggerVFSDLL;
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

		void close();

		DWORD readFile(const char *pathInVFS, char** ppBuffer);
		void readFile(const char *pathInVFS, VFSData& buf);
};

#endif //__ROSE_VFS__