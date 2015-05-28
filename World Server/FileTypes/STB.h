#pragma once

#ifndef __ROSE_STB__
#define __ROSE_STB__

#pragma warning(disable:4996)

#include <vector>
#include <string>
#include <iostream>

#include "STL.h"
#include "..\Structures.h"

class STBEntry {
	protected:
		std::vector<std::string> columns;
	public:
		STBEntry() {
			this->columns.clear();
		}
		template<class FileType> void init(FileType& file, const DWORD& columnCount) {
			char tmpField[0x400] = { 0x00 };
			for (unsigned int j = 0; j < columnCount; j++) {
				word_t length = file.readLengthThenString<word_t>(tmpField);
				if (length >= 0x400) {
					length = 0;
				}
				tmpField[length] = 0x00;
				this->columns.push_back(std::string(tmpField));
			}
		}
		void changeName(std::string& newName) {
			this->columns.at(0x00) = newName;
		}
		__inline virtual const dword_t operator[](const size_t colId) const {
			return this->getColumnAsInt(colId);
		}
		__inline const std::string& getColumn(const size_t colId) const {
			return this->columns.at(colId);
		}
		__inline const dword_t getColumnAsInt(const dword_t colId) const {
			return static_cast<DWORD>(atol(this->columns.at(colId).c_str()));
		}
		template<class _Ty> __inline const _Ty getColumn(const dword_t colId) const {
			return (std::is_arithmetic<_Ty>::value == true ? static_cast<_Ty>(this->getColumnAsInt(colId)) : _Ty(0x00));
		}

		__inline dword_t getColumnCount() const { return this->columns.size(); }
};

class STBEntry_INT {
	protected:
		std::vector<DWORD> columns;
	public:
		STBEntry_INT() {
			this->columns.clear();
		}
		template<class FileType> void init(FileType& file, const DWORD& columnCount) {
			char tmpField[0x400] = { 0x00 };
			for (unsigned int j = 0; j < columnCount; j++) {
				word_t length = file.readLengthThenString<word_t>(tmpField);
				tmpField[length] = 0x00;
				this->columns.push_back(atol(tmpField));
			}
		}
		void changeName(std::string& newName) {
			//do nothing
		}
		__inline virtual const dword_t operator[](const size_t colId) {
			return this->getColumn(colId);
		}
		__inline const dword_t getColumn(const dword_t colId) {
			return this->columns.at(colId);
		}
		template<class _Ty> __inline const _Ty getColumn(const dword_t colId) {
			return (std::is_arithmetic<_Ty>::value == true ? static_cast<_Ty>(this->getColumn(colId)) : _Ty(0x00));
		}

		__inline dword_t getColumnCount() const { return this->columns.size(); }
};

