#include <Windows.h>
#include <iostream>
#include <algorithm>
#include "VFS.h"
#include "D:\Programmieren\QuickInfos\QuickInfo.h"
#include "D:\Programmieren\GlobalLogger\GlobalLogger.h"

std::vector<std::string> VFSTree::getFilesInPath(std::string path, const char* contentFilter, bool addDirectoryNameToResult, bool includeFolder) {
	const VFSTree::Branch* branch = this->root->findBranchByPath(path); 
	if (branch) {
		std::vector<std::string> result = branch->getAllNextLowerBranchNames(addDirectoryNameToResult, includeFolder);
		if (contentFilter == nullptr || contentFilter[0] == 0x00)
			return result;
		for (dword_t i = 0; i < result.size(); i++) {
			//make sure it's only a copy
			std::string currentFile = result.at(i);
			const char* currentFileExtension = &currentFile.c_str()[currentFile.find_last_of(".")];
			if (!QuickInfo::substringExists(currentFile.c_str(), contentFilter)) {
				result.erase(result.begin() + i);
				i--;
			}
		}
		return result;
	}
	return std::vector<std::string>();
}

std::vector<std::string> VFSTree::Branch::getAllChildren() {
	VFSTree::Branch* branch = this;
	std::vector<std::string> result;
	while (branch != nullptr) {
		if (branch->hasChildren()) {
			std::vector<std::string> childResult = branch->getAllChildren();
			for (dword_t i = 0; i < childResult.size(); i++)
				result.push_back(childResult.at(i));
			childResult.clear();
		}
		else {
			result.push_back(branch->fullPath);
			branch = branch->nextBranch;
		}
	}
	return result;
}

std::vector<std::string> VFSTree::getAllLeafs() {
	return this->root->getAllChildren();
}

VFSTree::Branch* VFSTree::Branch::_getBranchByName(const char* branch) const {
	std::string branchName = std::string(branch);
	std::transform(branchName.begin(), branchName.end(), branchName.begin(), ::toupper);
	dword_t generatedHash = std::hash<std::string>()(branchName);
	if (this->lowerLevel.end != nullptr) {
		Branch* currentLeaf = this->lowerLevel.first;
		while (currentLeaf != nullptr) {
			if (currentLeaf->hash == generatedHash) {
				return currentLeaf;
			}
			currentLeaf = currentLeaf->nextBranch;
		}
	}
	return nullptr;
}

VFSTree::Branch::Branch(std::string nodeName, Branch* parentBranch) {

	this->name.addListener(ChangeListener([&](EventObject& e) {
		Observable<std::string>* s = reinterpret_cast<Observable<std::string>*>(e.source);
		dword_t prevHash = this->hash;
		this->hash = std::hash<std::string>()(s->get());
		//GlobalLogger::debug("Hash of %s was changed from 0x%x to 0x%x", s->get().c_str(), prevHash, this->hash);
	}));

	this->name = nodeName.find("\\") != -1 ? nodeName.substr(0, nodeName.find("\\")) : nodeName;
	this->lowerLevel.first = this->lowerLevel.end = nullptr;
	this->parent = parentBranch;
	this->nextBranch = nullptr;

	this->fullPath = this->name.get();
	if (this->parent) {
		this->fullPath = this->parent->fullPath + std::string("\\") + this->name.get();
	}
}

bool VFSTree::Branch::operator==(const Branch& branch) const {
	if (&branch == this)
		return true;
	return false;
}
bool VFSTree::Branch::operator!=(const Branch& branch) const {
	return !(this->operator==(branch));
}

VFSTree::Branch* VFSTree::Branch::appendLeaf(std::string value) {
	if (this->lowerLevel.first == nullptr) {
		this->name = this->name.get() + std::string("\\");
		this->lowerLevel.first = new Branch(value, this);
		this->lowerLevel.end = this->lowerLevel.first;
		return this->lowerLevel.first;
	}
	//Check whether a filename like this exists.
	Branch* existingBranch = this->_getBranchByName(value.c_str());
	if (existingBranch != nullptr) {
		return existingBranch;
	}

	Branch* lastBranch = this->lowerLevel.end;
	this->lowerLevel.end = new Branch(value, this);
	lastBranch->nextBranch = this->lowerLevel.end;

	return this->lowerLevel.end;
}

