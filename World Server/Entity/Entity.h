#pragma once

#ifndef __ROSE_ENTITY__
#define __ROSE_ENTITY__

#include "..\Structures.h"
#include "..\Map.h"

#undef max

class Monster;
class NPC;
class Player;


class EntitySpawnsVisually {
private:
	EntitySpawnsVisually();
	~EntitySpawnsVisually();
public:
	const static byte_t IS_MOVING = 0x01;
	const static byte_t IS_ATTACKING = 0x02;
	const static byte_t IS_DEAD = 0x03;
	const static byte_t IS_STANDING = 0x00;

	const static byte_t STANCE_WALKING = 0x00;
	const static byte_t STANCE_RUNNING = 0x01;
	const static byte_t STANCE_DRIVING = 0x02;
	const static byte_t STANCE_HITCHHIKER = 0x04;

	const static byte_t IS_FRIENDLY = 0x02;
	const static byte_t IS_ENEMY = 0x51;
};

struct _entityInfo {
	word_t id;
	bool ingame;
	word_t mapId;
	Map::Sector* nearestSector;		//not in use, yet
	bool needsVisualityUpdate;
	byte_t type;
	_entityInfo() {
		this->id = this->mapId = 0x00;
		this->ingame = false;
		this->needsVisualityUpdate = false;
		this->nearestSector = nullptr;
		this->type = std::numeric_limits<BYTE>::max();
	}
	__inline word_t getId() const { return this->id; }
	__inline bool isIngame() const { return this->ingame; }
	__inline word_t getMapId() const { return this->mapId; }
	__inline void setMapId(const word_t newId) { this->mapId = newId; }
	__inline Map::Sector* getSector() const { return this->nearestSector; }
	void setSector(Map::Sector* newSector) { this->nearestSector = newSector; }
	__inline byte_t getType() const { return this->type; }
};

class Entity {
	protected:
		Stats stats;
		Status status;
		struct _posStruct {
			position_t current;
			position_t destination;
			position_t source;
			clock_t lastCheckTime;
			clock_t lastSectorCheckTime;

			_posStruct() {
				lastCheckTime = clock();
				lastSectorCheckTime = clock();
			}
		} position;
		_entityInfo entityInfo;
		Combat combat;
		Animation animation;

		UniqueSortedList<DWORD, Map::Sector*> visibleSectors;
		virtual bool setPositionVisually(const position_t& pos) { return true; }

		virtual void addSectorVisually(Map::Sector* newSector);
		virtual void removeSectorVisually(Map::Sector* toRemove);
		
		virtual void addEntityVisually(Entity* entity) { }
		virtual void removeEntityVisually(Entity* entity) { if(this->getTarget() == entity) this->setTarget(nullptr); }

		virtual bool attackEnemy();
	public:
		const static byte_t TYPE_PLAYER = 0x00;
		const static byte_t TYPE_NPC = 0x01;
		const static byte_t TYPE_MONSTER = 0x02;
		const static byte_t TYPE_DROP = 0x03;

		class Movement {
			public:
				const static byte_t IDLE = 0x00;
				const static byte_t IS_MOVING = 0x01;
				const static byte_t INITIAL_ATTACK = 0x02;
				const static byte_t TARGET_REACHED = 0x04;
		};
		
		Entity();
		virtual ~Entity();

		virtual void updateAttackpower() {}
		virtual void updateAttackSpeed() {}
		virtual void updateDefense() {}
		virtual void updateMagicDefense() {}
		virtual void updateHitrate() {}
		virtual void updateMaxHP() {}
		virtual void updateDodgerate() {}
		virtual void updateCritrate() {}
		virtual void updateMovementSpeed() {}
		virtual void updateStats() {
			this->updateAttackpower();
			this->updateAttackSpeed();
			this->updateDefense();
			this->updateMagicDefense();
			this->updateHitrate();
			this->updateMaxHP();
			this->updateDodgerate();
			this->updateCritrate();
			this->updateMovementSpeed();
		}
		__inline virtual byte_t getEntityType() const { return this->entityInfo.getType(); }
		void runAnimationUpToCurrentTime();