template<class _STBEntry = ::STBEntry> class STBFile_Template {
	protected:
		std::vector<_STBEntry> entries;
		std::string filePath;

		template<class FileType> bool storeData(FileType& file) {
			unsigned dataOffset = 0;

			file.skip(0x04);
			dataOffset = file.read<DWORD>();
			
			dword_t entryCount = file.read<DWORD>();
			entryCount--;
			this->entries.reserve(entryCount);

			dword_t columnCount = file.read<DWORD>();
			columnCount--;
			for (unsigned int i = 0; i < entryCount; i++) {
				this->entries.push_back(_STBEntry());
			}
			
			file.setPosition(dataOffset);
			for (unsigned int i = 0; i < entryCount; i++) {
				_STBEntry& entry = this->entries.at(i);
				if (i == 128) {
					i = i;
				}
				entry.init<FileType>(file, columnCount);
			}
			return true;
		}
		STBFile_Template() {
			this->entries.clear();
		}
		template<class FileType> void construction(FileType& file) {
			this->entries.clear();

			this->storeData<FileType>(file);
		}
#ifdef __ROSE_USE_VFS__
		void read(VFS* pVFS, bool applySTL = true) {
			GlobalLogger::debug("Loading STB: %s\n", this->filePath.c_str());
			VFSData vfsData; pVFS->readFile(this->filePath.c_str(), vfsData);
			CMyBufferedFileReader<char> reader(vfsData.data, vfsData.data.size());
			this->construction<CMyBufferedFileReader<char>>(reader);

			if (applySTL) {
				std::string stlPath = vfsData.filePath;
				stlPath = stlPath.substr(0, stlPath.find_last_of(".")) + std::string("_S.STL");
				STLFile stlFile(pVFS, stlPath.c_str());
				for (unsigned int i = 0; i < stlFile.size(); i++) {
					this->entries.at(stlFile.getEntryId(i)).changeName(stlFile.getEntryName(i));
				}
			}

#else
		void init(const char* filePath) {
			this->filePath = filePath;
			CMyFile file(this->filePath.c_str(), "rb");
			if (!file.exists()) {
				std::cout << "Couldn't open: " << this->filePath.c_str() << "\n";
				return;
			}
			this->construction<CMyFile>(file);
#endif
		}
	public:
#ifdef __ROSE_USE_VFS__
		STBFile_Template(VFS* pVFS, std::string pathInVFS, bool applySTL = true) {
			this->filePath = pathInVFS;
			this->read(pVFS, applySTL);

#else //__ROSE_USE_VFS__
		STBFile_Template(const char* filePath) {
			this->read(filePath);
#endif //__ROSE_USE_VFS__
		} 
		virtual ~STBFile_Template() {
			this->filePath = "";
			this->entries.clear();
		}
		__inline virtual _STBEntry& operator[](const size_t rowId) { return this->getRow(rowId); }
		__inline _STBEntry& getRow(const dword_t rowId) { return this->entries.at(rowId); }
		__inline dword_t getRowCount() const { return this->entries.size(); }
};

typedef STBFile_Template<> STBFile;

class NPCSTB : public STBFile {
	public:
		const static word_t NAME_COLUMN = 0x00;
		const static word_t WALKSPEED_COLUMN = 0x02;
		const static word_t SPRINTSPEED_COLUMN = 0x03;
		const static word_t LEVEL_COLUMN = 0x07;
		const static word_t HP_COLUMN = 0x08;
		const static word_t ATTACKPOWER_COLUMN = 0x09;
		const static word_t HITRATE_COLUMN = 0x0A;
		const static word_t DEFENSE_COLUMN = 0x0B;
		const static word_t MAGICDEFENSE_COLUMN = 0x0C;
		const static word_t DODGERATE_COLUMN = 0x0D;
		const static word_t ATTACKSPEED_COLUMN = 0x0E;
		const static word_t AI_COLUMN = 0x10;
		const static word_t EXPERIENCE_COLUMN = 0x11;
		const static word_t DROPTABLE_COLUMN = 0x12;
		const static word_t DROPCHANCE_COLUMN = 0x14;
		const static word_t ATTACKRANGE_COLUMN = 0x1A;
		const static word_t AGGRO_COLUMN = 0x1B;
		const static word_t QUEST_STRING_COLUMN = 0x29;
#ifdef __ROSE_USE_VFS__
		NPCSTB(VFS* pVFS, std::string pathInVFS) {
			this->filePath = pathInVFS;
			this->read(pVFS);
#else
		NPCSTB(const char* filePath) {
			this->read(filePath);
#endif
		}
		__inline const std::string& getName(const word_t row) {
			return this->entries.at(row).getColumn(NPCSTB::NAME_COLUMN);
		}
		__inline word_t getWalkSpeed(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::WALKSPEED_COLUMN);
		}
		__inline word_t getSprintSpeed(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::SPRINTSPEED_COLUMN);
		}
		__inline byte_t getLevel(const word_t row) {
			return this->entries.at(row).getColumn<BYTE>(NPCSTB::LEVEL_COLUMN);
		}
		__inline word_t getHPperLevel(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::HP_COLUMN);
		}
		__inline word_t getAttackpower(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::ATTACKPOWER_COLUMN);
		}
		__inline word_t getHitrate(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::HITRATE_COLUMN);
		}
		__inline word_t getDefense(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::DEFENSE_COLUMN);
		}
		__inline word_t getMagicDefense(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::MAGICDEFENSE_COLUMN);
		}
		__inline word_t getDodgerate(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::DODGERATE_COLUMN);
		}
		__inline word_t getAttackspeed(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::ATTACKSPEED_COLUMN);
		}
		__inline word_t getExperience(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::EXPERIENCE_COLUMN);
		}
		__inline word_t getDroptableId(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::DROPTABLE_COLUMN);
		}
		__inline word_t getMoneyChance(const word_t row) {
			return 100 - this->entries.at(row).getColumn<WORD>(NPCSTB::DROPCHANCE_COLUMN);
		}
		__inline word_t getDropChance(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::DROPCHANCE_COLUMN);
		}
		__inline word_t getAttackrange(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::ATTACKRANGE_COLUMN);
		}
		__inline word_t getAIFileId(const word_t row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::AI_COLUMN);
		}
		__inline bool isNPCEntry(const word_t row) {
			return (this->entries.at(row).getColumn<WORD>(NPCSTB::AGGRO_COLUMN) == 999);
		}
		__inline const std::string& getQuestName(const word_t row) {
			return this->entries.at(row).getColumn(NPCSTB::QUEST_STRING_COLUMN);
		}
};
class ZoneSTB : public STBFile {
	public:
		const static word_t ZONE_COLUMN = 0x01;
		const static word_t NIGHT_ONLY_COLUMN = 0x04;
		const static word_t DAYCYCLE_LENGTH = 0x0D;
		const static word_t MORNING_BEGIN = 0x0E;
		const static word_t NOON_BEGIN = 0x0F;
		const static word_t EVENING_BEGIN = 0x10;
		const static word_t NIGHT_BEGIN = 0x11;
		const static word_t ZONESIZE_COLUMN = 0x19;
#ifdef __ROSE_USE_VFS__
		ZoneSTB(VFS* pVFS, std::string pathInVFS) {
			this->filePath = pathInVFS;
			this->read(pVFS);
#else
		ZoneSTB(const char* filePath) {
			this->read(filePath);
#endif
		}
		__inline word_t getId(const WORD& row) { return row; }
		__inline std::string getZoneFile(const word_t row) { return std::string(this->entries.at(row).getColumn(ZoneSTB::ZONE_COLUMN)); }
		__inline bool getIsNightOnly(const word_t row) { return this->entries.at(row).getColumn<WORD>(ZoneSTB::NIGHT_ONLY_COLUMN) > 0; }
		__inline word_t getDayLength(const word_t row) { return this->entries.at(row).getColumn<WORD>(ZoneSTB::DAYCYCLE_LENGTH); }
		__inline word_t getMorningTime(const word_t row) { return this->entries.at(row).getColumn<WORD>(ZoneSTB::MORNING_BEGIN); }
		__inline word_t getNoonTime(const word_t row) { return this->entries.at(row).getColumn<WORD>(ZoneSTB::NOON_BEGIN); }
		__inline word_t getEveningTime(const word_t row) { return this->entries.at(row).getColumn<WORD>(ZoneSTB::EVENING_BEGIN); }
		__inline word_t getNight(const word_t row) { return this->entries.at(row).getColumn<WORD>(ZoneSTB::NIGHT_BEGIN); }
		__inline dword_t getZoneSize(const word_t row) {
			try {
				return this->entries.at(row).getColumn<DWORD>(ZoneSTB::ZONESIZE_COLUMN);
			}
			catch (...) {
			}
			return 4000;
		}
};

class EquipmentSTB {
	private:
		EquipmentSTB() { }
		~EquipmentSTB() { }
	public:
		const static word_t TYPE = 0x04;
		const static word_t PRICE = 0x05;
		const static word_t PRICE_RATE = 0x06;
		const static word_t WEIGHT = 0x07;
		const static word_t QUALITY = 0x08;
		const static word_t REQUIRED_LEVEL = 0x0D;
		const static word_t CRAFTING_STB_REFERENCE = 0x0E;
		const static word_t UNION_POINTS = 0x0F;
		const static word_t DURABILITY_MINIMUM = 0x1D;
		const static word_t STAT_FIRST_TYPE = 0x18;
		const static word_t STAT_FIRST_AMOUNT = 0x19;
		const static word_t STAT_SECOND_TYPE = 0x1A;
		const static word_t STAT_SECOND_AMOUNT = 0x1B;
		const static word_t DEFENSE_PHYISCAL = 0x1F;
		const static word_t DEFENSE_MAGICAL = 0x20;
		const static word_t ATTACK_RANGE = 0x21;
		const static word_t MOVEMENT_SPEED = 0x21;
		const static word_t MOTION_COLUMN = 0x22;
		const static word_t ATTACK_POWER_PHYSICAL = 0x23;
		const static word_t ATTACK_SPEED = 0x24;
		const static word_t ATTACK_POWER_MAGICAL = 0x25;
};

//TODO: FILL BLANKS
class SkillType {
	private:
		SkillType() {}
		~SkillType() {}
	public:
		const static byte_t BASIC = 1;
		const static byte_t CRAFTING = 2;
		const static byte_t PHYSICAL_DAMAGE = 3;
		const static byte_t POWER_UP_ONE = 4;
		const static byte_t POWER_UP_TWO = 5;
		const static byte_t MAGIC_DAMAGE_SINGLE = 5;
		const static byte_t MAGIC_DAMAGE_AOE = 5;
		const static byte_t RECOVERY_ACTION = 11;
		const static byte_t PASSIVE = 15;
		const static byte_t EMOTION = 16;
};


class PlayerSkill {
	private:
		PlayerSkill() {}
		~PlayerSkill() {}
	public:
		const static byte_t PAGE_SIZE = 30;
		const static byte_t BASIC_BEGIN = 0;
		const static byte_t ACTIVE_BEGIN = 30;
		const static byte_t PASSIVE_BEGIN = 60;
		const static byte_t PREMIUM_BEGIN = 90;
		const static byte_t PLAYER_MAX_QUICKBAR = 48;
		const static byte_t PLAYER_MAX_SKILLS = 120;

