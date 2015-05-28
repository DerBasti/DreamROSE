#pragma once

#ifndef __ROSE_CHR__
#define __ROSE_CHR__

#include <string>
#include <vector>
#include "..\..\Common\Definitions.h"
#include "ZMO.h"

#ifdef __ROSE_USE_VFS__
#include "VFS.h"
#endif

//Not reading those safes around 20-100ms; not really that impressive.
/*
#define __ROSE_LOAD_SKELETONS__
#define __ROSE_LOAD_EFFECTS__
#define __ROSE_LOAD_NPC_EFFECTS__
*/

class CHR {
	private:
		const static byte_t WARNING_MOTION = 0;
		const static byte_t WALK_MOTION = 1;
		const static byte_t ATTACK_MOTION = 2;
		const static byte_t HIT_MOTION = 3;
		const static byte_t DIE_MOTION = 4;
		const static byte_t RUN_MOTION = 5;
		const static byte_t STOP_MOTION = 6;
		const static byte_t SKILL_MOTION = 7;
		const static byte_t CASTING_MOTION = 8;
		struct NPCInfos {
			NPCInfos(bool isEnabled) {
				this->isEnabled = isEnabled;
			}
			bool isEnabled;
			word_t id;
			std::string name;
			std::vector<word_t> objects;
			struct Animation {
				word_t type;
				word_t animationId;
			};
			std::vector<Animation> animations;
			struct Effect {
				word_t bone;
				word_t effect;
			};
			std::vector<Effect> effects;
		};
		std::string filePath;
		std::vector<NPCInfos> chars;
		std::vector<ZMO*> animations;
		template<class FileType> void loadInfos(VFS* pVFS, FileType& file);
	public:
#ifdef __ROSE_USE_VFS__
		CHR(VFS* pVFS, const char* pathInVFS);
#endif
		~CHR();
		ZMO* getWarningMotion(const word_t npcId) const;
		ZMO* getWalkMotion(const word_t npcId) const;
		ZMO* getAttackMotion(const word_t npcId) const;
		ZMO* getHitMotion(const word_t npcId) const;
		ZMO* getDieMotion(const word_t npcId) const;
		ZMO* getRunMotion(const word_t npcId) const;
};

#endif //__ROSE_CHR__