#pragma once

#ifndef __ROSE_STB__
#define __ROSE_STB__

#pragma warning(disable:4996)

#include <vector>
#include <string>
#include <iostream>

#include "STL.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;


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
				WORD length = file.read<WORD>();
				file.readString(length, tmpField);
				this->columns.push_back(std::string(tmpField));
			}
		}
		void changeName(std::string& newName) {
			this->columns.at(0x00) = newName;
		}
		__inline virtual const DWORD operator[](const size_t colId) {
			return this->getColumnAsInt(colId);
		}
		__inline std::string& getColumn(const size_t colId) {
			return this->columns.at(colId);
		}
		__inline const DWORD getColumnAsInt(const DWORD colId) {
			return static_cast<DWORD>(atol(this->columns.at(colId).c_str()));
		}
		template<class _Ty> __inline const _Ty getColumn(const DWORD colId) {
			return (std::is_arithmetic<_Ty>::value == true ? static_cast<_Ty>(this->getColumnAsInt(colId)) : _Ty(0x00));
		}

		__inline DWORD getColumnCount() const { return this->columns.size(); }
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
				WORD length = file.read<WORD>();
				file.readString(length, tmpField);
				this->columns.push_back(atol(tmpField));
			}
		}
		void changeName(std::string& newName) {
			//do nothing
		}
		__inline virtual const DWORD operator[](const size_t colId) {
			return this->getColumn(colId);
		}
		__inline const DWORD getColumn(const DWORD colId) {
			return this->columns.at(colId);
		}
		template<class _Ty> __inline const _Ty getColumn(const DWORD colId) {
			return (std::is_arithmetic<_Ty>::value == true ? static_cast<_Ty>(this->getColumn(colId)) : _Ty(0x00));
		}

		__inline DWORD getColumnCount() const { return this->columns.size(); }
};