		const static byte_t LEARN_FAILED = 0x00;
		const static byte_t LEARN_SUCCESS = 0x01;
		const static byte_t LEARN_FALSE_JOB = 0x02;
		const static byte_t LEARN_REQUIRED_SKILL = 0x03;
		const static byte_t LEARN_NEEDS_ABILITY = 0x04; //e.g. Level
		const static byte_t LEARN_NO_SLOTS_LEFT = 0x05;
		const static byte_t LEARN_INVALID_SKILL = 0x06;
		const static byte_t LEARN_NEEDS_SKILLPOINTS = 0x07;
		const static byte_t LEARN_DELETE = 0x08;

		const static byte_t UPGRADE_SUCCESS = 0x00;
		const static byte_t UPGRADE_FAILED = 0x01;
		const static byte_t UPGRADE_NEEDS_SKILLPOINTS = 0x02;
		const static byte_t UPGRADE_NEEDS_ABILITY = 0x03;
		const static byte_t UPGRADE_FALSE_JOB = 0x04;
		const static byte_t UPGRADE_REQUIRED_SKILL = 0x05;
		const static byte_t UPGRADE_NEEDS_ZULIES = 0x06;
};

class SkillEntry : public STBEntry {
	public:
		const static byte_t CONDITIONS_MAX_NUM = 0x02;
		const static byte_t COSTS_MAX_NUM = 0x02;
		const static byte_t BUFF_MAX_NUM = 0x03;
		const static byte_t REQUIRED_SKILL_MAX_NUM = 0x03;
		const static byte_t CLASS_MAX_NUM = 0x04;
		const static byte_t WEAPONS_MAX_NUM = 0x05;

