#include "ZMO.h"
#include "..\..\CMyFile\MyFile.h"

#ifdef __ROSE_USE_VFS__
ZMO::ZMO(char* fileBuf, const DWORD fileLen) {
	CMyBufferedReader reader(fileBuf, fileLen);
	this->init(reader);
}
#else
ZMO::ZMO(const char* filePath) {
	this->filePath = filePath;

	CMyFile file(this->filePath.c_str(), "rb");
	if(file.exists())
		this->init(file);
}
#endif

ZMO::~ZMO() {

}

template<class FileType> void ZMO::init(FileType& file) {
	char buf[0x10] = { 0x00 };
	file.readString(8, buf);
	if (_stricmp("ZMO0002", buf))
		return;

	this->framesPerSecond = static_cast<WORD>(file.read<DWORD>());
	this->totalFrameCount = static_cast<WORD>(file.read<DWORD>());

	//everything else is unimportant
}