#pragma once

#ifndef __ROSE_STRUCTURES__
#define __ROSE_STRUCTURES__

#include "FileTypes\STB.h"
#include "Buffs.h"
#include <algorithm>

struct Stats {
	WORD curHP;
	WORD curMP;
	DWORD maxHP;
	DWORD maxMP;

	WORD attackPower;
	WORD attackSpeed;
	clock_t attackDelay;
	WORD defensePhysical;
	WORD defenseMagical;
	WORD movementSpeed;
	WORD hitRate;
	WORD dodgeRate;
	WORD critRate;

	WORD stamina;

	Stats() {
		this->maxHP = this->maxMP = 100;
		this->curHP = this->curMP = 100;
		this->attackPower = this->attackSpeed = this->defenseMagical = this->defensePhysical = this->stamina = 0x00;
	}

	__inline WORD getCurrentHP() const { return this->curHP; }
	__inline WORD getCurrentMP() const { return this->curMP; }

	__inline DWORD getMaxHP() const { return this->maxHP; }
	__inline DWORD getMaxMP() const { return this->maxMP; }

	__inline WORD getAttackPower() const { return this->attackPower; }
	__inline WORD getAttackSpeed() const { return this->attackSpeed; }
	__inline WORD getDefensePhysical() const { return this->defensePhysical; }
	__inline WORD getDefenseMagical() const { return this->defenseMagical; }
	__inline WORD getMovementSpeed() const { return this->movementSpeed; }
	__inline WORD getHitrate() const { return this->hitRate; }
	__inline WORD getDodgerate() const { return this->dodgeRate; }
	__inline WORD getCritrate() const { return this->critRate; }

	__inline WORD getStamina() const { return this->stamina; }	
};

class Stance {
	private:
		BYTE type;
	public:
		const static BYTE NPC_WALKING = 0x00;
		const static BYTE NPC_RUNNING = 0x01;

		const static BYTE SITTING = 0x01;
		const static BYTE WALKING = 0x02;
		const static BYTE RUNNING = 0x03;
		const static BYTE DRIVING = 0x04;
		Stance() {
			type = Stance::RUNNING;
		}
		Stance& operator=(const BYTE& num) {
			this->type = num;
			return (*this);
		}
		Stance& operator=(const int& num) {
			this->type = static_cast<BYTE>(num);
			return (*this);
		}
		const BYTE asBYTE() const { return this->type; }
};

struct Status {
	time_t lastRegenCheck;
	Stance stance;
	Buffs buffs;

	const static BYTE DEFAULT_CHECK_TIME = 0x08;
	__inline bool addBuff(const BYTE visualityBit, const WORD amount, const DWORD timeInMilliseconds) {
		return this->buffs.addBuff(visualityBit, amount, timeInMilliseconds);
	}
	const DWORD getBuffsVisuality(BYTE buffType) { return this->buffs.getVisuality(buffType); }
	__inline bool checkBuffs() { return this->buffs.checkBuffs(); }
	__inline Stance getStance() const { return stance; }
	__inline void setStance( const Stance& newStance ) { stance = newStance; }
	__inline void setStance( const BYTE& newStance ) { stance = newStance; }

	__inline void updateLastRegen() { this->lastRegenCheck = time(NULL); }
};
 
struct Position {
	float x;
	float y;

	Position() { x = y = 520000.0f; }
	explicit Position(const float _x, const float _y) {
		this->x = _x;
		this->y = _y;
	}

	explicit Position(const DWORD _x, const DWORD _y) {
		this->x = static_cast<float>(_x);
		this->y = static_cast<float>(_y);
	}

	Position(const Position& rhs) {
		this->x = rhs.x;
		this->y = rhs.y;
	}
	Position& operator=(const Position& rhs) {
		this->x = rhs.x;
		this->y = rhs.y;
		return (*this);
	}

	bool operator==(const Position& rhs) {
		if (this->x == rhs.x && this->y == rhs.y)
			return true;
		return false;
	}
	bool operator!=(const Position& rhs) {
		return !(this->operator==(rhs));
	}

	__inline float distanceTo(const Position& rhs) {
		return this->distanceTo(rhs.x, rhs.y);
	}

