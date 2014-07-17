#pragma once

#ifndef __ROSE_STB__
#define __ROSE_STB__

#define __ROSE_APPLY_STL__
#pragma warning(disable:4996)

#include <vector>
#include <string>
#include <iostream>

#include "STL.h"

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;


class STBEntry {
	private:
		friend class STBFile;
		std::vector<std::string> columns;
	public:
		STBEntry(const DWORD& columnsAmount) {
			columns.reserve(columnsAmount);
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

class STBFile {
	protected:
		std::vector<STBEntry> entries;
		std::string filePath;

		bool storeData() {
			char tmpField[0x200];
			unsigned dataOffset = 0;
			unsigned short fieldlen = 0;

			FILE *fh = fopen(this->filePath.c_str(), "rb");
			if (!fh) {
				std::cout << "Couldn't open: " << this->filePath.c_str() << "\n";
				return false;
			}

			fseek(fh, 4, SEEK_SET);
			fread(&dataOffset, 4, 1, fh);
			
			DWORD entryCount = 0x00;
			fread(&entryCount, 4, 1, fh);
			entryCount--;
			this->entries.reserve(entryCount);

			DWORD columnCount = 0x00;
			fread(&columnCount, 4, 1, fh);
			columnCount--;
			for (unsigned int i = 0; i < entryCount; i++) {
				this->entries.push_back(STBEntry(columnCount));
			}
			
			fseek(fh, dataOffset, SEEK_SET);
			for (unsigned int i = 0; i < entryCount; i++) {
				STBEntry& entry = this->entries.at(i);
				entry.columns.reserve(columnCount);
				for (unsigned int j = 0; j < columnCount; j++) {
					fread(&fieldlen, 2, 1, fh);
					fread(tmpField, 1, fieldlen, fh);
					tmpField[fieldlen] = 0x00;
					entry.columns.push_back(std::string(tmpField));
				}
			}
			fclose(fh);
			return true;
		}
		STBFile() {
			this->filePath = "";
			this->entries.clear();
		}
		void construction() {
			this->entries.clear();

			this->storeData();

#ifdef __ROSE_APPLY_STL__
			std::string stlFile = filePath;
			stlFile = stlFile.substr(0,stlFile.find_last_of("."));
			stlFile += "_S.STL";
			STLFile stl(stlFile.c_str());
			for(unsigned int i=0;i<stl.size();i++) {
				STBEntry& entry = this->entries.at(stl.getEntryId(i));
				entry.columns.at(0x00) = stl.getEntryName(i).c_str();
			}
#endif //__ROSE_APPLY_STL__
		}
	public:
		STBFile(const char* filePath) {
			this->filePath = filePath;
			this->construction();
		}
		virtual ~STBFile() {
			this->filePath = "";
			this->entries.clear();
		}
		__inline virtual STBEntry& operator[](const size_t rowId) { return this->getRow(rowId); }
		__inline STBEntry& getRow(const DWORD rowId) { return this->entries.at(rowId); }
		__inline DWORD getRowCount() const { return this->entries.size(); }
};

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
		const static WORD MONEY_COLUMN = 0x13;
		const static WORD ATTACKRANGE_COLUMN = 0x1A;
		const static WORD AGGRO_COLUMN = 0x1B;
		NPCSTB(const char* filePath) {
			this->filePath = filePath;
			this->construction();
		}
		__inline std::string getName(const WORD row) {
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
		__inline WORD getMoney(const WORD row) {
			return this->entries.at(row).getColumn<WORD>(NPCSTB::MONEY_COLUMN);
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
		ZoneSTB(const char* filePath) {
			this->filePath = filePath;
			this->construction();
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
class SkillSTB : public STBFile {
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

		SkillSTB(const char* filePath) {
			this->filePath = filePath;
			this->construction();
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

class AISTB : public STBFile {
	public:
		const static WORD PATH_COLUMN = 0x00;
		AISTB(const char* filePath) {
			this->filePath = filePath;
			this->construction();
		}
		__inline std::string getFilePath(const WORD row) { return std::string(this->entries.at(row).getColumn(AISTB::PATH_COLUMN)); }
};
#endif //__ROSE_STB__