template<class _STBEntry = ::STBEntry> class STBFile_Template {
	protected:
		std::vector<_STBEntry> entries;
		std::string filePath;

		template<class FileType> bool storeData(FileType& file) {
			unsigned dataOffset = 0;

			file.skip(0x04);
			dataOffset = file.read<DWORD>();
			
			DWORD entryCount = file.read<DWORD>();
			entryCount--;
			this->entries.reserve(entryCount);

			DWORD columnCount = file.read<DWORD>();
			columnCount--;
			for (unsigned int i = 0; i < entryCount; i++) {
				this->entries.push_back(_STBEntry());
			}
			
			file.setPosition(dataOffset);
			for (unsigned int i = 0; i < entryCount; i++) {
				_STBEntry& entry = this->entries.at(i);
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
			VFSData vfsData; pVFS->readFile(this->filePath.c_str(), vfsData);
			CMyBufferedReader reader(vfsData.data, vfsData.data.size());
			this->construction<CMyBufferedReader>(reader);

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
		__inline _STBEntry& getRow(const DWORD rowId) { return this->entries.at(rowId); }
		__inline DWORD getRowCount() const { return this->entries.size(); }
};

typedef STBFile_Template<> STBFile;

class NPCSTB : public STBFile {
	public:
		const static WORD NAME_COLUMN = 0x00;
		const static WORD WALKSPEED_COLUMN = 0x02;
		const static WORD SPRINTSPEED_COLUMN = 0x03;
		const static WORD LEVEL_COLUMN = 0x07;
		const static WORD HP_COLUMN = 0x08;
		const static WORD ATTACKPOWER_COLUMN = 0x09;
		const static WORD HITRATE_COLUMN = 0x0A;
		const static WORD DEFENSE_COLUMN = 0x0B;
		const static WORD MAGICDEFENSE_COLUMN = 0x0C;
		const static WORD DODGERATE_COLUMN = 0x0D;
		const static WORD ATTACKSPEED_COLUMN = 0x0E;
		const static WORD AI_COLUMN = 0x10;
		const static WORD EXPERIENCE_COLUMN = 0x11;
		const static WORD DROPTABLE_COLUMN = 0x12;
		const static WORD DROPCHANCE_COLUMN = 0x14;
		const static WORD ATTACKRANGE_COLUMN = 0x1A;
		const static WORD AGGRO_COLUMN = 0x1B;
		const static WORD QUEST_STRING_COLUMN = 0x29;
#ifdef __ROSE_USE_VFS__
		NPCSTB(VFS* pVFS, std::string pathInVFS) {
			this->filePath = pathInVFS;
			this->read(pVFS);
#else
		NPCSTB(const char* filePath) {
			this->read(filePath);
#endif
		}
		__inline const std::string& getName(const WORD row) {
			return this->entries.at(row).getColumn(NPCSTB::NAME_COLUMN);
		}
		__inline WORD getWalkSpeed(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::WALKSPEED_COLUMN);
		}
		__inline WORD getSprintSpeed(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::SPRINTSPEED_COLUMN);
		}
		__inline BYTE getLevel(const WORD row) {
			return this->entries.at(row).getColumn<BYTE>(NPCSTB::LEVEL_COLUMN);
		}
		__inline WORD getHPperLevel(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::HP_COLUMN);
		}
		__inline WORD getAttackpower(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::ATTACKPOWER_COLUMN);
		}
		__inline WORD getHitrate(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::HITRATE_COLUMN);
		}
		__inline WORD getDefense(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::DEFENSE_COLUMN);
		}
		__inline WORD getMagicDefense(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::MAGICDEFENSE_COLUMN);
		}
		__inline WORD getDodgerate(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::DODGERATE_COLUMN);
		}
		__inline WORD getAttackspeed(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::ATTACKSPEED_COLUMN);
		}
		__inline WORD getExperience(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::EXPERIENCE_COLUMN);
		}
		__inline WORD getDroptableId(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::DROPTABLE_COLUMN);
		}
		__inline WORD getMoneyChance(const WORD row) {
			return 100 - this->entries.at(row).getColumn<WORD>(NPCSTB::DROPCHANCE_COLUMN);
		}
		__inline WORD getDropChance(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::DROPCHANCE_COLUMN);
		}
		__inline WORD getAttackrange(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::ATTACKRANGE_COLUMN);
		}
		__inline WORD getAIFileId(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::AI_COLUMN);
		}
		__inline bool isNPCEntry(const WORD row) {
			return (this->entries.at(row).getColumn<WORD>(NPCSTB::AGGRO_COLUMN) == 999);
		}
		__inline const std::string& getQuestName(const WORD row) {
			return this->entries.at(row).getColumn(NPCSTB::QUEST_STRING_COLUMN);
		}
};
class ZoneSTB : public STBFile {
	public:
		const static WORD ZONE_COLUMN = 0x01;
		const static WORD NIGHT_ONLY_COLUMN = 0x04;
		const static WORD DAYCYCLE_LENGTH = 0x0D;
		const static WORD MORNING_BEGIN = 0x0E;
		const static WORD NOON_BEGIN = 0x0F;
		const static WORD EVENING_BEGIN = 0x10;
		const static WORD NIGHT_BEGIN = 0x11;
		const static WORD ZONESIZE_COLUMN = 0x19;
#ifdef __ROSE_USE_VFS__
		ZoneSTB(VFS* pVFS, std::string pathInVFS) {
			this->filePath = pathInVFS;
			this->read(pVFS);
#else
		ZoneSTB(const char* filePath) {
			this->read(filePath);
#endif
		}
		__inline WORD getId(const WORD& row) { return row; }
		__inline std::string getZoneFile(const WORD row) { return std::string(this->entries.at(row).getColumn(ZoneSTB::ZONE_COLUMN)); }
		__inline bool getIsNightOnly(const WORD row) { return this->entries.at(row).getColumn<WORD>(ZoneSTB::NIGHT_ONLY_COLUMN) > 0; }
		__inline WORD getDayLength(const WORD row) { return this->entries.at(row).getColumn<WORD>(ZoneSTB::DAYCYCLE_LENGTH); }
		__inline WORD getMorningTime(const WORD row) { return this->entries.at(row).getColumn<WORD>(ZoneSTB::MORNING_BEGIN); }
		__inline WORD getNoonTime(const WORD row) { return this->entries.at(row).getColumn<WORD>(ZoneSTB::NOON_BEGIN); }
		__inline WORD getEveningTime(const WORD row) { return this->entries.at(row).getColumn<WORD>(ZoneSTB::EVENING_BEGIN); }
		__inline WORD getNight(const WORD row) { return this->entries.at(row).getColumn<WORD>(ZoneSTB::NIGHT_BEGIN); }
		__inline DWORD getZoneSize(const WORD row) {
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
		const static WORD TYPE = 0x04;
		const static WORD PRICE = 0x05;
		const static WORD PRICE_RATE = 0x06;
		const static WORD WEIGHT = 0x07;
		const static WORD QUALITY = 0x08;
		const static WORD REQUIRED_LEVEL = 0x0D;
		const static WORD CRAFTING_STB_REFERENCE = 0x0E;
		const static WORD UNION_POINTS = 0x0F;
		const static WORD DURABILITY_MINIMUM = 0x1D;
		const static WORD STAT_FIRST_TYPE = 0x18;
		const static WORD STAT_FIRST_AMOUNT = 0x19;
		const static WORD STAT_SECOND_TYPE = 0x1A;
		const static WORD STAT_SECOND_AMOUNT = 0x1B;
		const static WORD DEFENSE_PHYISCAL = 0x1F;
		const static WORD DEFENSE_MAGICAL = 0x20;
		const static WORD ATTACK_RANGE = 0x21;
		const static WORD MOVEMENT_SPEED = 0x21;
		const static WORD MOTION_COLUMN = 0x22;
		const static WORD ATTACK_POWER_PHYSICAL = 0x23;
		const static WORD ATTACK_SPEED = 0x24;
		const static WORD ATTACK_POWER_MAGICAL = 0x25;
};

//TODO: FILL BLANKS
class SkillType {
	private:
		SkillType() {}
		~SkillType() {}
	public:
		const static BYTE BASIC = 1;
		const static BYTE CRAFTING = 2;
		const static BYTE PHYSICAL_DAMAGE = 3;
		const static BYTE POWER_UP_ONE = 4;
		const static BYTE POWER_UP_TWO = 5;
		const static BYTE MAGIC_DAMAGE_SINGLE = 5;
		const static BYTE MAGIC_DAMAGE_AOE = 5;
		const static BYTE RECOVERY_ACTION = 11;
		const static BYTE PASSIVE = 15;
		const static BYTE EMOTION = 16;
};

class SkillEntry : public STBEntry {
	public:
		const static BYTE CONDITIONS_MAX_NUM = 0x02;
		const static BYTE COSTS_MAX_NUM = 0x02;
		const static BYTE BUFF_MAX_NUM = 0x03;
		const static BYTE REQUIRED_SKILL_MAX_NUM = 0x03;
		const static BYTE CLASS_MAX_NUM = 0x04;
		const static BYTE WEAPONS_MAX_NUM = 0x05;

		const static BYTE COLUMN_BASIC_ID = 0x01;
		const static BYTE COLUMN_LEVEL = 0x02;
		const static BYTE COLUMN_REQUIRED_POINTS_PER_LEVELUP = 0x03;
		const static BYTE COLUMN_SKILLTYPE = 0x04;
		const static BYTE COLUMN_INITRANGE = 0x06;
		const static BYTE COLUMN_TARGETTYPE = 0x07;
		const static BYTE COLUMN_AOERANGE = 0x08;
		const static BYTE COLUMN_ATTACKPOWER = 0x09;
		const static BYTE COLUMN_DOESHARM = 0x0A;
		const static BYTE COLUMN_STATUS_FIRST = 0x0B;
		const static BYTE COLUMN_STATUS_SECOND = 0x0C;
		const static BYTE COLUMN_SUCCESSRATE = 0x0D;
		const static BYTE COLUMN_DURATION = 0x0E;
		const static BYTE COLUMN_COST_TYPE_FIRST = 0x10;
		const static BYTE COLUMN_COST_TYPE_SECOND = 0x12;
		const static BYTE COLUMN_COST_AMOUNT_FIRST = 0x11;
		const static BYTE COLUMN_COST_AMOUNT_SECOND = 0x13;
		const static BYTE COLUMN_COOLDOWN = 0x14;
		const static BYTE COLUMN_BUFF_TYPE_FIRST = 0x15;
		const static BYTE COLUMN_BUFF_TYPE_SECOND = 0x18;
		const static BYTE COLUMN_BUFF_TYPE_LAST = 0x0B;
		const static BYTE COLUMN_BUFF_FLATVALUE_FIRST = 0x16;
		const static BYTE COLUMN_BUFF_FLATVALUE_SECOND = 0x19;
		const static BYTE COLUMN_BUFF_FLATVALUE_LAST = 0x9;
		const static BYTE COLUMN_BUFF_PERCENTVALUE_FIRST = 0x17;
		const static BYTE COLUMN_BUFF_PERCENTVALUE_SECOND = 0x1A;
		const static BYTE COLUMN_WEAPONS_BEGIN = 0x1E;
		const static BYTE COLUMN_WEAPONS_END = 0x22;
		const static BYTE COLUMN_CLASS_BEGIN = 0x23;
		const static BYTE COLUMN_CLASS_END = 0x26;
		const static BYTE COLUMN_REQUIRED_SKILL_ID_FIRST = 0x27;
		const static BYTE COLUMN_REQUIRED_SKILL_ID_SECOND = 0x29;
		const static BYTE COLUMN_REQUIRED_SKILL_ID_LAST = 0x2B;

		const static BYTE COLUMN_REQUIRED_SKILL_LEVEL_FIRST = 0x28;
		const static BYTE COLUMN_REQUIRED_SKILL_LEVEL_SECOND = 0x2A;
		const static BYTE COLUMN_REQUIRED_SKILL_LEVEL_LAST = 0x2C;
		const static BYTE REQUIRED_SKILL_MAX = 0x03;

		const static BYTE COLUMN_REQUIRED_CONDITION_TYPE_FIRST = 0x2D;
		const static BYTE COLUMN_REQUIRED_CONDITION_TYPE_LAST = 0x30;
		const static BYTE COLUMN_REQUIRED_CONDITION_AMOUNT_FIRST = 0x2E;
		const static BYTE COLUMN_REQUIRED_CONDITION_AMOUNT_LAST = 0x31;

		static BYTE getPage(SkillEntry* entry) {
			switch (entry->getType()) {
				case SkillType::BASIC:
				case SkillType::EMOTION:
				case SkillType::RECOVERY_ACTION:
					return 0x00;
				break;
				case SkillType::PASSIVE:
					return 0x02;
			}
			return 0x01;
		}

		SkillEntry() {
		}

		__inline WORD getIdBasic() { return this->getColumn<WORD>(SkillEntry::COLUMN_BASIC_ID); }
		WORD getId() { 
			BYTE level = this->getLevel();
			WORD basicId = this->getColumn<WORD>(SkillEntry::COLUMN_BASIC_ID);
			if (level == 0)
				return basicId;
			return basicId + level - 1;
		}
		__inline BYTE getLevel() { return this->getColumn<BYTE>(SkillEntry::COLUMN_LEVEL); }
		__inline BYTE getRequiredPointsPerLevelup() { return this->getColumn<BYTE>(SkillEntry::COLUMN_REQUIRED_POINTS_PER_LEVELUP); }
		__inline BYTE getType() { return this->getColumn<BYTE>(SkillEntry::COLUMN_SKILLTYPE); }
		__inline DWORD getInitRange() { return this->getColumn<DWORD>(SkillEntry::COLUMN_INITRANGE); }
		__inline BYTE getTargetType() { return this->getColumn<BYTE>(SkillEntry::COLUMN_TARGETTYPE); }
		__inline DWORD getAOERange() {
			return this->getColumn<DWORD>(SkillEntry::COLUMN_AOERANGE);
		}
		__inline WORD getAttackpower() {
			return this->getColumn<WORD>(SkillEntry::COLUMN_ATTACKPOWER);
		}
		__inline bool getDoesHarm() {
			DWORD res = this->getColumn<DWORD>(SkillEntry::COLUMN_DOESHARM);
			return (res > 0);
		}
		__inline BYTE getStatus(const WORD rowId, bool firstStatusEQFalse_SecondStatusEQTrue) {
			BYTE colId = static_cast<BYTE>(firstStatusEQFalse_SecondStatusEQTrue) | SkillEntry::COLUMN_STATUS_FIRST;
			return this->getColumn<BYTE>(colId);
		}
		__inline BYTE getSuccessrate() {
			return this->getColumn<BYTE>(SkillEntry::COLUMN_SUCCESSRATE);
		}
		__inline WORD getDuration() {
			return this->getColumn<WORD>(SkillEntry::COLUMN_DURATION);
		}
		__inline BYTE getCostType(BYTE firstTypeFalse_secondTypeTrue) {
			BYTE colId = (firstTypeFalse_secondTypeTrue % 2) | SkillEntry::COLUMN_COST_TYPE_FIRST;
			return this->getColumn<BYTE>(colId);
		}
		__inline WORD getCostAmount(BYTE firstTypeFalse_secondTypeTrue) {
			BYTE colId = (firstTypeFalse_secondTypeTrue % 2) | COLUMN_COST_AMOUNT_FIRST;
			return this->getColumn<WORD>(colId);
		}
		__inline WORD getCooldown() {
			return this->getColumn<WORD>(SkillEntry::COLUMN_COOLDOWN);;
		}
		WORD getBuffType(BYTE wantedTypeOutOfThree) {
			wantedTypeOutOfThree %= SkillEntry::BUFF_MAX_NUM; //3 MAX
			wantedTypeOutOfThree += SkillEntry::COLUMN_BUFF_TYPE_FIRST;
			return this->getColumn<WORD>(wantedTypeOutOfThree);
		}
		WORD getBuffValueFlat(BYTE wantedTypeOutOfThree) {
			wantedTypeOutOfThree %= SkillEntry::BUFF_MAX_NUM;
			wantedTypeOutOfThree += SkillEntry::COLUMN_BUFF_FLATVALUE_FIRST;
			return this->getColumn<WORD>(wantedTypeOutOfThree);
		}
		WORD getBuffValuePercentage(BYTE wantedTypeOutOfThree) {
			wantedTypeOutOfThree %= SkillEntry::BUFF_MAX_NUM;
			BYTE colId = 0x00;
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

		WORD getWeaponType(BYTE weaponTypeOutOfFive) {
			weaponTypeOutOfFive %= (SkillEntry::COLUMN_WEAPONS_END - SkillEntry::COLUMN_WEAPONS_BEGIN);
			weaponTypeOutOfFive += SkillEntry::COLUMN_WEAPONS_BEGIN;
			return this->getColumn<WORD>(weaponTypeOutOfFive);
		}
		WORD getClassType(BYTE classTypeOutOfFour) {
			classTypeOutOfFour %= (SkillEntry::COLUMN_CLASS_END - SkillEntry::COLUMN_CLASS_BEGIN);
			classTypeOutOfFour += COLUMN_CLASS_BEGIN;
			return this->getColumn<WORD>(classTypeOutOfFour);
		}

		WORD getRequiredSkillID(BYTE idOutOfThree) {
			idOutOfThree %= REQUIRED_SKILL_MAX;
			idOutOfThree = (idOutOfThree * 2) + COLUMN_REQUIRED_SKILL_ID_FIRST;
			return this->getColumn<WORD>(idOutOfThree);
		}

		BYTE getRequiredSkillLevel(BYTE levelOutOfThree) {
			levelOutOfThree %= REQUIRED_SKILL_MAX;
			levelOutOfThree = (levelOutOfThree * 2) + COLUMN_REQUIRED_SKILL_LEVEL_FIRST;
			return this->getColumn<BYTE>(levelOutOfThree);
		}
		__inline BYTE getRequiredConditionType(BYTE typeOutOfTwo) {
			typeOutOfTwo = (typeOutOfTwo % SkillEntry::CONDITIONS_MAX_NUM) * 2;
			return this->getColumn<BYTE>(typeOutOfTwo + COLUMN_REQUIRED_CONDITION_TYPE_FIRST);
		}

		__inline WORD getRequiredConditionAmount(BYTE amountOutOfTwo) {
			amountOutOfTwo = (amountOutOfTwo % SkillEntry::CONDITIONS_MAX_NUM) * 2;
			return this->getColumn<WORD>(amountOutOfTwo + COLUMN_REQUIRED_CONDITION_AMOUNT_FIRST);
		}
};

class SkillSTB : public STBFile_Template<SkillEntry> {
	public:
		const static BYTE CONDITIONS_MAX_NUM = 0x02;
		const static BYTE COSTS_MAX_NUM = 0x02;
		const static BYTE BUFF_MAX_NUM = 0x03;
		const static BYTE REQUIRED_SKILL_MAX_NUM = 0x03;
		const static BYTE CLASS_MAX_NUM  = 0x04;
		const static BYTE WEAPONS_MAX_NUM = 0x05;
		
		const static BYTE COLUMN_LEVEL = 0x02;
		const static BYTE COLUMN_REQUIRED_POINTS_PER_LEVELUP = 0x03;
		const static BYTE COLUMN_SKILLTYPE = 0x04;
		const static BYTE COLUMN_INITRANGE = 0x06;
		const static BYTE COLUMN_TARGETTYPE = 0x07;
		const static BYTE COLUMN_AOERANGE = 0x08;
		const static BYTE COLUMN_ATTACKPOWER = 0x09;
		const static BYTE COLUMN_DOESHARM = 0x0A;		
		const static BYTE COLUMN_STATUS_FIRST = 0x0B;
		const static BYTE COLUMN_STATUS_SECOND = 0x0C;
		const static BYTE COLUMN_SUCCESSRATE = 0x0D;
		const static BYTE COLUMN_DURATION = 0x0E;
		const static BYTE COLUMN_COST_TYPE_FIRST = 0x10;
		const static BYTE COLUMN_COST_TYPE_SECOND = 0x12;
		const static BYTE COLUMN_COST_AMOUNT_FIRST = 0x11;
		const static BYTE COLUMN_COST_AMOUNT_SECOND = 0x13;
		const static BYTE COLUMN_COOLDOWN = 0x14;
		const static BYTE COLUMN_BUFF_TYPE_FIRST = 0x15;
		const static BYTE COLUMN_BUFF_TYPE_SECOND = 0x18;
		const static BYTE COLUMN_BUFF_TYPE_LAST = 0x0B;
		const static BYTE COLUMN_BUFF_FLATVALUE_FIRST = 0x16;
		const static BYTE COLUMN_BUFF_FLATVALUE_SECOND = 0x19;
		const static BYTE COLUMN_BUFF_FLATVALUE_LAST = 0x9;
		const static BYTE COLUMN_BUFF_PERCENTVALUE_FIRST = 0x17;
		const static BYTE COLUMN_BUFF_PERCENTVALUE_SECOND = 0x1A;
		const static BYTE COLUMN_WEAPONS_BEGIN = 0x1E;
		const static BYTE COLUMN_WEAPONS_END = 0x22;
		const static BYTE COLUMN_CLASS_BEGIN = 0x23;
		const static BYTE COLUMN_CLASS_END = 0x26;
		const static BYTE COLUMN_REQUIRED_SKILL_ID_FIRST = 0x27;
		const static BYTE COLUMN_REQUIRED_SKILL_ID_SECOND = 0x29;
		const static BYTE COLUMN_REQUIRED_SKILL_ID_LAST = 0x2B;
		
		const static BYTE COLUMN_REQUIRED_SKILL_LEVEL_FIRST = 0x28;
		const static BYTE COLUMN_REQUIRED_SKILL_LEVEL_SECOND = 0x2A;
		const static BYTE COLUMN_REQUIRED_SKILL_LEVEL_LAST = 0x2C;
		const static BYTE REQUIRED_SKILL_MAX = 0x03;
		
		const static BYTE COLUMN_REQUIRED_CONDITION_TYPE_FIRST = 0x2D;
		const static BYTE COLUMN_REQUIRED_CONDITION_TYPE_LAST = 0x30;
		const static BYTE COLUMN_REQUIRED_CONDITION_AMOUNT_FIRST = 0x2E;
		const static BYTE COLUMN_REQUIRED_CONDITION_AMOUNT_LAST = 0x31;
		
#ifdef __ROSE_USE_VFS__
		SkillSTB(VFS* pVFS, std::string pathInVFS) {
			this->filePath = pathInVFS;
			this->read(pVFS);
#else
		SkillSTB(const char* filePath) {
			this->read(filePath);
#endif
		}
		__inline BYTE getLevel(const WORD rowId) { return this->entries.at(rowId).getColumn<BYTE>(SkillSTB::COLUMN_LEVEL); }
		__inline BYTE getRequiredPointsPerLevelup(const WORD rowId) { return this->entries.at(rowId).getColumn<BYTE>(SkillSTB::COLUMN_REQUIRED_POINTS_PER_LEVELUP); }
		__inline BYTE getType(const WORD rowId) { return this->entries.at(rowId).getColumn<BYTE>(SkillSTB::COLUMN_SKILLTYPE); }
		__inline DWORD getInitRange(const WORD rowId) { return this->entries.at(rowId).getColumn<DWORD>(SkillSTB::COLUMN_INITRANGE); }
		__inline BYTE getTargetType(const WORD rowId) { return this->entries.at(rowId).getColumn<BYTE>(SkillSTB::COLUMN_TARGETTYPE); }
		__inline DWORD getAOERange(const WORD rowId) { 
			return this->entries.at(rowId).getColumn<DWORD>(SkillSTB::COLUMN_AOERANGE); 
		}
		__inline WORD getAttackpower(const WORD rowId) { 
			return this->entries.at(rowId).getColumn<WORD>(SkillSTB::COLUMN_ATTACKPOWER); 
		}
		__inline bool getDoesHarm(const WORD rowId) { 
			DWORD res = this->entries.at(rowId).getColumn<DWORD>(SkillSTB::COLUMN_DOESHARM);
			return (res > 0); 
		}
		__inline BYTE getStatus(const WORD rowId, bool firstStatusEQFalse_SecondStatusEQTrue) {
			BYTE colId = static_cast<BYTE>(firstStatusEQFalse_SecondStatusEQTrue) | SkillSTB::COLUMN_STATUS_FIRST;
			return this->entries.at(rowId).getColumn<BYTE>(colId);
		}
		__inline BYTE getSuccessrate(const WORD rowId) {
			return this->entries.at(rowId).getColumn<BYTE>(SkillSTB::COLUMN_SUCCESSRATE);
		}
		__inline WORD getDuration(const WORD rowId) {
			return this->entries.at(rowId).getColumn<WORD>(SkillSTB::COLUMN_DURATION);
		}
		__inline BYTE getCostType(const WORD rowId, BYTE firstTypeFalse_secondTypeTrue) {
			BYTE colId = (firstTypeFalse_secondTypeTrue % 2) | SkillSTB::COLUMN_COST_TYPE_FIRST;
			return this->entries.at(rowId).getColumn<BYTE>(colId);
		}
		__inline WORD getCostAmount(const WORD rowId, BYTE firstTypeFalse_secondTypeTrue) {
			BYTE colId = (firstTypeFalse_secondTypeTrue % 2) | COLUMN_COST_AMOUNT_FIRST;
			return this->entries.at(rowId).getColumn<WORD>(colId);
		}
		__inline WORD getCooldown(const WORD rowId) {
			return this->entries.at(rowId).getColumn<WORD>(SkillSTB::COLUMN_COOLDOWN);;
		}
		WORD getBuffType(const WORD rowId, BYTE wantedTypeOutOfThree) {
			wantedTypeOutOfThree %= SkillSTB::BUFF_MAX_NUM; //3 MAX
			if(wantedTypeOutOfThree == SkillSTB::BUFF_MAX_NUM) 
				wantedTypeOutOfThree--;
			wantedTypeOutOfThree += SkillSTB::COLUMN_BUFF_TYPE_FIRST;
			return this->entries.at(rowId).getColumn<WORD>(wantedTypeOutOfThree);
		}
		WORD getBuffValueFlat(const WORD rowId, BYTE wantedTypeOutOfThree) {
			wantedTypeOutOfThree %= SkillSTB::BUFF_MAX_NUM;
			if(wantedTypeOutOfThree == SkillSTB::BUFF_MAX_NUM) 
				wantedTypeOutOfThree--;
			wantedTypeOutOfThree += SkillSTB::COLUMN_BUFF_FLATVALUE_FIRST;
			return this->entries.at(rowId).getColumn<WORD>(wantedTypeOutOfThree);
		}
		WORD getBuffValuePercentage(const WORD rowId, BYTE wantedTypeOutOfThree) {
			wantedTypeOutOfThree %= SkillSTB::BUFF_MAX_NUM;
			if(wantedTypeOutOfThree == SkillSTB::BUFF_MAX_NUM) 
				wantedTypeOutOfThree--;
			BYTE colId = 0x00;
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
		
		WORD getWeaponType(const WORD rowId, BYTE weaponTypeOutOfFive) {
			weaponTypeOutOfFive %= (SkillSTB::COLUMN_WEAPONS_END - SkillSTB::COLUMN_WEAPONS_BEGIN);
			weaponTypeOutOfFive += SkillSTB::COLUMN_WEAPONS_BEGIN;
			return this->entries.at(rowId).getColumn<WORD>(weaponTypeOutOfFive);
		}
		WORD getClassType(const WORD rowId, BYTE classTypeOutOfFour) {
			classTypeOutOfFour %= (SkillSTB::COLUMN_CLASS_END - SkillSTB::COLUMN_CLASS_BEGIN);
			classTypeOutOfFour += COLUMN_CLASS_BEGIN;
			return this->entries.at(rowId).getColumn<WORD>(classTypeOutOfFour);
		}
		
		WORD getRequiredSkillID(const WORD rowId, BYTE idOutOfThree) {
			idOutOfThree %= REQUIRED_SKILL_MAX;
			idOutOfThree = (idOutOfThree * 2) + COLUMN_REQUIRED_SKILL_ID_FIRST;
			return this->entries.at(rowId).getColumn<WORD>(idOutOfThree);
		}
		
		BYTE getRequiredSkillLevel(const WORD rowId, BYTE levelOutOfThree) {
			levelOutOfThree %= REQUIRED_SKILL_MAX;
			levelOutOfThree = (levelOutOfThree * 2) + COLUMN_REQUIRED_SKILL_LEVEL_FIRST;
			return this->entries.at(rowId).getColumn<BYTE>(levelOutOfThree);
		}
		__inline BYTE getRequiredConditionType(const WORD rowId, BYTE typeOutOfTwo) {
			typeOutOfTwo = (typeOutOfTwo % SkillSTB::CONDITIONS_MAX_NUM) * 2;
			return this->entries.at(rowId).getColumn<BYTE>(typeOutOfTwo + COLUMN_REQUIRED_CONDITION_TYPE_FIRST);
		}

		__inline WORD getRequiredConditionAmount(const WORD rowId, BYTE amountOutOfTwo) {
			amountOutOfTwo = (amountOutOfTwo % SkillSTB::CONDITIONS_MAX_NUM) * 2;
			return this->entries.at(rowId).getColumn<WORD>(amountOutOfTwo + COLUMN_REQUIRED_CONDITION_AMOUNT_FIRST);
		}
		
};

class ConsumeSTB : public STBFile {
public:
	const static WORD STAT_TYPE_NEEDED = 0x11;
	const static WORD STAT_VALUE_NEEDED = 0x12;

	const static WORD STAT_TYPE_ADD = 0x13;
	const static WORD STAT_VALUE_ADD = 0x14;
	const static WORD STATUS_STB_REFERENCE = 0x18;
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

class AISTB : public STBFile {
	public:
		const static WORD PATH_COLUMN = 0x00;
#ifdef __ROSE_USE_VFS__
		AISTB(VFS* pVFS, std::string pathInVFS) {
			this->filePath = pathInVFS;
			this->read(pVFS);
#else
		AISTB(const char* filePath) {
			this->read(filePath);
#endif
		}
		__inline std::string getFilePath(const WORD row) { return std::string(this->entries.at(row).getColumn(AISTB::PATH_COLUMN)); }
};
#endif //__ROSE_STB__