	float distanceTo(const float _x, const float _y) {
		float xDist = (this->x - _x);
		float yDist = (this->y - _y);

		return static_cast<float>(sqrt((xDist * xDist) + (yDist * yDist)));
	}

	Position calcNewPositionWithinRadius(const float radius) {
		//Get a random value and divide it by the diameter. In order to get a relation to the center, subtract it by the actual radius
#define RADIUS_CALCULATION ((rand() / static_cast<const DWORD>(radius)) * 2) - radius
		float preComma[2] = { RADIUS_CALCULATION, RADIUS_CALCULATION };
		float afterComma[2] = { static_cast<float>(rand() / static_cast<float>(RAND_MAX)), static_cast<float>(rand() / static_cast<float>(RAND_MAX)) };
#undef RADIUS_CALCULATION
		return Position(preComma[0] + afterComma[0], preComma[1] + afterComma[1]);
	}

	__inline Position copy() {
		return Position(*this);
	}
};

struct _entityInfo {
	WORD id;
	bool ingame;
	WORD mapId;
	class MapSector* nearestSector;
	BYTE type;
	_entityInfo() {
		this->id = this->mapId = 0x00;
		this->ingame = false;
		this->nearestSector = nullptr;
		this->type = std::numeric_limits<BYTE>::max();
	}
	__inline WORD getId() const { return this->id; }
	__inline bool isIngame() const { return this->ingame; }
	__inline WORD getMapId() const { return this->mapId; }
	__inline void setMapId(const WORD newId) { this->mapId = newId; }
	__inline class MapSector* getSector() const { return this->nearestSector; }
	void setSector(class MapSector* newSector) { this->nearestSector = newSector; }
	__inline BYTE getType() const { return this->type; }
};


class ItemType {
	private:
		ItemType() { };
		~ItemType() { };
	public:
		const static WORD FACE = 1;
		const static WORD HEADGEAR = 2;
		const static WORD ARMOR = 3;
		const static WORD GLOVES = 4;
		const static WORD SHOES = 5;
		const static WORD BACK = 6;
		const static WORD JEWELRY = 7;
		const static WORD WEAPON = 8;
		const static WORD SHIELD = 9;
		const static WORD CONSUMABLES = 10;
		const static WORD JEWELS = 11;
		const static WORD OTHER = 12;
		const static WORD QUEST = 13;
		const static WORD PAT = 14;
		const static WORD MONEY = 31; //6 Bits
};

class WeaponType {
	private:
		WeaponType() {}
		~WeaponType() {}
	public:
		const static WORD MELEE_ONE_HANDED_SWORD = 211;
		const static WORD MELEE_ONE_HANDED_BLUNT = 212;
		const static WORD MELEE_TWO_HANDED_SWORD = 221;
		const static WORD MELEE_TWO_HANDED_SPEAR = 222;
		const static WORD MELEE_TWO_HANDED_AXE = 223;
		const static WORD RANGE_BOW = 231;
		const static WORD RANGE_GUN = 232;
		const static WORD RANGE_LAUNCHER = 233;
		const static WORD MAGIC_WAND = 241;
		const static WORD MAGIC_STAFF = 242;
		const static WORD MELEE_KATAR = 251;
		const static WORD MELEE_DOUBLE_SWORD = 252;
		const static WORD RANGE_DUAL_GUN = 253;
		const static WORD RANGE_CROSSBOW = 271;
};

class JobType {
	private:
		JobType() {}
		~JobType() {}
	public:
		const static WORD VISITOR = 0;
		const static WORD SOLDIER = 111;
		const static WORD KNIGHT = 121;
		const static WORD CHAMPION = 122;

		const static WORD MUSE = 211;
		const static WORD MAGE = 221;
		const static WORD CLERIC = 222;

		const static WORD HAWKER = 311;
		const static WORD RAIDER = 321;
		const static WORD SCOUT = 322;

		const static WORD DEALER = 411;
		const static WORD BOURGEOIS = 421;
		const static WORD ARTISAN = 422;
};

