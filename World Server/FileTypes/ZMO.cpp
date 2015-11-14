#include "ZMO.h"
#include "D:\Programmieren\CMyFile\MyFile.h"

#ifdef __ROSE_USE_VFS__
ZMO::ZMO(VFS* pVFS, const char* filePath) {
	this->filePath = filePath;
	VFSData vfsData; 
	pVFS->readFile(this->filePath.c_str(), vfsData);
	if (vfsData.data.size() > 0) {
		CMyBufferedFileReader<char> reader(vfsData.data, vfsData.data.size());
		this->init(reader);
	}
}
#else
ZMO::ZMO(const char* filePath) {
	this->filePath = filePath;

	CMyFileReader<char> file(this->filePath.c_str(), "rb");
	if(file.exists())
		this->init(file);
}
#endif //__ROSE_USE_VFS__

ZMO::~ZMO() {

}

template<class FileType> void ZMO::init(FileType& file) {
	char buf[0x10] = { 0x00 };
	file.readStringWithGivenLength(8, buf);
	if (_stricmp("ZMO0002", buf))
		return;

	this->animationInfo.framesPerSecond = static_cast<word_t>(file.read<dword_t>());
	this->animationInfo.framesInTotal = static_cast<word_t>(file.read<dword_t>())+1;

	this->animationInfo.defaultTime = static_cast<word_t>(static_cast<float>(this->animationInfo.framesInTotal) / static_cast<float>(this->animationInfo.framesPerSecond) * 1000);

	//last 4 bytes are the ZMO-version type (e.g. 3ZMO)
	//The previous 10 bytes: an offset or anything of the sorts seems to be hiddin in there.
	file.setPosition(file.getTotalSize() - 14 - sizeof(word_t)*this->animationInfo.framesInTotal);
#ifdef __ROSE_ZMO_OUTPUT__
	std::cout << "[" << this->filePath.c_str() << "] Total time: " << this->animationInfo.defaultTime << "\n";
#endif
	word_t *newContent = new word_t[this->animationInfo.framesInTotal];
	for (unsigned int i = 0; i < this->animationInfo.framesInTotal; i++) {
		word_t currentType = file.read<word_t>();
		newContent[i] = currentType;
		if (i > 0 && (currentType >= 21 && currentType <= 29)) { //Dunno what this means, but it seems to be a valid indicator?

			/* current frame - 3, because the server may need up to 100ms to perform the actual action. 
				(3 * (1000ms / 30FPS) = 100ms -> the offset we need. 
			*/
			float percentage = static_cast<float>(i-3) / static_cast<float>(this->animationInfo.framesInTotal);
			ZMO::TimingInfo::Frame newFrame;

			dword_t storedPosition = file.getPosition();
			file.setPosition(storedPosition - sizeof(word_t)*3);
			newFrame.motionType = file.read<word_t>();
			file.setPosition(storedPosition);

			newFrame.timeToReach = static_cast<word_t>(percentage * this->animationInfo.defaultTime);
			this->timingInfo.attacksAtFrames.insert(std::pair<word_t, ZMO::TimingInfo::Frame>(i - 3, newFrame));
#ifdef __ROSE_ZMO_OUTPUT__
			auto it = this->timingInfo.attacksAtFrames.at(i - 3);
			std::cout << "Found attackPattern at " << percentage * 100.0f << "%\t| " << it.timeToReach << "ms\n";
#endif
		}
	}
	this->content.init(newContent, this->animationInfo.framesInTotal);

	delete[] newContent;
	newContent = nullptr;

	this->animationInfo.framesInTotal--;
#ifdef __ROSE_ZMO_OUTPUT__
		std::cout << "\n";
#endif
}