const VFSTree::Branch* VFSTree::Branch::operator[](const char* branchName) const {
	if (this->lowerLevel.end != nullptr) {
		Branch* currentLeaf = this->lowerLevel.first;
		while (currentLeaf != nullptr) {
			if (_stricmp(currentLeaf->getName().c_str(), branchName) == 0)
				return currentLeaf;
			currentLeaf = currentLeaf->nextBranch;
		}
	}
	return nullptr;
}

const VFSTree::Branch* VFSTree::Branch::findBranchByPath(std::string path) {
	if (path.at(path.length() - 1) != '\\') {
		path += std::string("\\");
	}
	dword_t pathDepth = (path.at(0) == '\\' ? -1 : 0);
	dword_t lastPos = path.find("\\"); // root
	do {
		pathDepth = lastPos == -1 ? pathDepth : pathDepth + 1;
		lastPos = path.find("\\", lastPos + 1);
	} while (lastPos != -1);

	lastPos = 0;
	Branch* branch = this;
	for (dword_t i = 0; branch != nullptr && i < pathDepth; i++) {
		dword_t substringOffset = path.find("\\", lastPos);
		std::string currentBranchName = path.substr(lastPos, substringOffset-lastPos + 1);
		branch = branch->_getBranchByName(currentBranchName.c_str());
		lastPos = substringOffset + 1;
	}
	return branch;
}

std::vector<std::string> VFSTree::Branch::getAllNextLowerBranchNames(bool addDirectoryNameToResult, bool includeFolder) const {
	std::vector<std::string> result;
	if (this->lowerLevel.end != nullptr) {
		Branch* currentLeaf = this->lowerLevel.first;
		while (currentLeaf != nullptr) {
			if (!includeFolder && currentLeaf->hasChildren()) {
				currentLeaf = currentLeaf->nextBranch;
				continue;
			}
			addDirectoryNameToResult ? result.push_back(currentLeaf->fullPath) : result.push_back(currentLeaf->name.get());
			currentLeaf = currentLeaf->nextBranch;
		}
		//addDirectoryNameToResult ? result.push_back(this->lowerLevel.end->fullPath) : result.push_back(this->lowerLevel.end->name);
	}
	return result;
}

void VFSTree::Branch::establishPath(const char *str) {
	std::string path = std::string(str);
	if (path.length() > 0) {
		if (path.find("\\") != -1) {
			//Remove a prepending slash
			if (path.find("\\") == 0) {
				path = path.substr(1);
			}
			Branch* branchPath = this->_getBranchByName(path.substr(0, path.find("\\") + 1).c_str());
			if (branchPath != nullptr) {
				return branchPath->establishPath(path.substr(path.find("\\") + 1));
			}
			else {
				Branch* newBranch = this->appendLeaf(path.substr(0, path.find("\\")));
				path = path.substr(path.find("\\") + 1);
				return newBranch->establishPath(path);
			}
		}
		else {
			this->appendLeaf(path);
		}
	}
}

