#pragma once

#ifndef __ROSE_ENTITY__
#define __ROSE_ENTITY__

#include "..\Structures.h"

class Monster;
class NPC;
class Player;


class EntitySpawnsVisually {
private:
	EntitySpawnsVisually();
	~EntitySpawnsVisually();
public:
	const static BYTE IS_MOVING = 0x01;
	const static BYTE IS_ATTACKING = 0x02;
	const static BYTE IS_DEAD = 0x03;
	const static BYTE IS_STANDING = 0x00;

	const static BYTE STANCE_WALKING = 0x00;
	const static BYTE STANCE_RUNNING = 0x01;
	const static BYTE STANCE_DRIVING = 0x02;
	const static BYTE STANCE_HITCHHIKER = 0x04;

	const static BYTE IS_FRIENDLY = 0x02;
	const static BYTE IS_ENEMY = 0x51;
};

class Entity {
	protected:
		Stats stats;
		Status status;
		struct _posStruct {
			Position current;
			Position destination;
			Position source;
			clock_t lastCheckTime;
			clock_t lastSectorCheckTime;

			_posStruct() {
				lastCheckTime = clock();
				lastSectorCheckTime = clock();
			}
		} position;
		_entityInfo entityInfo;
		Combat combat;

		UniqueSortedList<DWORD, MapSector*> visibleSectors;
		virtual bool setPositionVisually(const Position& pos) { return true; }

		virtual void addSectorVisually(MapSector* newSector);
		virtual void removeSectorVisually(MapSector* toRemove);
		
		virtual void addEntityVisually(Entity* entity) { }
		virtual void removeEntityVisually(Entity* entity) { }

		virtual bool attackEnemy();
	public:
		const static BYTE TYPE_PLAYER = 0x00;
		const static BYTE TYPE_NPC = 0x01;
		const static BYTE TYPE_MONSTER = 0x02;
		const static BYTE TYPE_DROP = 0x03;
		
		Entity();
		virtual ~Entity();

		virtual void updateAttackpower() {}
		virtual void updateAttackSpeed() {}
		virtual void updateDefense() {}
		virtual void updateMagicDefense() {}
		virtual void updateHitrate() {}
		virtual void updateDodgerate() {}
		virtual void updateCritrate() {}
		virtual void updateMovementSpeed() {}
		virtual void updateStats() {
			this->updateAttackpower();
			this->updateAttackSpeed();
			this->updateDefense();
			this->updateMagicDefense();
			this->updateHitrate();
			this->updateDodgerate();
			this->updateCritrate();
			this->updateMovementSpeed();
		}
		__inline virtual BYTE getEntityType() const { return this->entityInfo.getType(); }

		__inline UniqueSortedList<DWORD, MapSector*> getVisibleSectors() const { return this->visibleSectors; }
		
		__inline virtual WORD getCurrentHP() const { return this->stats.getCurrentHP(); }
		__inline virtual WORD getCurrentMP() const { return this->stats.getCurrentMP(); }
		__inline virtual void setCurrentHP(DWORD newHP) { this->stats.curHP = static_cast<WORD>(std::min(newHP, this->stats.getMaxHP())); }
		__inline virtual void setCurrentMP(DWORD newMP) { this->stats.curMP = static_cast<WORD>(std::min(newMP, this->stats.getMaxMP())); }

		__inline virtual DWORD getMaxHP() const { return this->stats.getMaxHP(); }
		__inline virtual DWORD getMaxMP() const { return this->stats.getMaxMP(); }
		
		__inline virtual BYTE getPercentHP() { return static_cast<BYTE>(static_cast<float>(this->getCurrentHP() * 100.0f) / static_cast<float>(this->getMaxHP())); }		

		__inline virtual WORD getAttackPower() const { return this->stats.getAttackPower(); }
		__inline virtual WORD getAttackSpeed() const { return this->stats.getAttackSpeed(); }
		virtual float getAttackRange() { return 100.0f; } //1meter
		__inline virtual WORD getDefensePhysical() const { return this->stats.getDefensePhysical(); }
		__inline virtual WORD getDefenseMagical() const { return this->stats.getDefenseMagical(); }
		__inline virtual WORD getHitrate() const { return this->stats.getHitrate(); }
		__inline virtual WORD getDodgerate() const { return this->stats.getDodgerate(); }
		__inline virtual WORD getMovementSpeed() const { return this->stats.getMovementSpeed(); }
		__inline virtual WORD getStamina() const { return this->stats.getStamina(); }

		__inline virtual Stance getStance() const { return this->status.getStance(); }
		__inline virtual void setStance(const BYTE newStance) { 
			this->status.setStance(newStance); 
			this->updateMovementSpeed();
		}

		__inline virtual BYTE getLevel() const { return 0; }

		__inline virtual WORD getStrength() const { return 0; }
		__inline virtual WORD getDexterity() const { return 0; }
		__inline virtual WORD getIntelligence() const { return 0; }
		__inline virtual WORD getConcentration() const { return 0; }		
		__inline virtual WORD getCharm() const { return 0; }
		__inline virtual WORD getSensibility() const { return 0; }

		__inline virtual float getCurrentX() const { return this->position.current.x; }
		__inline virtual float getCurrentY() const { return this->position.current.y; }

		__inline virtual float getDestinationX() const { return this->position.destination.x; }
		__inline virtual float getDestinationY() const { return this->position.destination.y; }

		__inline virtual std::string getName() const { return std::string(""); }

		__inline virtual void setClientId(WORD newId) { this->entityInfo.id = newId; }
		__inline virtual WORD getClientId() const { return this->entityInfo.getId(); }
		__inline virtual bool isIngame() const { return this->entityInfo.isIngame(); }

		__inline virtual WORD getMapId() const { return this->entityInfo.getMapId(); }
		__inline virtual void setMapId(const WORD newId) { this->entityInfo.setMapId(newId); }

		__inline virtual MapSector* getSector() const { return this->entityInfo.getSector(); }
		virtual void setSector(class MapSector* newSector);

		__inline virtual Entity* getTarget() const { return this->combat.getTarget(); }
		virtual void setTarget(Entity* target);

		virtual void addDamage(Entity* enemy, const DWORD amount) { }
		virtual void onTargetDead() { };
		virtual void onDeath() { };

		virtual bool isAllied( Entity* entity );
		__inline virtual bool isAllied( class NPC* npc ) { return true; }
		__inline virtual bool isAllied( class Monster* mon ) { return true; }
		__inline virtual bool isAllied( class Player* player ) { return true; }
	
		__inline virtual DWORD getBuffStatus(const BYTE type) {	return this->status.buffs.getVisuality(type); }
		__inline virtual const DWORD getBuffsVisuality(const BYTE buffType = 0x00) { return this->status.getBuffsVisuality(buffType); }
		__inline virtual bool checkBuffs() { return this->status.checkBuffs(); }

		__inline virtual Position getPositionCurrent() const { return Position(this->position.current); }
		__inline virtual Position getPositionDest() const { return Position(this->position.destination); }

		virtual void setPositionCurrent(const Position& newPos);
		virtual void setPositionDest(const Position& newPos);

		virtual MapSector* checkForNewSector();
		virtual void checkVisuality();
		virtual __inline DWORD getLastSectorCheckTime() const { return clock() - this->position.lastSectorCheckTime; }

		bool movementRoutine();
		bool attackRoutine();
		__inline clock_t intervalBetweenAttacks() { return 100000 / this->getAttackSpeed(); }

		bool sendToVisible(class Packet& pak);
		bool sendToMap(class Packet& pak);
};

#endif //__ROSE_ENTITY__