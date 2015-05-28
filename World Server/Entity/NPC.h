#pragma once

#ifndef __ROSE_NPC__
#define __ROSE_NPC__

#include "Entity.h"
#include "..\FileTypes\IFO.h"
#include "..\FileTypes\AIP.h"

extern long NPC_ATTACK_INTERVAL;

class NPC : public Entity {
	private:
		float direction;
	protected:
		const ZMO* attackAnimation;
		const AIP* ai;
		SortedList<word_t, dword_t> damageDealers;
		time_t lastAICheck;

		const static byte_t MAX_AIVAR = 0x05;
		const static byte_t MAX_OBJVAR = 0x14;
		word_t aiVar[MAX_AIVAR];
		word_t objVar[MAX_OBJVAR];

		IFOSpawn* spawn;
		const NPCData* data;
		NPC() {
			//Empty for now.
		}
		virtual void constructor(const NPCData* newData, const AIP* newAi, const word_t mapId, const position_t& pos);
		virtual bool setPositionVisually(const position_t& pos);
	public:
		NPC(const NPCData* newData, const AIP* newAi, const word_t mapId, const position_t& pos) {
			this->constructor(newData, newAi, mapId, pos);
		}
		virtual ~NPC() {
		}
		
		virtual void updateAttackpower();
		virtual void updateAttackSpeed();
		virtual void updateDefense();
		virtual void updateMagicDefense();
		virtual void updateHitrate();
		virtual void updateDodgerate();
		virtual void updateMovementSpeed();
		virtual bool convertTo(const word_t newType);

		__inline virtual position_t getSpawnPosition() const { return this->position.source; }

		__inline virtual bool isAllied( NPC* npc ) { return true; }
		__inline virtual bool isAllied( class Monster* mon ) { return true; }
		__inline virtual bool isAllied( Player* player ) { return true; }
#undef max
		__inline word_t getTypeId() const {
			return (this->data ? this->data->getTypeId() : (std::numeric_limits<word_t>::max()));
		}
		__inline virtual std::string getName() const { 
			return (this->data ? this->data->getName() : std::string(""));
		}
		__inline bool isNPC() const { return this->data->getIsNPC(); }
		__inline void addDamage(const dword_t amount) { this->stats.curHP -= static_cast<word_t>(amount > this->stats.curHP ? this->stats.curHP : amount); }
		__inline byte_t getLevel() const { return this->data->getLevel(); }
		virtual float getAttackRange() const;
		__inline float getDirection() const { return this->direction; }
		__inline void setDirection(const float newVal) { this->direction = newVal; }
		__inline bool hasDialogId() const { return (this->data->getDialogId() > 0); }
		__inline word_t getDialogId() const { return this->data->getDialogId(); }

		virtual void setStance(const byte_t newStance);

		virtual bool onDamageReceived(Entity* enemy, const word_t damage);
		virtual void addDamage(Entity* enemy, const dword_t amount) { }
		virtual bool getAttackAnimation();

		__inline virtual const IFOSpawn* getSpawn() const { return this->spawn; }

		virtual const AIP* getAI() const;
		__inline virtual const time_t getTimeAICheck() const { return this->lastAICheck; }
		__inline virtual void setTimeAICheck() { this->lastAICheck = time(nullptr); }
		
		__inline word_t getAIVar(byte_t idx) { return this->aiVar[idx % MAX_AIVAR]; }
		__inline void setAIVar(byte_t idx, int val) {
			this->aiVar[idx] = (val < 0x00) ? 0x00 : val;
			if(aiVar[idx] > 500)
				aiVar[idx] = 500;
		}
		__inline word_t getObjVar(byte_t idx) { return this->objVar[idx % MAX_OBJVAR]; }
		void setObjVar(byte_t idx, word_t val);
};


#endif //__ROSE_NPC__