VFS::VFS(const char* gameFolder) {
	this->filePath = gameFolder;

	this->triggerVFSDLL = (void*)::LoadLibraryA((this->filePath + "TriggerVFS.dll").c_str());
	this->vfsHandle = 0x00;
	if (this->triggerVFSDLL) {
		HMODULE triggerVFS = reinterpret_cast<HMODULE>(this->triggerVFSDLL);
		this->OpenVFS = reinterpret_cast<OpenVFS_FUNCPTR>(GetProcAddress(triggerVFS, "_OpenVFS@8"));
		this->CloseVFS = reinterpret_cast<CloseVFS_FUNCPTR>(GetProcAddress(triggerVFS, "_CloseVFS@4"));

		this->GetVFSCount = reinterpret_cast<GetVFSCount_FUNCPTR>(GetProcAddress(triggerVFS, "_VGetVfsCount@4"));
		this->GetVFSNames = reinterpret_cast<GetVFSNames_FUNCPTR>(GetProcAddress(triggerVFS, "_VGetVfsNames@16"));

		this->GetFileCount = reinterpret_cast<GetFileCount_FUNCPTR>(GetProcAddress(triggerVFS, "_VGetFileCount@8"));
		this->GetFileNames = reinterpret_cast<GetFileNames_FUNCPTR>(GetProcAddress(triggerVFS, "_VGetFileNames@20"));
		this->OpenFile = reinterpret_cast<OpenFile_FUNCPTR>(GetProcAddress(triggerVFS, "_VOpenFile@8"));
		//this->RemoveFile = reinterpret_cast<RemoveFile_FUNCPTR>(GetProcAddress(triggerVFS, "_VRemoveFile@8"));
		this->CloseFile = reinterpret_cast<CloseFile_FUNCPTR>(GetProcAddress(triggerVFS, "_VCloseFile@4"));
		this->ReadFile = reinterpret_cast<ReadFile_FUNCPTR>(GetProcAddress(triggerVFS, "_vfread@16"));
		this->GetFileSize = reinterpret_cast<GetFileSize_FUNCPTR>(GetProcAddress(triggerVFS, "_vfgetsize@4"));
		
		this->vfsHandle = this->OpenVFS((this->filePath + "data.idx").c_str(), "r");

		dword_t vfsCount = this->GetVFSCount(this->vfsHandle) - 1;
		char** tmpVFSNames = new char*[vfsCount];
		for (dword_t i = 0; i < vfsCount; i++)
			tmpVFSNames[i] = new char[0x100]; //256
		this->GetVFSNames(this->vfsHandle, tmpVFSNames, vfsCount, 0x100);

		std::vector<std::string> vfsNames;
		for (dword_t i = 0; i < vfsCount; i++) {
			vfsNames.push_back(std::string(tmpVFSNames[i]));
			delete[] tmpVFSNames[i];
			tmpVFSNames[i] = nullptr;
		}
		delete[] tmpVFSNames;
		tmpVFSNames = nullptr;

		std::string serverPath = "";  QuickInfo::getPath(&serverPath);
		CMyFileReader<char> reader((serverPath + "\\vfsIndex.idx").c_str(), true);
		if (!this->isLastIndexUpTodate(vfsNames, &reader)) {
			this->readFilePathsFromVFS(vfsNames);
		}
		else {
			this->readFilePathsFromLog(&reader);
		}
	}
}

VFS::~VFS() {
	this->close();
}

void VFS::close() {
	if (this->triggerVFSDLL) {
		if (this->fileIndex) {
			delete this->fileIndex;
			this->fileIndex = nullptr;
		}
		if (this->currentFileHandle) {
			this->CloseFile(this->currentFileHandle);
		}
		if (this->vfsHandle) {
			this->CloseVFS(this->vfsHandle);
		}
		FreeLibrary(reinterpret_cast<HMODULE>(this->triggerVFSDLL));
		this->triggerVFSDLL = nullptr;
	}
	this->filePath = "";
}

dword_t VFS::getFileSize(const char *pathInVFS) {
	if (this->vfsHandle) {
		this->currentFileHandle = this->OpenFile(pathInVFS, this->vfsHandle);
		if (this->currentFileHandle) {
			dword_t fileLen = this->GetFileSize(this->currentFileHandle);
			this->closeCurrentFile();
			return fileLen;
		}
	}
	return -1;
}

void VFS::closeCurrentFile() {
	if (this->currentFileHandle) {
		this->CloseFile(this->currentFileHandle);
		this->currentFileHandle = 0x00;
	}
}

void VFS::readFilePathsFromVFS(std::vector<std::string> vfsNames) {
	GlobalLogger::info("Indexing all VFS files.");
	this->fileIndex = new VFSTree();

	std::vector<std::string> allFiles;
	dword_t fileCount = 0;
	for (dword_t i = 0; i < vfsNames.size(); i++) {
		dword_t curFileCount = this->GetFileCount(this->vfsHandle, vfsNames[i].c_str());
		fileCount += curFileCount;
		GlobalLogger::debug("Filecount of %s: %i", vfsNames[i].c_str(), curFileCount);
	}
	allFiles.reserve(fileCount);
	for (dword_t i = 0; i < vfsNames.size(); i++) {
		fileCount = this->GetFileCount(this->vfsHandle, vfsNames[i].c_str());
		char** file = new char*[fileCount];

		for (dword_t j = 0; j < fileCount; j++) {
			file[j] = new char[0x100];
		}
		this->GetFileNames(this->vfsHandle, vfsNames[i].c_str(), file, fileCount, 0x100);

		GlobalLogger::debug("[%s] Indexed 0 out of %i files\r", vfsNames[i].c_str(), fileCount);
		allFiles.reserve(allFiles.size() + fileCount);
		for (dword_t j = 0; j < fileCount; j++) {
			allFiles.push_back(file[j]);
			this->fileIndex->appendFile(file[j]);
			delete file[j];
			file[j] = nullptr;
		}
		delete[] file;
		file = nullptr;
		GlobalLogger::debug("Indexing of %s finished!                     ", vfsNames[i].c_str());
	}

	std::string serverPath = ""; QuickInfo::getPath(&serverPath);
	GlobalLogger::info("Indexing finished. Establishing an index-file [%s] for later use.", (serverPath + "\\vfsIndex.idx").c_str());
	CMyFileWriter<char> writer((serverPath + "\\vfsIndex.idx").c_str(), true);
	writer.clear();

	byte_t vfsCount = vfsNames.size();
	writer.writePlain(&vfsCount, sizeof(byte_t), 1);
	for (dword_t i = 0; i < vfsNames.size(); i++) {
		dword_t lastChangeTime = QuickInfo::getFileLastChangeTime((this->filePath + "\\" + vfsNames.at(i)).c_str());
		writer.writePlain(&lastChangeTime, sizeof(dword_t), 1);
	}
	dword_t amountOfFiles = allFiles.size();
	writer.writePlain(&amountOfFiles, sizeof(dword_t), 1);
	for (dword_t i = 0; i < allFiles.size(); i++) {
		writer.putStringWithVar("%s\n", allFiles.at(i).c_str());
	}
	GlobalLogger::info("Index-file fully saved.");
}