		const static byte_t COLUMN_BASIC_ID = 0x01;
		const static byte_t COLUMN_LEVEL = 0x02;
		const static byte_t COLUMN_REQUIRED_POINTS_PER_LEVELUP = 0x03;
		const static byte_t COLUMN_SKILLTYPE = 0x05;
		const static byte_t COLUMN_INITRANGE = 0x06;
		const static byte_t COLUMN_TARGETTYPE = 0x07;
		const static byte_t COLUMN_AOERANGE = 0x08;
		const static byte_t COLUMN_ATTACKPOWER = 0x09;
		const static byte_t COLUMN_DOESHARM = 0x0A;
		const static byte_t COLUMN_STATUS_FIRST = 0x0B;
		const static byte_t COLUMN_STATUS_SECOND = 0x0C;
		const static byte_t COLUMN_SUCCESSRATE = 0x0D;
		const static byte_t COLUMN_DURATION = 0x0E;
		const static byte_t COLUMN_COST_TYPE_FIRST = 0x10;
		const static byte_t COLUMN_COST_TYPE_SECOND = 0x12;
		const static byte_t COLUMN_COST_AMOUNT_FIRST = 0x11;
		const static byte_t COLUMN_COST_AMOUNT_SECOND = 0x13;
		const static byte_t COLUMN_COOLDOWN = 0x14;
		const static byte_t COLUMN_BUFF_TYPE_FIRST = 0x15;
		const static byte_t COLUMN_BUFF_TYPE_SECOND = 0x18;
		const static byte_t COLUMN_BUFF_TYPE_LAST = 0x0B;
		const static byte_t COLUMN_BUFF_FLATVALUE_FIRST = 0x16;
		const static byte_t COLUMN_BUFF_FLATVALUE_SECOND = 0x19;
		const static byte_t COLUMN_BUFF_FLATVALUE_LAST = 0x9;
		const static byte_t COLUMN_BUFF_PERCENTVALUE_FIRST = 0x17;
		const static byte_t COLUMN_BUFF_PERCENTVALUE_SECOND = 0x1A;
		const static byte_t COLUMN_WEAPONS_BEGIN = 0x1E;
		const static byte_t COLUMN_WEAPONS_END = 0x22;
		const static byte_t COLUMN_CLASS_BEGIN = 0x23;
		const static byte_t COLUMN_CLASS_END = 0x26;
		const static byte_t COLUMN_REQUIRED_SKILL_ID_FIRST = 0x27;
		const static byte_t COLUMN_REQUIRED_SKILL_ID_SECOND = 0x29;
		const static byte_t COLUMN_REQUIRED_SKILL_ID_LAST = 0x2B;

		const static byte_t COLUMN_REQUIRED_SKILL_LEVEL_FIRST = 0x28;
		const static byte_t COLUMN_REQUIRED_SKILL_LEVEL_SECOND = 0x2A;
		const static byte_t COLUMN_REQUIRED_SKILL_LEVEL_LAST = 0x2C;
		const static byte_t REQUIRED_SKILL_MAX = 0x03;

		const static byte_t COLUMN_REQUIRED_CONDITION_TYPE_FIRST = 0x2D;
		const static byte_t COLUMN_REQUIRED_CONDITION_TYPE_LAST = 0x30;
		const static byte_t COLUMN_REQUIRED_CONDITION_AMOUNT_FIRST = 0x2E;
		const static byte_t COLUMN_REQUIRED_CONDITION_AMOUNT_LAST = 0x31;
		const static byte_t COLUMN_ANIMATION_ID = 0x34;
		const static byte_t COLUMN_ANIMATION_SPEED = 0x35;

		const static byte_t COLUMN_REQUIRED_ZULIES = 0x55;

		static byte_t getPage(SkillEntry* entry) {
			switch (entry->getType()) {
				case SkillType::BASIC:
				case SkillType::EMOTION:
				case SkillType::RECOVERY_ACTION:
					return 0x00;
				break;
				case SkillType::PASSIVE:
					return 0x02 * PlayerSkill::PAGE_SIZE;
			}
			return PlayerSkill::PAGE_SIZE;
		}

		SkillEntry() {
		}

		__inline word_t getIdBasic() const { return this->getColumn<WORD>(SkillEntry::COLUMN_BASIC_ID); }
		const std::string& getName() const { return this->getColumn(0); }
		word_t getId() { 
			byte_t level = this->getLevel();
			word_t basicId = this->getColumn<WORD>(SkillEntry::COLUMN_BASIC_ID);
			if (level == 0)
				return basicId;
			return basicId + level - 1;
		}
		__inline byte_t getLevel() const { 
			return this->getColumn<BYTE>(SkillEntry::COLUMN_LEVEL); 
		}

