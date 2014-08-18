#include "CHR.h"
#include "..\..\CMyFile\MyFile.h"

CHR::CHR(VFS* pVFS, const char* pathInVFS) {
	this->filePath = pathInVFS;
	VFSData data; pVFS->readFile(pathInVFS, data);
	CMyBufferedReader reader(data.data, data.data.size());
	this->loadInfos(pVFS, reader);
}

CHR::~CHR() {
	//nothing to do; nothing gets allocated.
}

template<class FileType> void CHR::loadInfos(VFS* pVFS, FileType& file) {
	std::cout << "Starting to read NPC animation infos...\r";
	clock_t startTime = clock();
	WORD skelFileCount = file.read<WORD>();
	std::vector<std::string> skelFiles;
	for (unsigned int i = 0; i < skelFileCount; i++) {
#ifdef __ROSE_LOAD_SKELETONS__
		skelFiles.push_back(file.readStringZero());
#else
		file.readStringZero();
#endif
	}
	WORD motionFileCount = file.read<WORD>();
	std::vector<std::string> motionFiles;
	for (unsigned int i = 0; i < motionFileCount; i++) {
		this->animations.push_back(ZMO(pVFS, file.readStringZero().c_str()));
	}
	WORD effectFileCount = file.read<WORD>();
	std::vector<std::string> effectFiles;
	for (unsigned int i = 0; i < effectFileCount; i++) {
#ifdef __ROSE_LOAD_EFFECTS__
		effectFiles.push_back(file.readStringZero());
#else
		file.readStringZero();
#endif
	}
	WORD charCount = file.read<WORD>();
	for (unsigned int i = 0; i < charCount; i++) {
		bool isCharEnabled = file.read<BYTE>() > 0;
		NPCInfos newChar(isCharEnabled);
		if (isCharEnabled) {
			newChar.id = file.read<WORD>();
			newChar.name = file.readStringZero();
			WORD objCount = file.read<WORD>();
			for (unsigned int j = 0; j < objCount; j++) {
				newChar.objects.push_back(file.read<WORD>());
			}
			WORD animationCount = file.read<WORD>();
			for (unsigned int j = 0; j < animationCount; j++) {
				NPCInfos::Animation ani;
				ani.type = file.read<WORD>();
				ani.animationId = file.read<WORD>();
				newChar.animations.push_back(ani);
			}
			WORD effectCount = file.read<WORD>();
			for (unsigned int j = 0; j < effectCount; j++) {
				NPCInfos::Effect effect;
				effect.bone = file.read<WORD>();
				effect.effect = file.read<WORD>();
#ifdef __ROSE_LOAD_NPC_EFFECTS__
				newChar.effects.push_back(effect);
#endif
			}
		}
		this->chars.push_back(newChar);
	}
	std::cout << "Finished reading NPC animation infos after " << clock() - startTime << "ms\n";
}

#define GET_ZMO(MOTION) NPCInfos npc = this->chars.at(npcId);\
if (!npc.isEnabled)\
	return nullptr;\
return &this->animations.at(npc.animations.at(MOTION).animationId);

ZMO* CHR::getWarningMotion(const WORD npcId) {
	GET_ZMO(CHR::WARNING_MOTION);
}

ZMO* CHR::getWalkMotion(const WORD npcId) {
	GET_ZMO(CHR::WALK_MOTION);
}

ZMO* CHR::getAttackMotion(const WORD npcId) {
	GET_ZMO(CHR::ATTACK_MOTION);
}

ZMO* CHR::getHitMotion(const WORD npcId) {
	GET_ZMO(CHR::HIT_MOTION);
}

ZMO* CHR::getDieMotion(const WORD npcId) {
	GET_ZMO(CHR::DIE_MOTION);
}

ZMO* CHR::getRunMotion(const WORD npcId) {
	GET_ZMO(CHR::RUN_MOTION);
}

#undef GET_ZMO