class OperationService {
	private:
		OperationService() { }
		~OperationService() { }
	public:
		template<class _Ty1, class _Ty2> static bool checkOperation(_Ty1& first, const _Ty2& second, const BYTE operation) {
			switch (operation) {
				case OperationService::OPERATION_EQUAL:
						return (first == second);
				case OperationService::OPERATION_BIGGER:
						return (first > second);
				case OperationService::OPERATION_BIGGER_EQUAL:
						return (first >= second);
				case OperationService::OPERATION_SMALLER:
						return (first < second);
				case OperationService::OPERATION_SMALLER_EQUAL:
						return (first <= second);
				case OperationService::OPERATION_NOT_EQUAL:
						return (first != second);
			}
			return false;
		}
		template<class _Ty> static _Ty resultOperation(_Ty first, const _Ty& second, const BYTE operation) {
			switch (operation) {
				case OperationService::OPERATION_ADDITION:
						return _Ty(first + second);
				case OperationService::OPERATION_SUBTRACTION:
						return _Ty(first - second);
				case OperationService::OPERATION_MULTIPLICATION:
						return _Ty(first * second);
				case OperationService::OPERATION_INCREMENT:
						return _Ty(first + 1);
				case OperationService::OPERATION_RETURN_RHS:
						return _Ty(second);
			}
			return _Ty(0);
		}
		static const char* operationName(BYTE operation) {
			switch (operation) {
				case OperationService::OPERATION_EQUAL:
					return "==";
				case OperationService::OPERATION_BIGGER:
					return ">";
				case OperationService::OPERATION_BIGGER_EQUAL:
					return ">=";
				case OperationService::OPERATION_SMALLER:
					return  "<";
				case OperationService::OPERATION_SMALLER_EQUAL:
					return "<=";
				case OperationService::OPERATION_NOT_EQUAL:
					return "!=";
				case OperationService::OPERATION_ADDITION:
					return "+";
				case OperationService::OPERATION_SUBTRACTION:
					return "-";
				case OperationService::OPERATION_MULTIPLICATION:
					return "*";
				case OperationService::OPERATION_INCREMENT:
					return "this++";
				case OperationService::OPERATION_RETURN_RHS:
					return "this = rhs";
			}
			return "UNKNOWN";
		}
		const static BYTE OPERATION_EQUAL = 0x00;
		const static BYTE OPERATION_BIGGER = 0x01;
		const static BYTE OPERATION_BIGGER_EQUAL = 0x02;
		const static BYTE OPERATION_SMALLER = 0x03;
		const static BYTE OPERATION_SMALLER_EQUAL = 0x04;
		const static BYTE OPERATION_RETURN_RHS = 0x05;
		const static BYTE OPERATION_ADDITION = 0x06;
		const static BYTE OPERATION_SUBTRACTION = 0x07;
		const static BYTE OPERATION_MULTIPLICATION = 0x08;
		const static BYTE OPERATION_INCREMENT = 0x09;
		const static BYTE OPERATION_NOT_EQUAL = 0x0A;
};

struct Item {
	BYTE type;
	WORD id;
	bool isAppraised;
	bool isSocketed;
	WORD lifespan;
	DWORD amount;
	BYTE durability;
	WORD refine;
	WORD gem;
	WORD stats;