		__inline byte_t getRequiredPointsPerLevelup() const { 
			return this->getColumn<BYTE>(SkillEntry::COLUMN_REQUIRED_POINTS_PER_LEVELUP); 
		}

		__inline byte_t getType() const { 
			return this->getColumn<BYTE>(SkillEntry::COLUMN_SKILLTYPE); 
		}

		__inline dword_t getInitRange() const { 
			return this->getColumn<DWORD>(SkillEntry::COLUMN_INITRANGE); 
		}

		__inline byte_t getTargetType() const { 
			return this->getColumn<BYTE>(SkillEntry::COLUMN_TARGETTYPE); 
		}

		__inline dword_t getAOERange() const {
			return this->getColumn<DWORD>(SkillEntry::COLUMN_AOERANGE);
		}

		__inline word_t getAttackpower() const {
			return this->getColumn<WORD>(SkillEntry::COLUMN_ATTACKPOWER);
		}

		__inline bool getDoesHarm() const {
			dword_t res = this->getColumn<DWORD>(SkillEntry::COLUMN_DOESHARM);
			return (res > 0);
		}
		__inline byte_t getStatus(const word_t rowId, bool firstStatusEQFalse_SecondStatusEQTrue) const {
			byte_t colId = static_cast<BYTE>(firstStatusEQFalse_SecondStatusEQTrue) | SkillEntry::COLUMN_STATUS_FIRST;
			return this->getColumn<BYTE>(colId);
		}
		__inline byte_t getSuccessrate() const {
			return this->getColumn<BYTE>(SkillEntry::COLUMN_SUCCESSRATE);
		}
		__inline word_t getDuration() const {
			return this->getColumn<WORD>(SkillEntry::COLUMN_DURATION);
		}
		__inline byte_t getCostType(byte_t firstTypeFalse_secondTypeTrue) {
			byte_t colId = (firstTypeFalse_secondTypeTrue % 2) | SkillEntry::COLUMN_COST_TYPE_FIRST;
			return this->getColumn<BYTE>(colId);
		}
		__inline word_t getCostAmount(byte_t firstTypeFalse_secondTypeTrue) const {
			byte_t colId = (firstTypeFalse_secondTypeTrue % 2) | COLUMN_COST_AMOUNT_FIRST;
			return this->getColumn<WORD>(colId);
		}
		__inline word_t getCooldown() const {
			return this->getColumn<WORD>(SkillEntry::COLUMN_COOLDOWN);;
		}
		word_t getBuffType(byte_t wantedTypeOutOfThree) const {
			wantedTypeOutOfThree %= SkillEntry::BUFF_MAX_NUM; //3 MAX
			wantedTypeOutOfThree += SkillEntry::COLUMN_BUFF_TYPE_FIRST;
			return this->getColumn<WORD>(wantedTypeOutOfThree);
		}
		word_t getBuffValueFlat(byte_t wantedTypeOutOfThree) const {
			wantedTypeOutOfThree %= SkillEntry::BUFF_MAX_NUM;
			wantedTypeOutOfThree += SkillEntry::COLUMN_BUFF_FLATVALUE_FIRST;
			return this->getColumn<WORD>(wantedTypeOutOfThree);
		}
		word_t getBuffValuePercentage(byte_t wantedTypeOutOfThree) const {
			wantedTypeOutOfThree %= SkillEntry::BUFF_MAX_NUM;
			byte_t colId = 0x00;
			switch (wantedTypeOutOfThree) {
			case 0x00:
				return this->getColumn<WORD>(SkillEntry::COLUMN_BUFF_PERCENTVALUE_FIRST);
				break;
			case 0x01:
				return this->getColumn<WORD>(SkillEntry::COLUMN_BUFF_PERCENTVALUE_SECOND);
				break;
			}
			return static_cast<WORD>(0x00);
		}

		word_t getWeaponType(byte_t weaponTypeOutOfFive) const {
			weaponTypeOutOfFive %= (SkillEntry::COLUMN_WEAPONS_END - SkillEntry::COLUMN_WEAPONS_BEGIN);
			weaponTypeOutOfFive += SkillEntry::COLUMN_WEAPONS_BEGIN;
			return this->getColumn<WORD>(weaponTypeOutOfFive);
		}
		word_t getClassType(byte_t classTypeOutOfFour) const {
			classTypeOutOfFour %= (SkillEntry::COLUMN_CLASS_END - SkillEntry::COLUMN_CLASS_BEGIN);
			classTypeOutOfFour += SkillEntry::COLUMN_CLASS_BEGIN;
			return this->getColumn<WORD>(classTypeOutOfFour);
		}

		word_t getRequiredSkillID(byte_t idOutOfThree) const {
			return (this->getRequiredSkillBasicID(idOutOfThree) + this->getRequiredSkillLevel(idOutOfThree));
		}

		word_t getRequiredSkillBasicID(byte_t idOutOfThree) const {
			idOutOfThree %= SkillEntry::REQUIRED_SKILL_MAX;
			idOutOfThree = (idOutOfThree * 2) + SkillEntry::COLUMN_REQUIRED_SKILL_ID_FIRST;
			return this->getColumn<WORD>(idOutOfThree);
		}

