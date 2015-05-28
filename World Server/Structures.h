#pragma once

#ifndef __ROSE_STRUCTURES__
#define __ROSE_STRUCTURES__

#include <algorithm>
#include <time.h>
#include <string>
#include "FileTypes\ZMO.h"
#include "..\Common\Definitions.h"
#include "D:\Programmieren\Exceptions\CustomExceptions.h"
#include "D:\Programmieren\GlobalLogger\GlobalLogger.h"

class TimeDifference {
private:
	TimeDifference() { }
	~TimeDifference() { }

public:
	static clock_t of(const clock_t value) {
		return clock() - value;
	}
	static bool passed(const clock_t timeStamp, const dword_t difference) {
		if (static_cast<dword_t>(clock() - timeStamp) >= difference) {
			return true;
		}
		return false;
	}
};

class Buffs {
	private:
		const static byte_t TOTAL_COUNT = 0x20;
		struct buffEntry {
			clock_t endTime;
			word_t amount;
		};
		buffEntry buffs[Buffs::TOTAL_COUNT];
		clock_t currentTime;

		const byte_t _getActiveBuffCount() const;
		void internalClear(dword_t clearFlags);
	public:
		class Visuality {
		private:
			Visuality() { }
			~Visuality() { }
		public:
			const static byte_t HP_REST = 0x00;
			const static byte_t RESURRECTION = 0x01;
			const static byte_t POISON = 0x02;
			const static byte_t UNKNOWN = 0x03;
			const static byte_t HP_UP = 0x04;
			const static byte_t MP_UP = 0x05;
			const static byte_t MOVEMENT_UP = 0x06;
			const static byte_t MOVEMENT_DOWN = 0x07;

			const static byte_t ATTACKSPEED_UP = 0x08;
			const static byte_t ATTACKSPEED_DOWN = 0x09;
			const static byte_t ATTACKPOWER_UP = 0x0A;
			const static byte_t ATTACKPOWER_DOWN = 0x0B;
			const static byte_t DEFENSE_UP = 0x0C;
			const static byte_t DEFENSE_DOWN = 0x0D;
			const static byte_t MAGIC_DEFENSE_UP = 0x0E;
			const static byte_t MAGIC_DEFENSE_DOWN = 0x0F;

			const static byte_t HITRATE_UP = 0x10;
			const static byte_t HITRATE_DOWN = 0x11;
			const static byte_t CRITRATE_UP = 0x12;
			const static byte_t CRITRATE_DOWN = 0x13;
			const static byte_t DODGERATE_UP = 0x14;
			const static byte_t DODGERATE_DOWN = 0x15;
			const static byte_t MUTED = 0x16;
			const static byte_t SLEEPING = 0x17;

			const static byte_t STUNNED = 0x18;
			const static byte_t INVISIBLE_GM = 0x19;
			const static byte_t INVISIBLE_NORMAL = 0x1A;
			const static byte_t SHIELD_REFLECT = 0x1B;
			const static byte_t DAMAGE_UP = 0x1C;
			const static byte_t SUMMON = 0x1D;
			const static byte_t INVINCIBLE = 0x1E;
			const static byte_t FLAMING = 0x1F;
		};
		const static byte_t POSITIVE_BUFFS = 0x01;
		const static byte_t NEGATIVE_BUFFS = 0x02;
		const static byte_t ALL_BUFFS_NORMAL = 0x03;
		const static byte_t ALL_BUFFS_ADMIN_INCLUDED = 0x04;

