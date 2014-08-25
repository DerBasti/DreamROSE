#include "ZMO.h"
#include "..\..\CMyFile\MyFile.h"

#ifdef __ROSE_USE_VFS__
ZMO::ZMO(VFS* pVFS, const char* filePath) {
	this->filePath = filePath;
	VFSData vfsData; pVFS->readFile(this->filePath.c_str(), vfsData);
	CMyBufferedReader reader(vfsData.data, vfsData.data.size());
	this->init(reader);
}
#else
ZMO::ZMO(const char* filePath) {
	this->filePath = filePath;

	CMyFile file(this->filePath.c_str(), "rb");
	if(file.exists())
		this->init(file);
}
#endif //__ROSE_USE_VFS__

ZMO::~ZMO() {

}

template<class FileType> void ZMO::init(FileType& file) {
	char buf[0x10] = { 0x00 };
	file.readString(8, buf);
	if (_stricmp("ZMO0002", buf))
		return;

	this->framesPerSecond = file.read<DWORD>();
	this->totalFrameCount = file.read<DWORD>();

	this->totalAnimationTime = static_cast<WORD>(static_cast<float>(this->totalFrameCount) / static_cast<float>(this->framesPerSecond) * 1000);

	//last 4 bytes are the ZMO-version type (e.g. 3ZMO)
	//The previous 10 bytes: an offset or anything of the sorts seems to be hiddin in there.
	file.setPosition(file.getTotalSize() - 14 - sizeof(WORD)*this->totalFrameCount);
#ifdef __ROSE_ZMO_OUTPUT__
	std::cout << "[" << this->filePath.c_str() << "] Total time: " << this->totalAnimationTime << "\n";
#endif
	for (unsigned int i = 0; i < this->totalFrameCount; i++) {
		WORD currentType = file.read<WORD>();
		if (currentType == MOTION_MELEE_ATTACK || currentType == MOTION_RANGED_ATTACK) { //Dunno what this means, but it seems to be a valid indicator, just like the following 2-frame following "21"
			float percentage = static_cast<float>(i - 1) / static_cast<float>(this->totalFrameCount);
			this->attackTimers.push_back(static_cast<WORD>(percentage * this->totalAnimationTime));
#ifdef __ROSE_ZMO_OUTPUT__
			std::cout << "Found attackPattern at " << percentage * 100.0f << "%\t| " << this->attackTimers.at(this->attackTimers.size() - 1) << "ms\n";
#endif
		}
	}
#ifdef __ROSE_ZMO_OUTPUT__
		std::cout << "\n";
#endif
}