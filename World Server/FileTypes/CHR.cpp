#include "CHR.h"
#include "D:\Programmieren\CMyFile\MyFile.h"
#include "D:\Programmieren\GlobalLogger\GlobalLogger.h"

CHR::CHR(VFS* pVFS, const char* pathInVFS) {
	this->filePath = pathInVFS;
	VFSData vfsData; pVFS->readFile(pathInVFS, vfsData);
	CMyBufferedFileReader<char> reader(vfsData.data, vfsData.data.size());
	this->loadInfos(pVFS, reader);
}

CHR::~CHR() {
	//nothing to do; nothing gets allocated.
	for (ZMO* zmo : this->animations) {
		delete zmo;
		zmo = nullptr;
	}
	this->animations.clear();
}

template<class FileType> void CHR::loadInfos(VFS* pVFS, FileType& file) {
	GlobalLogger* logger = &GlobalLogger::getLogger();
	logger->info("Starting to read NPC animation infos...\n");
	clock_t startTime = clock();
	word_t skelFileCount = file.read<WORD>();
	std::vector<std::string> skelFiles;
	for (unsigned int i = 0; i < skelFileCount; i++) {
#ifdef __ROSE_LOAD_SKELETONS__
		skelFiles.push_back(file.readStringUntilZero());
#else
		std::string result = file.readStringUntilZero();
		i = i;
#endif
	}
	word_t motionFileCount = file.read<word_t>();
	std::vector<std::string> motionFiles;
	for (unsigned int i = 0; i < motionFileCount; i++) {
		this->animations.push_back(new ZMO(pVFS, file.readStringUntilZero().c_str()));
	}
	word_t effectFileCount = file.read<word_t>();
	std::vector<std::string> effectFiles;
	for (unsigned int i = 0; i < effectFileCount; i++) {
#ifdef __ROSE_LOAD_EFFECTS__
		effectFiles.push_back(file.readStringUntilZero());
#else
		std::string result = file.readStringUntilZero();
		i = i;
#endif
	}
	word_t charCount = file.read<word_t>();
	logger->debug("NPC Amount in .CHR-File: %i\n", charCount);
	for (unsigned int i = 0; i < charCount; i++) {
		bool isCharEnabled = file.read<byte_t>() > 0;
		NPCInfos newChar(isCharEnabled);
		if (isCharEnabled) {
			newChar.id = file.read<word_t>();
			newChar.name = file.readStringUntilZero();
			word_t objCount = file.read<word_t>();
			for (unsigned int j = 0; j < objCount; j++) {
				newChar.objects.push_back(file.read<word_t>());
			}
			word_t animationCount = file.read<word_t>();
			for (unsigned int j = 0; j < animationCount; j++) {
				NPCInfos::Animation ani;
				ani.type = file.read<word_t>();
				ani.animationId = file.read<word_t>();
				newChar.animations.push_back(ani);
			}
			word_t effectCount = file.read<word_t>();
			for (unsigned int j = 0; j < effectCount; j++) {
				NPCInfos::Effect effect;
				effect.bone = file.read<word_t>();
				effect.effect = file.read<word_t>();
#ifdef __ROSE_LOAD_NPC_EFFECTS__
				newChar.effects.push_back(effect);
#endif
			}
		}
		this->chars.push_back(newChar);
	}
	logger->info("Finished reading NPC animation infos after %i ms...\n", clock() - startTime);
}

#define GET_ZMO(MOTION) NPCInfos npc = this->chars.at(npcId);\
if (!npc.isEnabled)\
	return nullptr;\
return this->animations.at(npc.animations.at(MOTION).animationId);

ZMO* CHR::getWarningMotion(const word_t npcId) const {
	GET_ZMO(CHR::WARNING_MOTION);
}

ZMO* CHR::getWalkMotion(const word_t npcId) const {
	GET_ZMO(CHR::WALK_MOTION);
}

ZMO* CHR::getAttackMotion(const word_t npcId) const {
	GET_ZMO(CHR::ATTACK_MOTION);
}

ZMO* CHR::getHitMotion(const word_t npcId) const {
	GET_ZMO(CHR::HIT_MOTION);
}

ZMO* CHR::getDieMotion(const word_t npcId) const {
	GET_ZMO(CHR::DIE_MOTION);
}

ZMO* CHR::getRunMotion(const word_t npcId) const {
	GET_ZMO(CHR::RUN_MOTION);
}

#undef GET_ZMO