	Item() {
		this->clear();
	}
	Item(const DWORD itemId) {
		this->clear();
		this->id = static_cast<WORD>(itemId % 1000);
		this->type = static_cast<BYTE>(itemId / 1000);
		this->amount = 0x01;
	}
	void clear() {
		this->type = 0x00;		
		this->gem = this->stats = this->refine = 0x00;
		this->id = 0x00;
		this->isAppraised = true;
		this->isSocketed = false;
		this->amount = 0x00;
		this->durability = 0x30;
		this->lifespan = 1000;
	}
	bool isValid() {
		if((this->type > 0 && this->type < ItemType::PAT) || this->type == ItemType::MONEY) {
			if(this->amount>0)
				return true;
		}
		return false;
	}
	const DWORD getPakVisuality() const {
		DWORD basicResult = (this->id | this->refine * 0x10000);
		if (this->gem == 0) {
			return basicResult;
		}
		return ((0xd0000) + ((this->gem - 320) * 0x400) | basicResult);
	}
	const WORD getPakHeader() const {
		if (this->amount == 0x00)
			return 0;
		WORD result = static_cast<WORD>((this->id << 5) & 0xFFE0);
		return static_cast<WORD>(result | (this->type & 0x1F));
	}
	const DWORD getPakData() const {
		if ((this->type >= ItemType::CONSUMABLES && this->type <= ItemType::QUEST) || this->type == ItemType::MONEY || this->amount == 0) {
			return this->amount;
		}

		//0101 1111 1001 0000
		DWORD refinePart = (this->refine >> 4) << 28;
		DWORD appraisePart = this->isAppraised << 27;
		DWORD socketPart = this->isSocketed << 26;
		DWORD lifeSpanPart = this->lifespan << 16;
		DWORD durabilityPart = this->durability << 9;
		DWORD stats = this->stats;
		DWORD gem = this->gem;
		if (gem != 0x00)
			stats = 0x00;

		return (refinePart | appraisePart | socketPart | lifeSpanPart | durabilityPart | stats | gem);
	}
};


struct Combat {
	class Entity* target;
	class ZMO* attackAnimation;

	//SkillEntry* = Skill* (typedef of it)
	class SkillEntry* skill;
	BYTE nextAttackId;
	DWORD animationTimePassed;

	Combat() { 
		this->target = nullptr; 
		this->animationTimePassed = 0x00;
		this->skill = nullptr;
	}

	__inline Entity* getTarget() const { return this->target; }
	__inline void setTarget( Entity* newTarget ) { this->target = newTarget; }
};

extern const DWORD makeQuestHash(const char *data);

class NPCData {
	private:
		WORD id;
		std::string name;
		WORD walkSpeed;
		WORD sprintSpeed;
		BYTE level;
		WORD hpPerLevel;
		DWORD maxHP;
		WORD attackPower;
		WORD hitrate;
		WORD defense;
		WORD magicDefense;
		WORD dodgeRate;
		WORD attackSpeed;
		WORD AIId;
		WORD expPerLevel;
		WORD moneyPercentage;
		WORD dropTableId;
		float attackRange;
		WORD dialogId;
		DWORD questHash;
		bool isNPC;
	public:
		NPCData() {
			this->id = this->dialogId = this->hpPerLevel = this->expPerLevel = 0x00;
			this->attackRange = 0.0f;
			this->name ="";
			this->level = 0x00;
		}
		NPCData(NPCSTB* stb, const WORD rowId) {
			this->id = rowId;
			this->name = stb->getName(rowId);
			this->walkSpeed = stb->getWalkSpeed(rowId);
			this->sprintSpeed = stb->getSprintSpeed(rowId);
			this->level = stb->getLevel(rowId);
			this->hpPerLevel = stb->getHPperLevel(rowId);
			this->maxHP = this->hpPerLevel * this->level;
			this->attackPower = stb->getAttackpower(rowId);
			this->hitrate = stb->getHitrate(rowId);
			this->defense = stb->getDefense(rowId);
			this->magicDefense = stb->getMagicDefense(rowId);
			this->dodgeRate = stb->getDodgerate(rowId);
			this->attackSpeed = stb->getAttackspeed(rowId);
			this->expPerLevel = stb->getExperience(rowId);
			this->attackRange = stb->getAttackrange(rowId);
			this->AIId = stb->getAIFileId(rowId);
			this->isNPC = stb->isNPCEntry(rowId);
			this->moneyPercentage = stb->getMoneyChance(rowId);
			this->dropTableId = stb->getDroptableId(rowId);
			this->questHash = ::makeQuestHash(stb->getQuestName(rowId).c_str());
			this->dialogId = 0x00;
		}
		__inline WORD getTypeId() const { return this->id; }
		__inline std::string getName() const { return this->name; }
		__inline BYTE getLevel() const { return this->level; }
		__inline WORD getHPPerLevel() const { return this->hpPerLevel; }
		__inline DWORD getMaxHP() const { return this->maxHP; }
		__inline WORD getAttackpower() const { return this->attackPower; }
		__inline WORD getHitrate() const { return this->hitrate; }
		__inline WORD getDefense() const { return this->defense; }
		__inline WORD getMagicDefense() const { return this->magicDefense; }
		__inline WORD getDodgerate() const { return this->dodgeRate; }
		__inline WORD getAttackspeed() const { return this->attackSpeed; }
		__inline WORD getWalkspeed() const { return this->walkSpeed; }
		__inline WORD getSprintSpeed() const { return this->sprintSpeed; }
		__inline WORD getExpPerLevel() const { return this->expPerLevel; }
		__inline float getAttackRange() const { return this->attackRange; }
		__inline WORD getMoneyPercentage() const { return this->moneyPercentage; }
		__inline WORD getDropPercentage() const { return 100 - this->moneyPercentage; }
		__inline WORD getDropTableId() const { return this->dropTableId; }
		__inline WORD getDialogId() const { return this->dialogId; }
		__inline WORD getAIId() const {return this->AIId; }
		__inline bool getIsNPC() const { return this->isNPC; }
		__inline const DWORD getQuestHash() const { return this->questHash; }
};

