#pragma once

#ifndef __ROSE_VFS__
#define __ROSE_VFS__

#include <string>
#include <map>
#include <vector>

#include "D:\Programmieren\QuickInfos\Trackable.hpp"
#include "D:\Programmieren\CMyFile\MyFile.h"
#include "..\..\Common\Definitions.h"
#include "D:\Programmieren\Listener\Listener.h"
#include "D:\Programmieren\QuickInfos\Array.h"
#include "D:\Programmieren\QuickInfos\VirtualFileSystem.h"

typedef DWORD(__stdcall* OpenVFS_FUNCPTR)(const char*, const char*);
typedef DWORD(__stdcall* CloseVFS_FUNCPTR)(dword_t fileHandle);
typedef DWORD(__stdcall* GetVFSCount_FUNCPTR)(dword_t fileHandle);
typedef void(__stdcall* GetVFSNames_FUNCPTR)(dword_t fileHandle, char **array, dword_t arrayLen, word_t lengthOfSingleArrayItem);
typedef DWORD(__stdcall* GetFileCount_FUNCPTR)(dword_t fileHandle, const char* vfsName);
typedef void(__stdcall* GetFileNames_FUNCPTR)(dword_t fileHandle, const char* vfsName, char **fileNameArray, DWORD lengthOfArray, dword_t maxStringLength);
typedef DWORD(__stdcall* OpenFile_FUNCPTR)(const char* fileName, dword_t fileHandle);
typedef DWORD(__stdcall* RemoveFile_FUNCPTR)(dword_t handle, const char* fileName);
typedef void(__stdcall* CloseFile_FUNCPTR)(dword_t fileHandle);
typedef DWORD(__stdcall* ReadFile_FUNCPTR)(void* buffer, dword_t size, dword_t count, dword_t fileHandle);
typedef DWORD(__stdcall* GetFileSize_FUNCPTR)(dword_t fileHandle);

struct VFSData {
	RAII_Array<char> data;
	std::string filePath;
};

template<class _Ty> class Tree {
	private:
		VirtualFileSystem<_Ty> *storage;
	public:
		Tree() {
			storage = new VirtualFileSystem<_Ty>();
		}

		void appendPath(std::vector<_Ty> path) {
			typename VirtualFileSystem<_Ty>::Entry* current = storage->getRoot();
			for (_Ty value : path) {
				current = current->appendEntry(value);
			}
		}

		__inline bool existsPath(std::vector<_Ty> path) const {
			return storage->exists(path);
		}

		__inline std::string getExistingValuePath(std::vector<_Ty> path, const char *delimiter = " ") const {
			return storage->getExistingValuePath(path, delimiter);
		}

		template<class _Container> typename VirtualFileSystem<_Ty>::Entry *getEntry(_Container path) const {
			auto current = storage->getRoot();
			for (_Ty value : path) {
				current = current->getEntry(value);
				if (!current) {
					break;
				}
			}
			return current;
		}
};

typedef Tree<std::string> VFSTree;

class VFS {
	private:
		//Dirty work-around to avoid using <windows.h>; it would lead to several instances of "makro already defined"-situations.
		void* triggerVFSDLL;
		std::string filePath;
		dword_t vfsHandle;
		dword_t currentFileHandle;

		VFSTree* fileIndex;

		OpenVFS_FUNCPTR OpenVFS;
		CloseVFS_FUNCPTR CloseVFS;
		//VFSCount is too high (e.g. 5 VFS exists, but it returns 6 for the data.idx)
		GetVFSCount_FUNCPTR GetVFSCount;
		GetVFSNames_FUNCPTR GetVFSNames;
		GetFileCount_FUNCPTR GetFileCount;
		GetFileNames_FUNCPTR GetFileNames;
		OpenFile_FUNCPTR OpenFile;
		RemoveFile_FUNCPTR RemoveFile;
		CloseFile_FUNCPTR CloseFile;
		ReadFile_FUNCPTR ReadFile;
		GetFileSize_FUNCPTR GetFileSize;
		bool isLastIndexUpTodate(std::vector<std::string> vfsNames, CMyFileReader<char>* reader);
		void readFilePathsFromVFS(std::vector<std::string> vfsNames);
		void readFilePathsFromLog(CMyFileReader<char>* reader);

		void closeCurrentFile();
	public:
		VFS(const char* gameFolder);
		~VFS();

		void close();

		std::vector<std::string> getFileNamesFromFolder(const char* pathInVFS, const char* contentFilter, bool addDirectoryNameToResult=true, bool addSubdirectories = false);
		dword_t getFileSize(const char *pathInVFS);
		dword_t readFile(const char *pathInVFS, char** ppBuffer);
		void readFile(const char *pathInVFS, VFSData& buf);
};

#endif //__ROSE_VFS__