void VFS::readFilePathsFromLog(CMyFileReader<char>* reader) {
	if (!reader || !reader->exists())
		throw TraceableException("FileReader invalid!");
	GlobalLogger::info("Reading VFS-Index file...");
	dword_t fileAmount = reader->read<dword_t>();
	this->fileIndex = new VFSTree();
	GlobalLogger::debug("A total of %i files are indexed...", fileAmount);

	for (dword_t i = 0; i < fileAmount; i++) {
		std::string file = reader->readLine();
		this->fileIndex->appendFile(file);
	}
	GlobalLogger::info("Finished indexing the VFS-Contents!                          ");
}

bool VFS::isLastIndexUpTodate(std::vector<std::string> vfsNames, CMyFileReader<char>* reader) {
	if (!reader->exists())
		return false;

	byte_t vfsDatesCount = reader->read<byte_t>();
	if (vfsNames.size() != vfsDatesCount) {
		GlobalLogger::debug("The amount of currently existing VFS' are different from previous starts.\nSwitching over to Refresh-Mode.");
		return false;
	}
	dword_t* vfsDates = new dword_t[vfsDatesCount];

	bool success = true;
	for (dword_t i = 0; i < vfsDatesCount; i++) {
		vfsDates[i] = reader->read<dword_t>();
		if (vfsDates[i] != QuickInfo::getFileLastChangeTime((this->filePath + "\\" + vfsNames.at(i)).c_str())) {
			GlobalLogger::debug("The timestamp of the last file change was refreshed.\nSwitching over to Refresh-Mode.");
			success = false;
			break;
		}
	}

	delete[] vfsDates;
	vfsDates = nullptr;

	return success;
}

std::vector<std::string> VFS::getFileNamesFromFolder(const char* pathInVFS, const char* contentFilter, bool addDirectoryNameToResult, bool includeFolder) {
	if (this->fileIndex) {
		return this->fileIndex->getFilesInPath(pathInVFS, contentFilter, addDirectoryNameToResult, includeFolder);
	}
	return std::vector<std::string>();
}

void VFS::readFile(const char *pathInVFS, VFSData& buf) {
	if (!pathInVFS)
		return;
	if (pathInVFS[0] == '\\')
		pathInVFS++;
	char* tmpBuf = nullptr;
	dword_t len = this->readFile(pathInVFS, &tmpBuf);

	buf.filePath = pathInVFS;
	buf.data
}

dword_t VFS::readFile(const char *pathInVFS, char** ppBuffer) {
	if (this->vfsHandle) {
		this->currentFileHandle = this->OpenFile(pathInVFS, this->vfsHandle);
		if (this->currentFileHandle) {
			dword_t fileLen = this->GetFileSize(this->currentFileHandle);
			if (fileLen == 0)
				return 0;
			char*& pBuf = *ppBuffer;
			pBuf = new char[fileLen];

			dword_t result = this->ReadFile(pBuf, 1, fileLen, this->currentFileHandle);

			this->closeCurrentFile();

			if (result == fileLen)
				return fileLen;
			
			//In case the read result was not the wanted file length
			delete pBuf;
			pBuf = nullptr;
		}
	}
	return 0;
}