		const static dword_t POSITIVE_BITS =
			(1 << Visuality::ATTACKPOWER_UP | 1 << Visuality::ATTACKSPEED_UP |
			1 << Visuality::CRITRATE_UP | 1 << Visuality::DAMAGE_UP | 1 << Visuality::DEFENSE_UP |
			1 << Visuality::DODGERATE_UP | 1 << Visuality::HITRATE_UP | 1 << Visuality::HP_UP |
			1 << Visuality::MAGIC_DEFENSE_UP | 1 << Visuality::MOVEMENT_UP | 1 << Visuality::MP_UP);
		const static dword_t NEGATIVE_BITS =
			(1 << Visuality::ATTACKPOWER_DOWN | 1 << Visuality::ATTACKSPEED_DOWN |
			1 << Visuality::CRITRATE_DOWN | 1 << Visuality::DEFENSE_DOWN |
			1 << Visuality::DODGERATE_DOWN | 1 << Visuality::FLAMING | 1 << Visuality::HITRATE_DOWN |
			1 << Visuality::MAGIC_DEFENSE_DOWN | 1 << Visuality::MOVEMENT_DOWN | 1 << Visuality::MUTED |
			1 << Visuality::POISON | Visuality::SLEEPING | 1 << Visuality::STUNNED);

		Buffs();
		~Buffs();

		Buffs& operator=(const Buffs& rhs);
		void clearBuff(byte_t clearType = 0xFF);
		bool addBuff(byte_t bitFromVisuality, word_t amount, dword_t durationInMilliseconds);
		__inline void removeBuff(const size_t pos);

		//Returns true if a buff was deleted
		bool checkBuffs();

		const word_t getStatusAmount(byte_t bitFromVisuality);

		/* byte_t = POSITIVE_BUFFS or NEGATIVE_BUFFS constants */
		const dword_t getVisuality(byte_t buffType);
};

struct Stats {
	word_t curHP;
	word_t curMP;
	dword_t maxHP;
	dword_t maxMP;

	word_t attackPower;
	word_t attackSpeed;
	clock_t attackDelay;
	word_t defensePhysical;
	word_t defenseMagical;
	word_t movementSpeed;
	word_t hitRate;
	word_t dodgeRate;
	word_t critRate;

	word_t stamina;

	Stats() {
		this->maxHP = this->maxMP = 100;
		this->curHP = this->curMP = 100;
		this->attackPower = this->attackSpeed = this->defenseMagical = this->defensePhysical = this->stamina = 0x00;
	}

	__inline word_t getCurrentHP() const { return this->curHP; }
	__inline word_t getCurrentMP() const { return this->curMP; }

	__inline dword_t getMaxHP() const { return this->maxHP; }
	__inline dword_t getMaxMP() const { return this->maxMP; }

	__inline word_t getAttackPower() const { return this->attackPower; }
	__inline word_t getAttackSpeed() const { return this->attackSpeed; }
	__inline word_t getDefensePhysical() const { return this->defensePhysical; }
	__inline word_t getDefenseMagical() const { return this->defenseMagical; }
	__inline word_t getMovementSpeed() const { return this->movementSpeed; }
	__inline word_t getHitrate() const { return this->hitRate; }
	__inline word_t getDodgerate() const { return this->dodgeRate; }
	__inline word_t getCritrate() const { return this->critRate; }

	__inline word_t getStamina() const { return this->stamina; }	
};

class Stance {
	private:
		byte_t type;
	public:
		const static byte_t NPC_WALKING = 0x00;
		const static byte_t NPC_RUNNING = 0x01;

		const static byte_t SITTING = 0x01;
		const static byte_t WALKING = 0x02;
		const static byte_t RUNNING = 0x03;
		const static byte_t DRIVING = 0x04;
		Stance() {
			type = Stance::RUNNING;
		}
		Stance& operator=(const byte_t& num) {
			this->type = num;
			return (*this);
		}
		Stance& operator=(const int& num) {
			this->type = static_cast<byte_t>(num);
			return (*this);
		}
		const byte_t asBYTE() const { return this->type; }
};

struct Status {
	time_t lastRegenCheck;
	Stance stance;
	Buffs buffs;

