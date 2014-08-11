#include "VFS.h"

VFS::VFS(const char* gameFolder) {
	this->filePath = gameFolder;

	this->triggerVFSDLL = ::LoadLibraryA((this->filePath + "TriggerVFS.dll").c_str());
	this->vfsHandle = 0x00;
	if (this->triggerVFSDLL) {
		this->OpenVFS = reinterpret_cast<OpenVFS_FUNCPTR>(GetProcAddress(this->triggerVFSDLL, "_OpenVFS@8"));
		this->CloseVFS = reinterpret_cast<CloseVFS_FUNCPTR>(GetProcAddress(this->triggerVFSDLL, "_CloseVFS@4"));
		this->GetVFSCount = reinterpret_cast<GetVFSCount_FUNCPTR>(GetProcAddress(this->triggerVFSDLL, "_VGetVfsCount@4"));
		this->GetVFSNames = reinterpret_cast<GetVFSNames_FUNCPTR>(GetProcAddress(this->triggerVFSDLL, "_VGetVfsNames@16"));
		this->GetFileCount = reinterpret_cast<GetFileCount_FUNCPTR>(GetProcAddress(this->triggerVFSDLL, "_VGetFileCount@8"));
		this->GetFileNames = reinterpret_cast<GetFileNames_FUNCPTR>(GetProcAddress(this->triggerVFSDLL, "_VGetFileNames@20"));
		this->OpenFile = reinterpret_cast<OpenFile_FUNCPTR>(GetProcAddress(this->triggerVFSDLL, "_VOpenFile@8"));
		this->RemoveFile = reinterpret_cast<RemoveFile_FUNCPTR>(GetProcAddress(this->triggerVFSDLL, "_VRemoveFile@8"));
		this->CloseFile = reinterpret_cast<CloseFile_FUNCPTR>(GetProcAddress(this->triggerVFSDLL, "_VCloseFile@4"));
		this->ReadFile = reinterpret_cast<ReadFile_FUNCPTR>(GetProcAddress(this->triggerVFSDLL, "_vfread@16"));
		this->GetFileSize = reinterpret_cast<GetFileSize_FUNCPTR>(GetProcAddress(this->triggerVFSDLL, "_vfgetsize@4"));

		this->vfsHandle = this->OpenVFS((this->filePath + "data.idx").c_str(), "r");
	}
}

VFS::~VFS() {
	if (this->triggerVFSDLL) {
		if (this->currentFileHandle) {
			this->CloseFile(this->currentFileHandle);
		}
		if (this->vfsHandle) {
			this->CloseVFS(this->vfsHandle);
		}
		FreeLibrary(this->triggerVFSDLL);
		this->triggerVFSDLL = nullptr;
	}
	this->filePath = "";
}

void VFS::readFile(const char *pathInVFS, Trackable<char>& buf) {
	char* tmpBuf = nullptr;
	DWORD len = this->readFile(pathInVFS, &tmpBuf);

	buf.init(tmpBuf, len);
}

DWORD VFS::readFile(const char *pathInVFS, char** ppBuffer) {
	if (this->vfsHandle) {
		this->currentFileHandle = this->OpenFile(pathInVFS, this->vfsHandle);
		if (this->currentFileHandle) {
			DWORD fileLen = this->GetFileSize(this->currentFileHandle);
			if (fileLen == 0)
				return 0;
			char*& pBuf = *ppBuffer;
			pBuf = new char[fileLen];

			DWORD result = this->ReadFile(pBuf, 1, fileLen, this->currentFileHandle);

			this->CloseFile(this->currentFileHandle);
			this->currentFileHandle = 0x00;

			if (result == fileLen)
				return fileLen;
			
			//In case the read result was not the wanted file length
			delete pBuf;
			pBuf = nullptr;
		}
	}
	return 0;
}


