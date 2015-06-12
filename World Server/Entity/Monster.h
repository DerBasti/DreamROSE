#pragma once

#undef min
#undef max

#ifndef __ROSE_MONSTER__
#define __ROSE_MONSTER__

#include "NPC.h"

class Monster : public NPC {
	private:
		Entity* owner;
	public:		
		Monster(const NPCData* newData, const AIP* newAi, const word_t mapId, const position_t& pos, class IFOSpawn* spawn = nullptr);
		~Monster();

		__inline bool isAllied( NPC* npc ) { return true; }
		__inline bool isAllied( Monster* mon ) { return true; }
		__inline bool isAllied( Player* player ) { return false; }

		__inline virtual std::string getName() const { 
			return (this->data ? this->data->getName() : std::string(""));
		}
		void onDeath();

		__inline Entity* getOwner() const { return this->owner; }
		__inline void setOwner(Entity* newOwner) { this->owner = newOwner; }

		__inline dword_t getMaxHP() const { return this->data->getMaxHP(); }
};

#endif //__ROSE_MONSTER__