	const static byte_t DEFAULT_CHECK_TIME = 0x08;
	bool addBuff(const byte_t visualityBit, const word_t amount, const dword_t timeInMilliseconds);
	const dword_t getBuffsVisuality(byte_t buffType);
	bool checkBuffs();
	__inline Stance getStance() const { return stance; }
	__inline void setStance( const Stance& newStance ) { stance = newStance; }
	__inline void setStance( const byte_t& newStance ) { stance = newStance; }

	__inline void updateLastRegen() { this->lastRegenCheck = time(NULL); }
};
 
struct position_t {
	float x;
	float y;

	position_t() { x = y = 520000.0f; }
	explicit position_t(const float _x, const float _y) {
		this->x = _x;
		this->y = _y;
	}

	explicit position_t(const dword_t _x, const dword_t _y) {
		this->x = static_cast<float>(_x);
		this->y = static_cast<float>(_y);
	}

	position_t(const position_t& rhs) {
		this->x = rhs.x;
		this->y = rhs.y;
	}
	position_t& operator=(const position_t& rhs) {
		this->x = rhs.x;
		this->y = rhs.y;
		return (*this);
	}

	bool operator==(const position_t& rhs) {
		if (this->x == rhs.x && this->y == rhs.y)
			return true;
		return false;
	}
	bool operator!=(const position_t& rhs) {
		return !(this->operator==(rhs));
	}

	__inline float distanceTo(const position_t& rhs) {
		return this->distanceTo(rhs.x, rhs.y);
	}

	float distanceTo(const float _x, const float _y) {
		float xDist = (this->x - _x);
		float yDist = (this->y - _y);

		return static_cast<float>(sqrt((xDist * xDist) + (yDist * yDist)));
	}

	position_t calcNewPositionWithinRadius(const float radius) {
		//Get a random value and divide it by the diameter. In order to get a relation to the center, subtract it by the actual radius
#define RADIUS_CALCULATION ((rand() / static_cast<const dword_t>(radius)) * 2) - radius
		float preComma[2] = { RADIUS_CALCULATION, RADIUS_CALCULATION };
		float afterComma[2] = { static_cast<float>(rand() / static_cast<float>(RAND_MAX)), static_cast<float>(rand() / static_cast<float>(RAND_MAX)) };
#undef RADIUS_CALCULATION
		return position_t(preComma[0] + afterComma[0], preComma[1] + afterComma[1]);
	}

	__inline position_t copy() {
		return position_t(*this);
	}
};

class ItemType {
	private:
		ItemType() { };
		~ItemType() { };
	public:
		const static word_t FACE = 1;
		const static word_t HEADGEAR = 2;
		const static word_t ARMOR = 3;
		const static word_t GLOVES = 4;
		const static word_t SHOES = 5;
		const static word_t BACK = 6;
		const static word_t JEWELRY = 7;
		const static word_t WEAPON = 8;
		const static word_t SHIELD = 9;
		const static word_t CONSUMABLES = 10;
		const static word_t JEWELS = 11;
		const static word_t OTHER = 12;
		const static word_t QUEST = 13;
		const static word_t PAT = 14;
		const static word_t MONEY = 31; //6 Bits
};

class WeaponType {
	private:
		WeaponType() {}
		~WeaponType() {}
	public:
		const static word_t MELEE_ONE_HANDED_SWORD = 211;
		const static word_t MELEE_ONE_HANDED_BLUNT = 212;
		const static word_t MELEE_TWO_HANDED_SWORD = 221;
		const static word_t MELEE_TWO_HANDED_SPEAR = 222;
		const static word_t MELEE_TWO_HANDED_AXE = 223;
		const static word_t RANGE_BOW = 231;
		const static word_t RANGE_GUN = 232;
		const static word_t RANGE_LAUNCHER = 233;
		const static word_t MAGIC_WAND = 241;
		const static word_t MAGIC_STAFF = 242;
		const static word_t MELEE_KATAR = 251;
		const static word_t MELEE_DOUBLE_SWORD = 252;
		const static word_t RANGE_DUAL_GUN = 253;
		const static word_t RANGE_CROSSBOW = 271;
};

