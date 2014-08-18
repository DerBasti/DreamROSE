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
		const static BYTE WARNING_MOTION = 0;
		const static BYTE WALK_MOTION = 1;
		const static BYTE ATTACK_MOTION = 2;
		const static BYTE HIT_MOTION = 3;
		const static BYTE DIE_MOTION = 4;
		const static BYTE RUN_MOTION = 5;
		const static BYTE STOP_MOTION = 6;
		const static BYTE SKILL_MOTION = 7;
		const static BYTE CASTING_MOTION = 8;
		struct NPCInfos {
			NPCInfos(bool isEnabled) {
				this->isEnabled = isEnabled;
			}
			bool isEnabled;
			WORD id;
			std::string name;
			std::vector<WORD> objects;
			struct Animation {
				WORD type;
				WORD animationId;
			};
			std::vector<Animation> animations;
			struct Effect {
				WORD bone;
				WORD effect;
			};
			std::vector<Effect> effects;
		};
		std::string filePath;
		std::vector<NPCInfos> chars;
		std::vector<ZMO> animations;
		template<class FileType> void loadInfos(VFS* pVFS, FileType& file);
	public:
#ifdef __ROSE_USE_VFS__
		CHR(VFS* pVFS, const char* pathInVFS);
#endif
		~CHR();
		ZMO* getWarningMotion(const WORD npcId);
		ZMO* getWalkMotion(const WORD npcId);
		ZMO* getAttackMotion(const WORD npcId);
		ZMO* getHitMotion(const WORD npcId);
		ZMO* getDieMotion(const WORD npcId);
		ZMO* getRunMotion(const WORD npcId);
};

#endif //__ROSE_CHR__