		__inline UniqueSortedList<DWORD, Map::Sector*> getVisibleSectors() const { return this->visibleSectors; }
		bool isVisible(const word_t localId) const;
		bool isVisible(const Entity* entity) const;
		Entity* getVisibleEntity(const word_t localId) const;
		
		__inline virtual word_t getCurrentHP() const { return this->stats.getCurrentHP(); }
		__inline virtual word_t getCurrentMP() const { return this->stats.getCurrentMP(); }
		__inline virtual void setCurrentHP(dword_t newHP) { this->stats.curHP = static_cast<WORD>(newHP < this->stats.getMaxHP() ? newHP : this->stats.getMaxHP()); }
		__inline virtual void setCurrentMP(dword_t newMP) { this->stats.curMP = static_cast<WORD>(newMP < this->stats.getMaxMP() ? newMP : this->stats.getMaxMP()); }

		__inline virtual dword_t getMaxHP() const { return this->stats.getMaxHP(); }
		__inline virtual dword_t getMaxMP() const { return this->stats.getMaxMP(); }
		
		__inline virtual byte_t getPercentHP() { return static_cast<BYTE>(static_cast<float>(this->getCurrentHP() * 100.0f) / static_cast<float>(this->getMaxHP())); }		

		__inline virtual word_t getAttackPower() const { return this->stats.getAttackPower(); }
		__inline virtual word_t getAttackSpeed() const { return this->stats.getAttackSpeed(); }
		virtual float getAttackRange() const { return 100.0f; } //1meter
		__inline virtual word_t getDefensePhysical() const { return this->stats.getDefensePhysical(); }
		__inline virtual word_t getDefenseMagical() const { return this->stats.getDefenseMagical(); }
		__inline virtual word_t getHitrate() const { return this->stats.getHitrate(); }
		__inline virtual word_t getDodgerate() const { return this->stats.getDodgerate(); }
		__inline virtual word_t getMovementSpeed() const { return this->stats.getMovementSpeed(); }
		__inline virtual word_t getStamina() const { return this->stats.getStamina(); }

		__inline virtual Stance getStance() const { return this->status.getStance(); }
		__inline virtual void setStance(const byte_t newStance) { 
			this->status.setStance(newStance); 
			this->updateMovementSpeed();
		}

		__inline virtual byte_t getLevel() const { return 0; }

		__inline virtual word_t getStrength() const { return 0; }
		__inline virtual word_t getDexterity() const { return 0; }
		__inline virtual word_t getIntelligence() const { return 0; }
		__inline virtual word_t getConcentration() const { return 0; }		
		__inline virtual word_t getCharm() const { return 0; }
		__inline virtual word_t getSensibility() const { return 0; }
		template<class _Ty> _Ty getStatType(const word_t statType) {
			_Ty result = 0;
			switch (statType) {
				case StatType::ATTACK_POWER:
					result = static_cast<_Ty>(this->stats.attackPower);
				break;
				case StatType::ATTACK_SPEED:
					result = static_cast<_Ty>(this->stats.attackSpeed);
					break;
				case StatType::CRIT_RATE:
					result = static_cast<_Ty>(this->stats.critRate);
					break;
				case StatType::CURRENT_HP:
					result = static_cast<_Ty>(this->stats.curHP);
					break;
				case StatType::CURRENT_MP:
					result = static_cast<_Ty>(this->stats.curMP);
					break;
				case StatType::DEFENSE_MAGICAL:
					result = static_cast<_Ty>(this->stats.defenseMagical);
					break;
				case StatType::DEFENSE_PHYSICAL:
					result = static_cast<_Ty>(this->stats.defensePhysical);
					break;
				case StatType::DODGE_RATE:
					result = static_cast<_Ty>(this->stats.dodgeRate);
					break;
				case StatType::EXPERIENCE_RATE:
					result = 0;
					break;
				case StatType::HIT_RATE:
					result = static_cast<_Ty>(this->stats.hitRate);
					break;
				case StatType::LEVEL:
					result = static_cast<_Ty>(this->getLevel());
					break;
				case StatType::MAX_HP:
					result = static_cast<_Ty>(this->stats.maxHP);
					break;
				case StatType::MAX_MP:
					result = static_cast<_Ty>(this->stats.maxMP);
					break;
				case StatType::MOVEMENT_SPEED:
					result = static_cast<_Ty>(this->stats.movementSpeed);
					break;
			}
			if (result > 0)
				return result;
			return _Ty(this->getSpecialStatType(statType));
		}
		virtual dword_t getSpecialStatType(const word_t statType) { return 0; }