class JobType {
	private:
		JobType() {}
		~JobType() {}
	public:
		const static word_t VISITOR = 0;
		const static word_t SOLDIER = 111;
		const static word_t KNIGHT = 121;
		const static word_t CHAMPION = 122;

		const static word_t MUSE = 211;
		const static word_t MAGE = 221;
		const static word_t CLERIC = 222;

		const static word_t HAWKER = 311;
		const static word_t RAIDER = 321;
		const static word_t SCOUT = 322;

		const static word_t DEALER = 411;
		const static word_t BOURGEOIS = 421;
		const static word_t ARTISAN = 422;
};

class OperationService {
	private:
		OperationService() { }
		~OperationService() { }
	public:
		template<class _Ty1, class _Ty2> static bool checkOperation(_Ty1& first, const _Ty2& second, const byte_t operation) {
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
		template<class _Ty> static _Ty resultOperation(_Ty first, const _Ty& second, const byte_t operation) {
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
		static const char* operationName(byte_t operation) {
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
		const static byte_t OPERATION_EQUAL = 0x00;
		const static byte_t OPERATION_BIGGER = 0x01;
		const static byte_t OPERATION_BIGGER_EQUAL = 0x02;
		const static byte_t OPERATION_SMALLER = 0x03;
		const static byte_t OPERATION_SMALLER_EQUAL = 0x04;
		const static byte_t OPERATION_RETURN_RHS = 0x05;
		const static byte_t OPERATION_ADDITION = 0x06;
		const static byte_t OPERATION_SUBTRACTION = 0x07;
		const static byte_t OPERATION_MULTIPLICATION = 0x08;
		const static byte_t OPERATION_INCREMENT = 0x09;
		const static byte_t OPERATION_NOT_EQUAL = 0x0A;
};


struct Item {
	byte_t type;
	word_t id;
	bool isAppraised;
	bool isSocketed;
	word_t lifespan;
	dword_t amount;
	byte_t durability;
	word_t refine;
	word_t gem;
	word_t stats;

	Item() {
		this->clear();
	}
	Item(const dword_t itemId) {
		this->clear();
		this->id = static_cast<word_t>(itemId % 1000);
		this->type = static_cast<byte_t>(itemId / 1000);
		this->amount = 0x01;
	}
	Item(const byte_t itemType, const word_t itemId, const byte_t durability = 120, const word_t amount = 0x01) {
		this->clear();
		this->id = itemId;
		this->type = itemType;
		this->durability = durability;
		this->amount = amount;
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
	const dword_t getPakVisuality() const {
		dword_t basicResult = (this->id | this->refine * 0x10000);
		if (this->gem == 0) {
			return basicResult;
		}
		return ((0xd0000) + ((this->gem - 320) * 0x400) | basicResult);
	}
	const word_t getPakHeader() const {
		if (this->amount == 0x00)
			return 0;
		word_t result = static_cast<word_t>((this->id << 5) & 0xFFE0);
		return static_cast<word_t>(result | (this->type & 0x1F));
	}
	const dword_t getPakData() const {
		if ((this->type >= ItemType::CONSUMABLES && this->type <= ItemType::QUEST) || this->type == ItemType::MONEY || this->amount == 0) {
			return this->amount;
		}

		//0101 1111 1001 0000
		dword_t refinePart = (this->refine >> 4) << 28;
		dword_t appraisePart = this->isAppraised << 27;
		dword_t socketPart = this->isSocketed << 26;
		dword_t lifeSpanPart = this->lifespan << 16;
		dword_t durabilityPart = this->durability << 9;
		dword_t stats = this->stats;
		dword_t gem = this->gem;
		if (gem != 0x00)
			stats = 0x00;

		return (refinePart | appraisePart | socketPart | lifeSpanPart | durabilityPart | stats | gem);
	}
};
class PlayerInventory {
	public:
		class Slots {
			private:
				Slots() {}
				~Slots() {}
			public:
				const static word_t FACE = 1;
				const static word_t HEADGEAR = 2;
				const static word_t ARMOR = 3;
				const static word_t GLOVES = 6;
				const static word_t SHOES = 4;
				const static word_t BACK = 5;
				const static word_t WEAPON = 7;
				const static word_t SHIELD = 8;

				const static word_t ARROWS = 132;
				const static word_t BULLETS = 133;
				const static word_t CANNONSHELLS = 134;
				const static word_t CART_FRAME = 135;
				const static word_t CART_ENGINE = 136;
				const static word_t CART_WHEELS = 137;
				const static word_t CART_WEAPON = 138;
				const static word_t CART_ABILITY = 139;

				const static word_t TAB_SIZE = 30;
				const static word_t MAXIMUM = 140;
		};
	private:
		Item internalInventory[PlayerInventory::Slots::MAXIMUM];
	public:
		PlayerInventory() {
			for (unsigned int i = 0; i < PlayerInventory::Slots::MAXIMUM; i++) {
				this->internalInventory[i].clear();
			}
		}
		template<class _Ty, class = std::enable_if<std::is_integral<_Ty>::value>::type> const Item& operator[](const _Ty pos) const { return this->internalInventory[pos]; }
		template<class _Ty, class = std::enable_if<std::is_integral<_Ty>::value>::type> Item& operator[](const _Ty pos) { return this->internalInventory[pos]; }
		Item& operator[](int pos) { return this->internalInventory[pos]; }

		const static byte_t fromItemType(const byte_t itemType) {
			switch (itemType) {
			case ItemType::HEADGEAR:
				return PlayerInventory::Slots::HEADGEAR;
			case ItemType::FACE:
				return PlayerInventory::Slots::FACE;
			case ItemType::ARMOR:
				return PlayerInventory::Slots::ARMOR;
			case ItemType::GLOVES:
				return PlayerInventory::Slots::GLOVES;
			case ItemType::SHOES:
				return PlayerInventory::Slots::SHOES;
			case ItemType::BACK:
				return PlayerInventory::Slots::BACK;
			case ItemType::WEAPON:
				return PlayerInventory::Slots::WEAPON;
			case ItemType::SHIELD:
				return PlayerInventory::Slots::SHIELD;
			case ItemType::MONEY:
				return 0x00; //TEST
			}
			return PlayerInventory::Slots::MAXIMUM - 1;
		}
};

class Animation {
	private:
		clock_t initTimeStamp;
		word_t framesPlayed;
		ZMO* animation;
	public:
		Animation() {
			this->animation = nullptr;
			this->initTimeStamp = 0;
			this->framesPlayed = 0;
		}
		~Animation() {
			this->animation = nullptr;
			this->initTimeStamp = 0;
			this->framesPlayed = 0;
		}
		const ZMO* operator=(ZMO* newAnim) {
			this->animation = newAnim;
			this->framesPlayed = 0;
			this->initTimeStamp = clock();
			return this->animation;
		}
		bool operator==(std::nullptr_t) {
			return this->animation == nullptr;
		}
		bool operator!=(std::nullptr_t) {
			return !(this->animation == nullptr);
		}
		const word_t getFPS() const {
			if (!this->animation) {
				throw NullpointerException("Animation has an invalid state!");
			}
			return this->animation->getInfo().getFPS();
		}
		__inline float getTimePerFrame(const word_t speed = 100) const {
			float defaultTime = (1000.0f / static_cast<float>(this->getFPS()));
			return (defaultTime * 100.0f / static_cast<float>(speed));
		}
		const word_t getDefaultPlayTime() const {
			if (!this->animation) {
				throw NullpointerException("Animation has an invalid state!");
			}
			return this->animation->getInfo().getDefaultTime(); 
		}
		const word_t getAdjustedPlayTime(const word_t attackSpeed) const {
			if (!this->animation) {
				throw NullpointerException("Animation has an invalid state!");
			}
			return this->animation->getInfo().getAdjustedTime(attackSpeed);
		}
		const word_t getFrameAmount() const {
			if (!this->animation) {
				throw NullpointerException("Animation has an invalid state!");
			}
			return this->animation->getInfo().getFrameAmount();
		}
		const word_t getFramesAlreadyPlayed() const {
			return this->framesPlayed;
		}
		const word_t getFramesToPlay(const word_t speed = 100) const {
			clock_t timeDiff = clock() - this->initTimeStamp;
			word_t framesInTotal = static_cast<word_t>(timeDiff / this->getTimePerFrame(speed));
			return framesInTotal;
		}
		void setFramesAlreadyPlayed(const word_t framesPlayed) {
			this->framesPlayed = framesPlayed;
		}
		void reset(const word_t speed, const word_t framesPlayed) {
			word_t frameOverflow = framesPlayed - this->getFrameAmount();
			this->framesPlayed = 0;
			this->initTimeStamp = clock() - static_cast<clock_t>(frameOverflow * this->getTimePerFrame(speed));
		}
		const word_t getFrameType(const word_t frame) {
			if (this->animation != nullptr) {
				return this->animation->getTimingInfo().getFrameType(frame);
			}
			return (std::numeric_limits<word_t>::max)();
		}
};

struct Combat {
	const static byte_t NONE = 0; // No combat mode, e.g. standing around
	const static byte_t NORMAL = 1; //Normal chainable attack
	const static byte_t SKILL = 2; //A single targeted skill

	class Entity* target;
public:
	byte_t type; //Type of Combat -- see static bytes
	clock_t animationStartDelay;

	//SkillEntry* = Skill* (typedef of it)
	class SkillEntry* skill;

	Combat() { 
		this->target = nullptr; 
		this->skill = nullptr;
	}
	/*
		Clears all stats concerning a combat (this also includes the animation)
	*/
	void clear() {
		this->target = nullptr;
		this->type = Combat::NONE;
	}
	__inline Entity* getTarget() const { return this->target; }
	__inline void setTarget( Entity* newTarget ) { this->target = newTarget; }
};

extern const dword_t makeQuestHash(const char *data);

class NPCData {
	private:
		word_t id;
		std::string name;
		word_t walkSpeed;
		word_t sprintSpeed;
		byte_t level;
		word_t hpPerLevel;
		dword_t maxHP;
		word_t attackPower;
		word_t hitrate;
		word_t defense;
		word_t magicDefense;
		word_t dodgeRate;
		word_t attackSpeed;
		word_t AIId;
		word_t expPerLevel;
		word_t moneyPercentage;
		word_t dropTableId;
		float attackRange;
		word_t dialogId;
		dword_t questHash;
		bool isNPC;
	public:
		NPCData() {
			this->id = this->dialogId = this->hpPerLevel = this->expPerLevel = 0x00;
			this->attackRange = 0.0f;
			this->name ="";
			this->level = 0x00;
		}
		NPCData(class NPCSTB* stb, const word_t rowId);
		__inline word_t getTypeId() const { return this->id; }
		__inline std::string getName() const { return this->name; }
		__inline byte_t getLevel() const { return this->level; }
		__inline word_t getHPPerLevel() const { return this->hpPerLevel; }
		__inline dword_t getMaxHP() const { return this->maxHP; }
		__inline word_t getAttackpower() const { return this->attackPower; }
		__inline word_t getHitrate() const { return this->hitrate; }
		__inline word_t getDefense() const { return this->defense; }
		__inline word_t getMagicDefense() const { return this->magicDefense; }
		__inline word_t getDodgerate() const { return this->dodgeRate; }
		__inline word_t getAttackspeed() const { return this->attackSpeed; }
		__inline word_t getWalkspeed() const { return this->walkSpeed; }
		__inline word_t getSprintSpeed() const { return this->sprintSpeed; }
		__inline word_t getExpPerLevel() const { return this->expPerLevel; }
		__inline float getAttackRange() const { return this->attackRange; }
		__inline word_t getMoneyPercentage() const { return this->moneyPercentage; }
		__inline word_t getDropPercentage() const { return 100 - this->moneyPercentage; }
		__inline word_t getDropTableId() const { return this->dropTableId; }
		__inline word_t getDialogId() const { return this->dialogId; }
		__inline word_t getAIId() const {return this->AIId; }
		__inline bool getIsNPC() const { return this->isNPC; }
		__inline const dword_t getQuestHash() const { return this->questHash; }
};

class StatType {
	private:
		StatType() {}
		~StatType() {}
	public:
		const static word_t JOB = 0x04;
		const static word_t UNION_FACTION = 0x05;
		const static word_t RANKING = 0x06;
		const static word_t REPUTATION = 0x07;
		const static word_t FACE_STYLE = 0x08;
		const static word_t HAIR_STYLE = 0x09;
		const static word_t STRENGTH = 0x0A;
		const static word_t DEXTERITY = 0x0B;
		const static word_t INTELLIGENCE = 0x0C;
		const static word_t CONCENTRATION = 0x0D;
		const static word_t CHARM = 0x0E;
		const static word_t SENSIBILITY = 0x0F;
		const static word_t CURRENT_HP = 0x10;
		const static word_t CURRENT_MP = 0x11;
		const static word_t ATTACK_POWER = 0x12;
		const static word_t DEFENSE_PHYSICAL = 0x13;
		const static word_t HIT_RATE = 0x14;
		const static word_t DEFENSE_MAGICAL = 0x15;
		const static word_t DODGE_RATE = 0x16;
		const static word_t MOVEMENT_SPEED = 0x17;
		const static word_t ATTACK_SPEED = 0x18;
		const static word_t INVENTORY_CAPACITY = 0x19;
		const static word_t CRIT_RATE = 0x1A;
		const static word_t HP_RECOVERY_RATE = 0x1B;
		const static word_t MP_RECOVERY_RATE = 0x1C;
		const static word_t MP_CONSUMPTION_RATE = 0x1D;
		const static word_t EXPERIENCE_RATE = 0x1E;
		const static word_t LEVEL = 0x1F;
		const static word_t STAT_POINT = 0x20; //????
		const static word_t TENDENCY = 0x21; //????
		const static word_t PK_LEVEL = 0x22;
		const static word_t HEAD_SIZE = 0x23;
		const static word_t BODY_SIZE = 0x24;
		const static word_t SKILL_POINTS = 0x25;
		const static word_t MAX_HP = 0x26;
		const static word_t MAX_MP = 0x27;
		const static word_t MONEY = 0x28;
};

class Telegate {
	private:
		struct mapPos {
			position_t pos;
			word_t mapId;
		} source, dest;
	public:
		Telegate() { source.mapId = 0xFFFF; dest.mapId = 0xFFFF; }
		Telegate(const position_t& sourcePos, const word_t sourceMapId, const position_t& destPos, const word_t destMapId) {
			this->init(sourcePos, sourceMapId, destPos, destMapId);
		}
		void init(const position_t& sourcePos, const word_t sourceMapId, const position_t& destPos, const word_t destMapId) {
			this->source.pos = sourcePos;
			this->source.mapId = sourceMapId;
			this->dest.pos = destPos;
			this->dest.mapId = destMapId;
		}
		__inline position_t getSourcePosition() const { return position_t(this->source.pos); }
		__inline position_t getDestPosition() const { return position_t(this->dest.pos); }

		__inline const word_t getSourceMap() const { return this->source.mapId; }
		__inline const word_t getDestMap() const { return this->dest.mapId; }

};

#endif //__ROSE_STRUCTURES__