#pragma once


#ifndef __ROSE_NPC__
#define __ROSE_NPC__

#include "Entity.h"
#include "..\FileTypes\IFO.h"
#include "..\FileTypes\AIP.h"

class NPC : public Entity {
	private:
		float direction;
	protected:
		const AIP* ai;
		time_t lastAICheck;

		const static BYTE MAX_AIVAR = 0x05;
		const static BYTE MAX_OBJVAR = 0x14;
		WORD aiVar[MAX_AIVAR];
		WORD objVar[MAX_OBJVAR];

		const IFOSpawn* spawn;
		const NPCData* data;
		NPC() {
			this->constructor(nullptr, nullptr, 0x01, Position(520000, 520000));
		}
		virtual void constructor(const NPCData* newData, const AIP* newAi, const WORD mapId, const Position& pos);
		virtual bool setPositionVisually(const Position& pos);
	public:
		NPC(const NPCData* newData, const AIP* newAi, const WORD mapId, const Position& pos) {	
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

		virtual void setPositionCurrent(const Position& newPos);
		virtual void setPositionDest(const Position& newPos);
		__inline virtual Position getSpawnPosition() const { return (this->spawn == nullptr ? Position(520000.f, 520000.0f) : this->spawn->getPosition()); }
		
		virtual bool isAllied( Entity* entity );
		__inline virtual bool isAllied( NPC* npc ) { return true; }
		__inline virtual bool isAllied( class Monster* mon ) { return true; }
		__inline virtual bool isAllied( Player* player ) { return true; }
#undef max
		__inline WORD getTypeId() const {
			return (this->data ? this->data->getTypeId() : (std::numeric_limits<WORD>::max()));
		}
		__inline virtual std::string getName() const { 
			return (this->data ? this->data->getName() : std::string(""));
		}
		__inline bool isNPC() const { return this->data->getIsNPC(); }
		__inline void addDamage(const DWORD amount) { this->stats.curHP -= static_cast<WORD>(amount > this->stats.curHP ? this->stats.curHP : amount); }
		__inline BYTE getLevel() const { return this->data->getLevel(); }
		__inline float getDirection() const { return this->direction; }
		__inline void setDirection(const float& newVal) { this->direction = newVal; }
		__inline bool hasDialogId() const { return (this->data->getDialogId() > 0); }
		__inline WORD getDialogId() const { return this->data->getDialogId(); }

		__inline virtual const IFOSpawn* getSpawn() const { return this->spawn; }

		virtual const AIP* getAI() const;
		__inline virtual const time_t getTimeAICheck() const { return this->lastAICheck; }
		__inline virtual void setTimeAICheck() { this->lastAICheck = time(nullptr); }
		
		__inline WORD getAIVar(BYTE idx) { return this->aiVar[idx % MAX_AIVAR]; }
		__inline void setAIVar(BYTE idx, int val) {
			this->aiVar[idx] = (static_cast<WORD>(val) < 0x00) ? 0x00 : val;
			if(aiVar[idx] > 500)
				aiVar[idx] = 500;
		}
		__inline WORD getObjVar(BYTE idx) { return this->objVar[idx % MAX_OBJVAR]; }
		void setObjVar(BYTE idx, WORD val);
};


#endif //__ROSE_NPC__