		__inline virtual float getCurrentX() const { return this->position.current.x; }
		__inline virtual float getCurrentY() const { return this->position.current.y; }

		__inline virtual float getDestinationX() const { return this->position.destination.x; }
		__inline virtual float getDestinationY() const { return this->position.destination.y; }

		__inline virtual std::string getName() const { return std::string(""); }

		__inline virtual void setLocalId(word_t newId) { this->entityInfo.id = newId; }
		__inline virtual word_t getLocalId() const { return this->entityInfo.getId(); }
		__inline virtual bool isIngame() const { return this->entityInfo.isIngame(); }

		__inline virtual word_t getMapId() const { return this->entityInfo.getMapId(); }
		__inline virtual void setMapId(const word_t newId) { this->combat.clear(); this->entityInfo.setMapId(newId); }

		__inline virtual Entity* getTarget() const { return this->combat.getTarget(); }
		virtual void setTarget(Entity* target);

		virtual bool castSkill(const word_t skillId) { return true; }

		virtual bool addDamage(Entity* enemy, const word_t amount, WORD& flag);
		virtual bool onDamageReceived(Entity* enemy, const word_t damage) { return true; };
		virtual void onTargetDead() { };
		virtual void onDeath() { };

		virtual bool isAllied( Entity* entity );
		__inline virtual bool isAllied( class NPC* npc ) { return true; }
		__inline virtual bool isAllied( class Monster* mon ) { return true; }
		__inline virtual bool isAllied( class Player* player ) { return true; }

		__inline virtual bool isPlayer() const { return this->getEntityType() == Entity::TYPE_PLAYER; }
		__inline virtual bool isNPC() const { return this->getEntityType() == Entity::TYPE_NPC; }
		__inline virtual bool isDrop() const { return this->getEntityType() == Entity::TYPE_DROP; }
		__inline virtual bool isMonster() const { return this->getEntityType() == Entity::TYPE_MONSTER; }
	
		__inline virtual dword_t getBuffStatus(const byte_t type) {	return this->status.buffs.getVisuality(type); }
		__inline virtual word_t getBuffAmount(const byte_t type) { return this->status.buffs.getStatusAmount(type); }
		__inline virtual const dword_t getBuffsVisuality(const byte_t buffType = 0x00) { return this->status.getBuffsVisuality(buffType); }
		__inline virtual bool checkBuffs() { return this->status.checkBuffs(); }

		__inline virtual position_t getPositionCurrent() const { return position_t(this->position.current); }
		__inline virtual position_t getPositionDest() const { return position_t(this->position.destination); }

		virtual void setPositionCurrent(const position_t& newPos);
		virtual void setPositionDest(const position_t& newPos);

		virtual Map::Sector* checkForNewSector();
		virtual void checkVisuality();
		virtual __inline dword_t getLastSectorCheckTime() const { return clock() - this->position.lastSectorCheckTime; }
		
		__inline Map::Sector* getSector() const { return this->entityInfo.getSector(); }
		LinkedList<Entity*>::Node* setSector(Map::Sector* newSector);

		bool playAnimation();
		byte_t movementRoutine();
		virtual bool getAttackAnimation() {
			return false;
		}
		
		bool sendToVisible(class Packet& pak, Entity* exceptThis = nullptr);
		bool sendToMap(class Packet& pak);
};

#endif //__ROSE_ENTITY__