#pragma once

#ifndef __ROSE_VFS__
#define __ROSE_VFS__

#include <string>
#include <map>
#include <vector>

#include "D:\Programmieren\QuickInfos\Trackable.hpp"
#include "D:\Programmieren\CMyFile\MyFile.h"
#include "..\..\Common\Definitions.h"

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
	Trackable<char> data;
	std::string filePath;
};

class VFSTree {
	public:
		class Branch {
			private:
				friend class VFSTree;
				std::string name;
				std::string fullPath;
				struct _lowerLevel {
					Branch* first;
					Branch* end;
				} lowerLevel;
				Branch* parent;
				Branch* nextBranch;
				Branch* operator=(const Branch* newBranch) {
					this->name = newBranch->name;
					this->lowerLevel.first = newBranch->lowerLevel.first;
					this->lowerLevel.end = newBranch->lowerLevel.end;
					this->parent = newBranch->parent;
					this->nextBranch = newBranch->nextBranch;
					return this;
				}

				Branch* _getBranchByName(const char* branchName) const;
				std::vector<std::string> getAllChildren();
			public:
				Branch(std::string nodeName, Branch* parentBranch = nullptr);

				bool operator==(const Branch& branch) const;
				bool operator!=(const Branch& branch) const;

				Branch* appendLeaf(std::string value);
				const Branch* operator[](const char* branchName) const;
				const Branch* findBranchByPath(std::string path);
				std::vector<std::string> getAllNextLowerBranchNames(bool addDirectoryNameToResult, bool includeFolder) const;
				void establishPath(std::string path);

				__inline const Branch* findBranchByName(const char* branchName) const {
					return (*this)[branchName];
				}
				__inline void establishPath(const char* path) {
					return this->establishPath(std::string(path));
				}
				__inline std::string getName() const { return this->name; }
				__inline std::string getTotalPath() const { return this->fullPath; }
				__inline bool hasChildren() const {
					return (this->lowerLevel.first != nullptr);
				}
		};
	private:
		Branch* root;
	public:
		VFSTree() {
			this->root = new Branch("\\");
		}
		__inline Branch* getRoot() const { return this->root; }
		__inline const Branch* getBranchByPath(std::string value) const { return this->root->findBranchByPath(value); }
		__inline void appendFile(const char* filePath) { return this->appendFile(std::string(filePath)); }
		__inline void appendFile(std::string filePath) { this->root->establishPath(filePath); }
		std::vector<std::string> getFilesInPath(std::string path, const char* contentFilter = nullptr, bool addDirectoryNameToResult = true, bool includeFolder = false);
		std::vector<std::string> getAllLeafs();
};

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