class StatType {
	private:
		StatType() {}
		~StatType() {}
	public:
		const static WORD JOB = 0x04;
		const static WORD UNION_FACTION = 0x05;
		const static WORD RANKING = 0x06;
		const static WORD REPUTATION = 0x07;
		const static WORD FACE_STYLE = 0x08;
		const static WORD HAIR_STYLE = 0x09;
		const static WORD STRENGTH = 0x0A;
		const static WORD DEXTERITY = 0x0B;
		const static WORD INTELLIGENCE = 0x0C;
		const static WORD CONCENTRATION = 0x0D;
		const static WORD CHARM = 0x0E;
		const static WORD SENSIBILITY = 0x0F;
		const static WORD CURRENT_HP = 0x10;
		const static WORD CURRENT_MP = 0x11;
		const static WORD ATTACK_POWER = 0x12;
		const static WORD DEFENSE_PHYSICAL = 0x13;
		const static WORD HIT_RATE = 0x14;
		const static WORD DEFENSE_MAGICAL = 0x15;
		const static WORD DODGE_RATE = 0x16;
		const static WORD MOVEMENT_SPEED = 0x17;
		const static WORD ATTACK_SPEED = 0x18;
		const static WORD INVENTORY_CAPACITY = 0x19;
		const static WORD CRIT_RATE = 0x1A;
		const static WORD HP_RECOVERY_RATE = 0x1B;
		const static WORD MP_RECOVERY_RATE = 0x1C;
		const static WORD MP_CONSUMPTION_RATE = 0x1D;
		const static WORD EXPERIENCE_RATE = 0x1E;
		const static WORD LEVEL = 0x1F;
		const static WORD STAT_POINT = 0x20; //????
		const static WORD TENDENCY = 0x21; //????
		const static WORD PK_LEVEL = 0x22;
		const static WORD HEAD_SIZE = 0x23;
		const static WORD BODY_SIZE = 0x24;
		const static WORD SKILL_POINTS = 0x25;
		const static WORD MAX_HP = 0x26;
		const static WORD MAX_MP = 0x27;
		const static WORD MONEY = 0x28;
};

class Telegate {
	private:
		struct mapPos {
			Position pos;
			WORD mapId;
		} source, dest;
	public:
		Telegate() { source.mapId = 0xFFFF; dest.mapId = 0xFFFF; }
		Telegate(const Position& sourcePos, const WORD sourceMapId, const Position& destPos, const WORD destMapId) {
			this->init(sourcePos, sourceMapId, destPos, destMapId);
		}
		void init(const Position& sourcePos, const WORD sourceMapId, const Position& destPos, const WORD destMapId) {
			this->source.pos = sourcePos;
			this->source.mapId = sourceMapId;
			this->dest.pos = destPos;
			this->dest.mapId = destMapId;
		}
		__inline Position getSourcePosition() const { return Position(this->source.pos); }
		__inline Position getDestPosition() const { return Position(this->dest.pos); }

		__inline const WORD getSourceMap() const { return this->source.mapId; }
		__inline const WORD getDestMap() const { return this->dest.mapId; }

};

#endif //__ROSE_STRUCTURES__