		byte_t getRequiredSkillLevel(byte_t levelOutOfThree) const {
			levelOutOfThree %= SkillEntry::REQUIRED_SKILL_MAX;
			levelOutOfThree = (levelOutOfThree * 2) + SkillEntry::COLUMN_REQUIRED_SKILL_LEVEL_FIRST;
			return this->getColumn<BYTE>(levelOutOfThree);
		}
		__inline byte_t getRequiredConditionType(byte_t typeOutOfTwo) const {
			typeOutOfTwo = (typeOutOfTwo % SkillEntry::CONDITIONS_MAX_NUM) * 2;
			return this->getColumn<BYTE>(typeOutOfTwo + SkillEntry::COLUMN_REQUIRED_CONDITION_TYPE_FIRST);
		}

		__inline word_t getRequiredConditionAmount(byte_t amountOutOfTwo) const {
			amountOutOfTwo = (amountOutOfTwo % SkillEntry::CONDITIONS_MAX_NUM) * 2;
			return this->getColumn<WORD>(amountOutOfTwo + SkillEntry::COLUMN_REQUIRED_CONDITION_AMOUNT_FIRST);
		}

		__inline dword_t getRequiredZulies() const {
			return this->getColumn<DWORD>(SkillEntry::COLUMN_REQUIRED_ZULIES) * 100;
		}

		__inline word_t getAnimationId() const {
			return this->getColumn<word_t>(SkillEntry::COLUMN_ANIMATION_ID);
		}
		
		__inline word_t getAnimationSpeed() const {
			return this->getColumn<word_t>(SkillEntry::COLUMN_ANIMATION_SPEED);
		}
};

typedef SkillEntry Skill;

class SkillSTB : public STBFile_Template<SkillEntry> {
	public:
		const static byte_t CONDITIONS_MAX_NUM = 0x02;
		const static byte_t COSTS_MAX_NUM = 0x02;
		const static byte_t BUFF_MAX_NUM = 0x03;
		const static byte_t REQUIRED_SKILL_MAX_NUM = 0x03;
		const static byte_t CLASS_MAX_NUM  = 0x04;
		const static byte_t WEAPONS_MAX_NUM = 0x05;
		
		const static byte_t COLUMN_LEVEL = 0x02;
		const static byte_t COLUMN_REQUIRED_POINTS_PER_LEVELUP = 0x03;
		const static byte_t COLUMN_SKILLTYPE = 0x04;
		const static byte_t COLUMN_INITRANGE = 0x06;
		const static byte_t COLUMN_TARGETTYPE = 0x07;
		const static byte_t COLUMN_AOERANGE = 0x08;
		const static byte_t COLUMN_ATTACKPOWER = 0x09;
		const static byte_t COLUMN_DOESHARM = 0x0A;		
		const static byte_t COLUMN_STATUS_FIRST = 0x0B;
		const static byte_t COLUMN_STATUS_SECOND = 0x0C;
		const static byte_t COLUMN_SUCCESSRATE = 0x0D;
		const static byte_t COLUMN_DURATION = 0x0E;
		const static byte_t COLUMN_COST_TYPE_FIRST = 0x10;
		const static byte_t COLUMN_COST_TYPE_SECOND = 0x12;
		const static byte_t COLUMN_COST_AMOUNT_FIRST = 0x11;
		const static byte_t COLUMN_COST_AMOUNT_SECOND = 0x13;
		const static byte_t COLUMN_COOLDOWN = 0x14;
		const static byte_t COLUMN_BUFF_TYPE_FIRST = 0x15;
		const static byte_t COLUMN_BUFF_TYPE_SECOND = 0x18;
		const static byte_t COLUMN_BUFF_TYPE_LAST = 0x0B;
		const static byte_t COLUMN_BUFF_FLATVALUE_FIRST = 0x16;
		const static byte_t COLUMN_BUFF_FLATVALUE_SECOND = 0x19;
		const static byte_t COLUMN_BUFF_FLATVALUE_LAST = 0x9;
		const static byte_t COLUMN_BUFF_PERCENTVALUE_FIRST = 0x17;
		const static byte_t COLUMN_BUFF_PERCENTVALUE_SECOND = 0x1A;
		const static byte_t COLUMN_WEAPONS_BEGIN = 0x1E;
		const static byte_t COLUMN_WEAPONS_END = 0x22;
		const static byte_t COLUMN_CLASS_BEGIN = 0x23;
		const static byte_t COLUMN_CLASS_END = 0x26;
		const static byte_t COLUMN_REQUIRED_SKILL_ID_FIRST = 0x27;
		const static byte_t COLUMN_REQUIRED_SKILL_ID_SECOND = 0x29;
		const static byte_t COLUMN_REQUIRED_SKILL_ID_LAST = 0x2B;
		
		const static byte_t COLUMN_REQUIRED_SKILL_LEVEL_FIRST = 0x28;
		const static byte_t COLUMN_REQUIRED_SKILL_LEVEL_SECOND = 0x2A;
		const static byte_t COLUMN_REQUIRED_SKILL_LEVEL_LAST = 0x2C;
		const static byte_t REQUIRED_SKILL_MAX = 0x03;
		
		const static byte_t COLUMN_REQUIRED_CONDITION_TYPE_FIRST = 0x2D;
		const static byte_t COLUMN_REQUIRED_CONDITION_TYPE_LAST = 0x30;
		const static byte_t COLUMN_REQUIRED_CONDITION_AMOUNT_FIRST = 0x2E;
		const static byte_t COLUMN_REQUIRED_CONDITION_AMOUNT_LAST = 0x31;
		
#ifdef __ROSE_USE_VFS__
		SkillSTB(VFS* pVFS, std::string pathInVFS) {
			this->filePath = pathInVFS;
			this->read(pVFS);
#else
		SkillSTB(const char* filePath) {
			this->read(filePath);
#endif
		}
		__inline byte_t getLevel(const word_t rowId) { return this->entries.at(rowId).getColumn<BYTE>(SkillSTB::COLUMN_LEVEL); }
		__inline byte_t getRequiredPointsPerLevelup(const word_t rowId) { return this->entries.at(rowId).getColumn<BYTE>(SkillSTB::COLUMN_REQUIRED_POINTS_PER_LEVELUP); }
		__inline byte_t getType(const word_t rowId) { return this->entries.at(rowId).getColumn<BYTE>(SkillSTB::COLUMN_SKILLTYPE); }
		__inline dword_t getInitRange(const word_t rowId) { return this->entries.at(rowId).getColumn<DWORD>(SkillSTB::COLUMN_INITRANGE); }
		__inline byte_t getTargetType(const word_t rowId) { return this->entries.at(rowId).getColumn<BYTE>(SkillSTB::COLUMN_TARGETTYPE); }
		__inline dword_t getAOERange(const word_t rowId) { 
			return this->entries.at(rowId).getColumn<DWORD>(SkillSTB::COLUMN_AOERANGE); 
		}
		__inline word_t getAttackpower(const word_t rowId) { 
			return this->entries.at(rowId).getColumn<WORD>(SkillSTB::COLUMN_ATTACKPOWER); 
		}
		__inline bool getDoesHarm(const word_t rowId) { 
			dword_t res = this->entries.at(rowId).getColumn<DWORD>(SkillSTB::COLUMN_DOESHARM);
			return (res > 0); 
		}
		__inline byte_t getStatus(const word_t rowId, bool firstStatusEQFalse_SecondStatusEQTrue) {
			byte_t colId = static_cast<BYTE>(firstStatusEQFalse_SecondStatusEQTrue) | SkillSTB::COLUMN_STATUS_FIRST;
			return this->entries.at(rowId).getColumn<BYTE>(colId);
		}
		__inline byte_t getSuccessrate(const word_t rowId) {
			return this->entries.at(rowId).getColumn<BYTE>(SkillSTB::COLUMN_SUCCESSRATE);
		}
		__inline word_t getDuration(const word_t rowId) {
			return this->entries.at(rowId).getColumn<WORD>(SkillSTB::COLUMN_DURATION);
		}
		__inline byte_t getCostType(const word_t rowId, byte_t firstTypeFalse_secondTypeTrue) {
			byte_t colId = (firstTypeFalse_secondTypeTrue % 2) | SkillSTB::COLUMN_COST_TYPE_FIRST;
			return this->entries.at(rowId).getColumn<BYTE>(colId);
		}
		__inline word_t getCostAmount(const word_t rowId, byte_t firstTypeFalse_secondTypeTrue) {
			byte_t colId = (firstTypeFalse_secondTypeTrue % 2) | COLUMN_COST_AMOUNT_FIRST;
			return this->entries.at(rowId).getColumn<WORD>(colId);
		}
		__inline word_t getCooldown(const word_t rowId) {
			return this->entries.at(rowId).getColumn<WORD>(SkillSTB::COLUMN_COOLDOWN);;
		}
		word_t getBuffType(const word_t rowId, byte_t wantedTypeOutOfThree) {
			wantedTypeOutOfThree %= SkillSTB::BUFF_MAX_NUM; //3 MAX
			if(wantedTypeOutOfThree == SkillSTB::BUFF_MAX_NUM) 
				wantedTypeOutOfThree--;
			wantedTypeOutOfThree += SkillSTB::COLUMN_BUFF_TYPE_FIRST;
			return this->entries.at(rowId).getColumn<WORD>(wantedTypeOutOfThree);
		}
		word_t getBuffValueFlat(const word_t rowId, byte_t wantedTypeOutOfThree) {
			wantedTypeOutOfThree %= SkillSTB::BUFF_MAX_NUM;
			if(wantedTypeOutOfThree == SkillSTB::BUFF_MAX_NUM) 
				wantedTypeOutOfThree--;
			wantedTypeOutOfThree += SkillSTB::COLUMN_BUFF_FLATVALUE_FIRST;
			return this->entries.at(rowId).getColumn<WORD>(wantedTypeOutOfThree);
		}
		word_t getBuffValuePercentage(const word_t rowId, byte_t wantedTypeOutOfThree) {
			wantedTypeOutOfThree %= SkillSTB::BUFF_MAX_NUM;
			if(wantedTypeOutOfThree == SkillSTB::BUFF_MAX_NUM) 
				wantedTypeOutOfThree--;
			byte_t colId = 0x00;
			switch(wantedTypeOutOfThree) {
				case 0x00:
					return this->entries.at(rowId).getColumn<WORD>(SkillSTB::COLUMN_BUFF_PERCENTVALUE_FIRST);
				break;
				case 0x01:
					return this->entries.at(rowId).getColumn<WORD>(SkillSTB::COLUMN_BUFF_PERCENTVALUE_SECOND);
				break;
			}
			return static_cast<WORD>(0x00);
		}
		
		word_t getWeaponType(const word_t rowId, byte_t weaponTypeOutOfFive) {
			weaponTypeOutOfFive %= (SkillSTB::COLUMN_WEAPONS_END - SkillSTB::COLUMN_WEAPONS_BEGIN);
			weaponTypeOutOfFive += SkillSTB::COLUMN_WEAPONS_BEGIN;
			return this->entries.at(rowId).getColumn<WORD>(weaponTypeOutOfFive);
		}
		word_t getClassType(const word_t rowId, byte_t classTypeOutOfFour) {
			classTypeOutOfFour %= (SkillSTB::COLUMN_CLASS_END - SkillSTB::COLUMN_CLASS_BEGIN);
			classTypeOutOfFour += COLUMN_CLASS_BEGIN;
			return this->entries.at(rowId).getColumn<WORD>(classTypeOutOfFour);
		}
		
		word_t getRequiredSkillID(const word_t rowId, byte_t idOutOfThree) {
			idOutOfThree %= REQUIRED_SKILL_MAX;
			idOutOfThree = (idOutOfThree * 2) + COLUMN_REQUIRED_SKILL_ID_FIRST;
			return this->entries.at(rowId).getColumn<WORD>(idOutOfThree);
		}
		
		byte_t getRequiredSkillLevel(const word_t rowId, byte_t levelOutOfThree) {
			levelOutOfThree %= REQUIRED_SKILL_MAX;
			levelOutOfThree = (levelOutOfThree * 2) + COLUMN_REQUIRED_SKILL_LEVEL_FIRST;
			return this->entries.at(rowId).getColumn<BYTE>(levelOutOfThree);
		}
		__inline byte_t getRequiredConditionType(const word_t rowId, byte_t typeOutOfTwo) {
			typeOutOfTwo = (typeOutOfTwo % SkillSTB::CONDITIONS_MAX_NUM) * 2;
			return this->entries.at(rowId).getColumn<BYTE>(typeOutOfTwo + COLUMN_REQUIRED_CONDITION_TYPE_FIRST);
		}

		__inline word_t getRequiredConditionAmount(const word_t rowId, byte_t amountOutOfTwo) {
			amountOutOfTwo = (amountOutOfTwo % SkillSTB::CONDITIONS_MAX_NUM) * 2;
			return this->entries.at(rowId).getColumn<WORD>(amountOutOfTwo + COLUMN_REQUIRED_CONDITION_AMOUNT_FIRST);
		}
		
};

class ConsumeSTB : public STBFile {
	public:
		const static word_t EXECUTION_TYPE = 0x07;
		const static word_t STAT_TYPE_NEEDED = 0x11;
		const static word_t STAT_VALUE_NEEDED = 0x12;

		const static word_t STAT_TYPE_ADD = 0x13;
		const static word_t STAT_VALUE_ADD = 0x14;
		const static word_t STATUS_STB_REFERENCE = 0x18;
	#ifdef __ROSE_USE_VFS__
		ConsumeSTB(VFS* pVFS, std::string pathInVFS) {
			this->filePath = pathInVFS;
			this->read(pVFS);

	#else
		ConsumeSTB(const char* filePath) {
			this->read(filePath);
	#endif
		}
};

class StatusSTB : public STBFile {
	public:
		const static word_t COLUMN_IS_STATUS_STACKABLE = 0x02;
		const static word_t COLUMN_BUFF_OR_DEBUFF = 0x03;
		const static word_t COLUMN_VALUE_INCREASE_FIRST = 0x06;
		const static word_t COLUMN_VALUE_INCREASE_SECOND = 0x08;

	#ifdef __ROSE_USE_VFS__
		StatusSTB(VFS* pVFS, std::string pathInVFS) {
			this->filePath = pathInVFS;
			this->read(pVFS);

	#else
		StatusSTB(const char* filePath) {
			this->read(filePath);
	#endif
		}
};

class AISTB : public STBFile {
	public:
		const static word_t PATH_COLUMN = 0x00;
#ifdef __ROSE_USE_VFS__
		AISTB(VFS* pVFS, std::string pathInVFS) {
			this->filePath = pathInVFS;
			this->read(pVFS);
#else
		AISTB(const char* filePath) {
			this->read(filePath);
#endif
		}
		__inline std::string getFilePath(const word_t row) { return std::string(this->entries.at(row).getColumn(AISTB::PATH_COLUMN)); }
};
#endif //__ROSE_STB__