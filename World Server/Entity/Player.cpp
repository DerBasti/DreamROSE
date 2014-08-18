#include "Player.h"
#include "Monster.h"
#include "Drop.h"
#include "..\WorldServer.h"
#include <stdlib.h>

CMyFile visualityLog;
long PLAYER_ATTACK_INTERVAL = 125000;

Player::Player(SOCKET sock, ServerSocket* server){ 
	visualityLog.openFile("D:\\Games\\iROSE Online Server\\Visuality.log", "a+");
	this->socket = sock;
	this->serverDelegate = server;
	this->entityInfo.type = Entity::TYPE_PLAYER;
	this->entityInfo.ingame = false;
	this->status.updateLastRegen();

	for (unsigned int i = 0; i < Inventory::MAXIMUM; i++) {
		this->inventory[i].clear();
	}
	this->skills.reserve(PlayerSkill::PLAYER_MAX_SKILLS);
	for (unsigned int i = 0; i < this->skills.capacity(); i++)
		this->skills.addValue(nullptr);

	this->quest.journey.reserve(Player::questInfo::JOURNEY_MAX);
	this->quest.var.episode.reserve(Player::questInfo::questVars::EPISODE_MAX);
	this->quest.var.fraction.reserve(Player::questInfo::questVars::UNION_MAX);
	this->quest.var.job.reserve(Player::questInfo::questVars::JOB_MAX);
	this->quest.var.planet.reserve(Player::questInfo::questVars::PLANET_MAX);
	for (unsigned int i = 0; i < this->quest.journey.capacity(); i++) {

		if (i < Player::questInfo::questVars::EPISODE_MAX)
			this->quest.var.episode.addValue(0x00);
		if (i < Player::questInfo::questVars::UNION_MAX)
			this->quest.var.fraction.addValue(0x00);
		if (i < Player::questInfo::questVars::JOB_MAX)
			this->quest.var.job.addValue(0x00);
		if (i < Player::questInfo::questVars::PLANET_MAX)
			this->quest.var.planet.addValue(0x00);

		this->quest.journey.addValue(new PlayerQuest(nullptr));
	}
}

Player::~Player() {
	this->saveInfos();
	for(unsigned int i=0;i<this->visibleSectors.size();i++) {
		this->removeSectorVisually(this->visibleSectors.getValue(i));
	}
	for (unsigned int i = 0; i < this->quest.journey.capacity(); i++)
		delete this->quest.journey.getValue(i);
}

void Player::setPositionCurrent(const Position& newPos) {
	this->position.current = newPos;
}

void Player::setPositionDest(const Position& newPos) {
	this->position.destination = newPos;
	this->setPositionVisually(newPos);
}

bool Player::setPositionVisually(const Position& newPos) {
	Entity* target = this->combat.getTarget();

	Packet pak(PacketID::World::Response::MOVEMENT_PLAYER);
	pak.addWord( this->getClientId() );
	pak.addWord( target != nullptr ? target->getClientId() : 0x00 );
	pak.addWord( this->getMovementSpeed() );
	pak.addFloat( newPos.x );
	pak.addFloat( newPos.y );
	pak.addWord( 0xcdcd ); //Z
	
	this->position.lastCheckTime = clock();

	return this->sendToVisible(pak);
}

const BYTE Player::findSlot( const Item& item ) {
	BYTE inventoryTab = 0x00;
	switch(item.type) {
		case ItemType::CONSUMABLES:
			inventoryTab = 0x01;
		break;
		case ItemType::JEWELRY:
		case ItemType::OTHER:
			inventoryTab = 0x02;
		break;
		case ItemType::PAT:
			inventoryTab = 0x03;
		break;
		default:
			if(item.type == 0x00 || item.type >= ItemType::PAT)
				return std::numeric_limits<BYTE>::max();
	}	
	BYTE slotId = 12 + (Inventory::TAB_SIZE * inventoryTab);
	if(inventoryTab == 0x01 || inventoryTab == 0x02) {
		DWORD totalCount = 0x00;
		for(unsigned int i=0;i<Inventory::TAB_SIZE;i++) {
			if(this->inventory[slotId].id == item.id && this->inventory[slotId].type == item.type) {
				totalCount = this->inventory[slotId].amount + item.amount;
				if(totalCount < 1000)
					return slotId;
			}
			slotId++;
		}
		slotId -= Inventory::TAB_SIZE;
		for(unsigned int i=0;i<Inventory::TAB_SIZE;i++) {
			if(!this->inventory[slotId].isValid()) {
				return slotId;
			}
			slotId++;
		}
	} else {
		for(unsigned int i=0;i<Inventory::TAB_SIZE;i++) {
			if(this->inventory[slotId].type == 0x00 && this->inventory[slotId].amount == 0x00)
				return slotId;
			slotId++;
		}
	}
	return std::numeric_limits<BYTE>::max();
}

void Player::addEntityVisually(Entity* entity) {
	if(!entity || entity == this)
		return;
	Player* player = nullptr; 
	switch(entity->getEntityType()) {
		case Entity::TYPE_PLAYER:
			player = dynamic_cast<Player*>(entity);
			this->pakSpawnPlayer(player);
			player->pakSpawnPlayer(this);
		break;
		case Entity::TYPE_NPC:
			this->pakSpawnNPC(dynamic_cast<NPC*>(entity));
		break;
		case Entity::TYPE_MONSTER:
			this->pakSpawnMonster(dynamic_cast<Monster*>(entity));
		break;
		case Entity::TYPE_DROP:
			this->pakSpawnDrop(dynamic_cast<Drop*>(entity));
		break;
	}
}

//Delegate
void Player::removeEntityVisually(Entity* entity) {
	this->pakRemoveEntityVisually(entity);
}

bool Player::pakRemoveEntityVisually(Entity* entity) {
	if(!entity)
		return false;

	//In case we're targeted, tell the entity to stop doing that.
	if(entity->getTarget() == this) {
		entity->setTarget(nullptr);
		entity->setPositionDest(entity->getPositionCurrent());
	}
	Packet pak(PacketID::World::Response::REMOVE_VISIBLE_PLAYER);
	pak.addWord(entity->getClientId());
	return this->sendData(pak);
}

void Player::checkRegeneration() {
	if(time(NULL) - this->status.lastRegenCheck >= Status::DEFAULT_CHECK_TIME) {
		if(this->getCurrentHP() != this->getMaxHPW() || this->getCurrentMP() != this->getMaxMPW()) {
			float hpRegenAmountF = static_cast<float>(ceil(this->getMaxHP() * 2.0f / 100.0f));
			float mpRegenAmountF = static_cast<float>(ceil(this->getMaxMP() * 2.0f / 100.0f));
			if(this->getStance().asBYTE() == Stance::SITTING) {
				hpRegenAmountF *= 4;
				mpRegenAmountF *= 4;
			}
			//Just to be sure we add enough HP when one sits
			WORD hpRegenAmount = static_cast<WORD>(hpRegenAmountF);
			WORD mpRegenAmount = static_cast<WORD>(mpRegenAmountF);

			this->stats.curHP += hpRegenAmount;
			if(this->stats.curHP > this->getMaxHPW())
				this->stats.curHP = this->getMaxHPW();

			this->stats.curMP += mpRegenAmount;
			if(this->stats.curMP > this->getMaxMPW())
				this->stats.curMP = this->getMaxMPW();

			this->pakUpdateLifeStats();
		}
		this->status.updateLastRegen();
	}
}

bool Player::pakUpdateLifeStats() {
	Packet pak( PacketID::World::Response::REGENERATION );
	pak.addWord( this->getCurrentHP() );
	pak.addWord( this->getCurrentMP() );
	return this->sendData(pak);
}

bool Player::searchAndSelectQuest(const DWORD questHash) {
	for (unsigned int i = 0; i < this->quest.journey.size(); i++) {
		PlayerQuest* entry = this->quest.journey[i];
		if (entry && entry->getQuestHash() == questHash) {
			this->quest.selected = this->quest.journey[i];
			return true;
		}
	}
	return false;
}

const WORD Player::getQuestVariable(WORD varType, const WORD varId) {
	varType >>= 8;
	try {
		switch (varType) {
			case 0x00:
				if (this->quest.selected)
					return this->quest.selected->getQuestVar(varId);
				return 0;
			case 0x01:
				return this->quest.var.switches[varId];
			case 0x02: //Time?
				return 0x00;
			case 0x03:
				return this->quest.var.episode[varId];
			case 0x04:
				return this->quest.var.job[varId];
			case 0x05:
				return this->quest.var.planet[varId];
			case 0x06:
				return this->quest.var.fraction[varId];
			default:
				throw TraceableExceptionARGS("varType %i is illegal!", varType);
		}
	} catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return 0x00;
}

Item Player::getItemFromInventory(const WORD itemSlot) {
	if (itemSlot == 0 || itemSlot >= Inventory::MAXIMUM)
		return Item();
	return this->inventory[itemSlot];
}

Item Player::getQuestItem(const DWORD itemId) {
	Item wantedItem(itemId);
	for (unsigned int i = 0; i < PlayerQuest::QUEST_ITEMS_MAX; i++) {
		Item currentItem = this->quest.selected->getItem(i);
		if (wantedItem.type == currentItem.type && wantedItem.id == currentItem.id)
			return currentItem;
	}
	return Item();
}

DWORD Player::getSpecialStatType(const WORD statType) {
	switch (statType) {
		case StatType::CHARM:
			return this->getCharmTotal();
		case StatType::CONCENTRATION:
			return this->getConcentrationTotal();
		case StatType::DEXTERITY:
			return this->getDexterityTotal();
		case StatType::FACE_STYLE:
			return this->getVisualTraits().faceStyle;
		case StatType::HAIR_STYLE:
			return this->getVisualTraits().hairStyle;
		case StatType::HEAD_SIZE:
			return 0;
		case StatType::HP_RECOVERY_RATE:
			return 0;
		case StatType::INTELLIGENCE:
			return this->getIntelligenceTotal();
		case StatType::JOB:
			return this->getJob();
		case StatType::MONEY:
			return this->inventory[0x00].amount;
		case StatType::MP_CONSUMPTION_RATE:
			return 0;
		case StatType::MP_RECOVERY_RATE:
			return 0;
		case StatType::PK_LEVEL:
		case StatType::POINT:
		case StatType::RANKING:
		case StatType::REPUTATION:
		case StatType::TENDENCY:
			return 0;
		case StatType::SENSIBILITY:
			return this->getSensibilityTotal();
		case StatType::SKILL_POINTS:
			return this->charInfo.skillPoints;
		case StatType::STRENGTH:
			return this->getStrengthTotal();
		case StatType::UNION_FACTION:
			return 0;
	}
	return 0;
}

//TODO: ADD TO STAT CALCULATION
WORD Player::checkClothesForStats(const WORD statAmount, ...) {
	std::vector<DWORD> stats;
	va_list ap;
	va_start(ap, statAmount);

	WORD result = 0x00;
	for(unsigned int i=0;i<statAmount;i++) {
		stats.push_back(va_arg(ap, DWORD));
	}
	for(unsigned int i=1;i<=Inventory::SHIELD;i++) {
		if(!this->inventory[i].isValid())
			continue;
		try {
			STBEntry& entry = mainServer->getEquipmentEntry( this->inventory[i].type, this->inventory[i].id );
			WORD firstStatType = entry.getColumn<WORD>( EquipmentSTB::STAT_FIRST_TYPE );
			WORD secondStatType = entry.getColumn<WORD>( EquipmentSTB::STAT_SECOND_TYPE );
			for(unsigned int k=0;k<stats.size();k++) {
				if(firstStatType == stats.at(k)) {
					result += entry.getColumn<WORD>( EquipmentSTB::STAT_FIRST_AMOUNT );
				}
				if(secondStatType == stats.at(k)) {
					result += entry.getColumn<WORD>( EquipmentSTB::STAT_SECOND_AMOUNT );
				}
			}
		} catch(std::exception& ex) {
			std::cout << ex.what() << "\n";
		}
	}
	return result;
}

WORD Player::checkSkillsForStats(const WORD basicAmount, const WORD statAmount, ...) {
	std::vector<DWORD> stats;
	va_list ap;
	va_start(ap, statAmount);

	WORD result = 0x00;
	for(unsigned int i=0;i<statAmount;i++) {
		stats.push_back(va_arg(ap, DWORD));
	}
	for(unsigned int i=0;i<this->skills.size();i++) {
		if(!this->skills[i])
			continue;
		
		Skill* currentSkill = this->skills[i];
		if(currentSkill->getType() != SkillType::PASSIVE)
			continue;
		for(unsigned int j=0;j<3;j++) {
			for(unsigned int k=0;k<stats.size();k++) {
				if(currentSkill->getBuffType(j) == stats.at(k)) {
					result += currentSkill->getBuffValueFlat(j);
					if(currentSkill->getBuffValuePercentage(j)>0)
						result += basicAmount * 100 / currentSkill->getBuffValuePercentage(j);
				}
			}
		}
	}
	return result;
}


//Ex.: Atkpower + 10 - (Defense * 0.7)+5 = 
//33 - 23 = ~10 Dmg Jelly Bean
//43 - 34 = ~9 Dmg Mother Choropy
//43 - 26 = ~17 Dmg
//43 - 18 = ~25 Dmg
//43 - 27 = ~16 Dmg (350 Hp ~ 20 hits)
//43 - 33 = ~10 Dmg (360 Hp ~ 30 Hits)
//52 - 33 = ~19 Dmg (360 hp ~ 15 hits)
//170 - 93 = ~80 dmg (1188 hp ~ 15 hits)
void Player::updateAttackpower() {
	WORD totalAttackpower = 0x00;
	WORD weaponAtkPower = 0x00;
	WORD weaponType = 0x00;
	if(this->inventory[Inventory::WEAPON].isValid()) {
		weaponAtkPower += mainServer->getWeaponAttackpower( this->inventory[Inventory::WEAPON].id );
		weaponType = mainServer->getSubType( ItemType::WEAPON, this->inventory[Inventory::WEAPON].id );
	} else {
		WORD dexPart = static_cast<WORD>(this->attributes.getDexterityTotal() * 0.3);
		WORD strPart = static_cast<WORD>(this->attributes.getStrengthTotal() * 0.5);
		WORD lvlPart = static_cast<WORD>(this->charInfo.level * 0.2);

		totalAttackpower = dexPart + strPart + lvlPart;
	}
	WORD weaponAtkParts[5] = { 0x00 };
	switch( weaponType ) {
		case WeaponType::MELEE_ONE_HANDED_SWORD:
		case WeaponType::MELEE_ONE_HANDED_BLUNT:
		case WeaponType::MELEE_TWO_HANDED_AXE:
		case WeaponType::MELEE_TWO_HANDED_SPEAR:
		case WeaponType::MELEE_TWO_HANDED_SWORD:
			weaponAtkParts[0] = static_cast<WORD>(this->attributes.getStrengthTotal() * 0.75);
			weaponAtkParts[1] = static_cast<WORD>(this->getLevel() * 0.2);
			weaponAtkParts[2] = static_cast<WORD>(((this->attributes.getStrengthTotal() * 0.05) + 29) * weaponAtkPower / 30.0);
		break;
		case WeaponType::MELEE_DOUBLE_SWORD:
			weaponAtkParts[0] = static_cast<WORD>(this->getLevel() * 0.2);
			weaponAtkParts[1] = static_cast<WORD>(this->attributes.getStrengthTotal() * 0.63);
			weaponAtkParts[2] = static_cast<WORD>(this->attributes.getDexterityTotal() * 0.45);
			weaponAtkParts[3] = static_cast<WORD>((this->attributes.getDexterityTotal() * 0.05 + 25) * weaponAtkPower / 26.0);
		break;
		case WeaponType::MELEE_KATAR:
			weaponAtkParts[0] = static_cast<WORD>(this->getLevel() * 0.2);
			weaponAtkParts[1] = static_cast<WORD>(this->attributes.getStrengthTotal() * 0.42);
			weaponAtkParts[2] = static_cast<WORD>(this->attributes.getDexterityTotal() * 0.55);
			weaponAtkParts[3] = static_cast<WORD>((this->attributes.getDexterityTotal() * 0.05 + 20) * weaponAtkPower / 21);
		break;
		case WeaponType::RANGE_BOW:
			weaponAtkParts[0] = static_cast<WORD>((this->attributes.getStrengthTotal() + this->getLevel()) * 0.1);
			weaponAtkParts[1] = static_cast<WORD>(((this->attributes.getDexterityTotal() * 0.04) + (this->attributes.getSensibilityTotal() * 0.03 + 29)) * weaponAtkPower / 30.0);
			weaponAtkParts[2] = static_cast<WORD>(this->attributes.getDexterityTotal() * 0.52);
			weaponAtkParts[3] = static_cast<WORD>(mainServer->getQuality(ItemType::OTHER, this->inventory[Inventory::ARROWS].id) * 0.5);
		break;
		case WeaponType::RANGE_GUN:
		case WeaponType::RANGE_DUAL_GUN:
			weaponAtkParts[0] = static_cast<WORD>(this->attributes.getSensibilityTotal() * 0.47);
			weaponAtkParts[1] = static_cast<WORD>(mainServer->getQuality(ItemType::OTHER, this->inventory[Inventory::BULLETS].id) * 0.8);
			weaponAtkParts[2] = static_cast<WORD>(this->getLevel() * 0.1);
			weaponAtkParts[3] = static_cast<WORD>(this->attributes.getDexterityTotal() * 0.3);
			weaponAtkParts[4] = static_cast<WORD>((this->attributes.getConcentrationTotal() * 0.04 + this->attributes.getSensibilityTotal() * 0.05 + 29) * weaponAtkPower / 30.0);
		break;
		case WeaponType::RANGE_LAUNCHER:
			weaponAtkParts[0] = static_cast<WORD>(this->attributes.getConcentrationTotal() * 0.47);
			weaponAtkParts[1] = static_cast<WORD>(mainServer->getQuality(ItemType::OTHER, this->inventory[Inventory::CANNONSHELLS].id));
			weaponAtkParts[2] = static_cast<WORD>(this->attributes.getStrengthTotal() * 0.32);
			weaponAtkParts[3] = static_cast<WORD>(this->attributes.getConcentrationTotal() * 0.45);
			weaponAtkParts[4] = static_cast<WORD>((this->attributes.getConcentrationTotal() * 0.04 +
				this->attributes.getSensibilityTotal() * 0.05 + 29) * weaponAtkPower / 30.0);
		break;
		case WeaponType::RANGE_CROSSBOW:
			weaponAtkParts[0] = static_cast<WORD>((this->attributes.getStrengthTotal() + this->getLevel()) * 0.1);
			weaponAtkParts[1] = static_cast<WORD>(this->attributes.getDexterityTotal() * 0.04);
			weaponAtkParts[2] = static_cast<WORD>((this->attributes.getSensibilityTotal() * 0.03 + 29) * weaponAtkPower / 30);
			weaponAtkParts[3] = static_cast<WORD>(this->attributes.getDexterityTotal() * 0.52);
			weaponAtkParts[4] = static_cast<WORD>(mainServer->getQuality( ItemType::OTHER, this->inventory[Inventory::ARROWS].id ) * 0.5);
		break;
		case WeaponType::MAGIC_WAND:
			weaponAtkParts[0] = static_cast<WORD>(this->getLevel() * 0.2);
			weaponAtkParts[1] = static_cast<WORD>(this->attributes.getIntelligenceTotal() * 0.6);
			weaponAtkParts[2] = static_cast<WORD>((this->attributes.getSensibilityTotal() * 0.1 + 26) * weaponAtkPower / 27);
		break;
		case WeaponType::MAGIC_STAFF:
			weaponAtkParts[0] = static_cast<WORD>(this->getLevel() * 0.2);
			weaponAtkParts[1] = static_cast<WORD>((this->attributes.getIntelligenceTotal() + this->attributes.getStrengthTotal()) * 0.4);
			weaponAtkParts[2] = static_cast<WORD>((this->attributes.getIntelligenceTotal() * 0.05 + 29) * weaponAtkPower / 30.0);
		break;
	}
	for(unsigned int i=0;i<5;i++)
		totalAttackpower += weaponAtkParts[i];

	totalAttackpower += this->getBuffAmount( Buffs::Visuality::ATTACKPOWER_UP );
	totalAttackpower -= this->getBuffAmount( Buffs::Visuality::ATTACKPOWER_DOWN );

	this->stats.attackPower = totalAttackpower;
}

/**************************************************
Weapon: 6 = 136
Weapon: 7 = 125
Weapon: 8 = 115
Weapon: 9 = 107
Weapon: 10 = 100
Weapon: 11 = 93
Weapon: 12 = 88
Weapon: 13 = 83
-----------------

x / weaponSpeed = Atkspeed

Weaponspeeds:
6: x = 6 * 136 = 816
7: x = 7 * 125 = 875
8: x = 8 * 115 = 920
9: x = 9 * 107 = 963
10: x = 10 * 100 = 1000
11: x = 11 * 93 = 1023
12: x = 12 * 88 = 1056
13: x = 13 * 83 = 1079
-----------------------
Growthrate decreases;
Theory: divisor increases with increasing weaponSpeedType (static?)

Y = 1; difference too big
---------
x / (weaponSpeed + y) = AtkSpeed
x = Atkspeed * (weaponSpeed + y)
x = 136 * (6 + 1)
x = 136 * 7
x = 952

x = 125 * 8
x = 1000
----------

Y = 2; difference too big
-----------
x = 136 * 8
x = 1088

x = 125 * 9
x = 1125

-----------
Y=4; difference smaller, still too big

x = 136 * 10
x = 1360

x = 125 * 11
x = 1375

----------
Y = 6; difference smaller, still too big

x = 136 * 12
x = 1632

x = 125 * 13
x = 1625

x = 115 * 14
x = 1610
------------
Y = 5.5; first two values are suiting, afterwards too varying

x = 136 * 11.5
x = 1564

x = 125 * 12.5
x = 1562,5

x = 115 * 13.5
= 1552,5
------------
Y = 5.25; very fitting results

x = 136 * 11.25
x = 1530

x = 125 * 12.25
  = 1531,25

x = 115 * 13,25
x = 1523,75

x = 107 * 14,25
x = 1524,75

x = 100 * 15,25
x = 1525,00

x = 93 * 16,25
x = 1511,25

x = 88 * 17,25
  = 1518

x = 83 * 18,25
x = 1514,75
----------------
1522,34

REVERSAL_PROCESS:
1522,34 / (6 + 5,25) = 135,00 (+1) = 136
1522,34 / (7 + 5,25) = 124,28 (+1) = 125
1522,34 / (8 + 5,25) = 114,89 (+1) = 115
1522,34 / (9 + 5,25) = 106,83 (+1) = 107
1522,34 / (10 + 5,25) = 99,82 (+1) = 100
1522,34 / (11 + 5,25) = 93,68 (+1) = 94  --- FAIL
----------------
Y = 5.0; best results

136 * 11 = 1496
125 * 12 = 1500
115 * 13 = 1495
107 * 14 = 1498
100 * 15 = 1500
93 * 16 = 1488
88 * 17 = 1496
83 * 18 = 1494
----------------
1496

1496 / (6 + 5)  = 136,0
1496 / 12 = 124,6666 -> FAIL; 

As the first value has "space" until the next integer occurs,
we can safely increase the average static value from before:
125 * 12 = 1500
----------------
1500

1500 / (6+5) = 136,36
1500 / (7+5) = 125,00
1500 / (8+5) = 115,38
1500 / (9+5) = 107,14
1500 / (10+15) = 100,00
...............
NECESSARY FUNCTION: 
1500 / (WEAPON_SPEED + 5)
*****************************************************/

void Player::updateAttackSpeed() {
	WORD atkSpeed = 115;
	if(this->isWeaponEquipped()) {
		int speedType = mainServer->getWeaponAttackspeed(this->inventory[Inventory::WEAPON].id);
		atkSpeed = 1500 / (speedType + 5);
	}
	atkSpeed += this->checkClothesForStats(1, StatType::ATTACK_SPEED);
	atkSpeed += this->getBuffAmount( Buffs::Visuality::ATTACKSPEED_UP );
	atkSpeed -= this->getBuffAmount( Buffs::Visuality::ATTACKSPEED_DOWN );

	this->stats.attackSpeed = atkSpeed;
}

bool Player::getAttackAnimation() {
	this->combat.attackAnimation = mainServer->getAttackMotionPlayer(mainServer->getWeaponMotion(this->inventory[Inventory::WEAPON].id)); 
	return this->combat.attackAnimation != nullptr;
}

void Player::updateDefense() {
	WORD defense = 0x00;
	if(this->getJob() & 0x17) //Second tier jobs
		defense += 25;

	defense += static_cast<WORD>((this->attributes.getStrengthTotal() + 5) * 0.35);
	defense += static_cast<WORD>((this->getLevel() + 15) * 0.7);

	defense += this->checkClothesForStats(1, StatType::DEFENSE_PHYSICAL);
	defense += this->getBuffAmount( Buffs::Visuality::DEFENSE_UP );
	defense -= this->getBuffAmount( Buffs::Visuality::DEFENSE_DOWN );
	for(unsigned int i=Inventory::FACE;i<Inventory::SHIELD;i++) {
		if(i == Inventory::WEAPON || !this->inventory[i].isValid())
			continue;
		try {
			STBEntry& armorEntry = mainServer->getEquipmentEntry(this->inventory[i].type, this->inventory[i].id);
			defense += armorEntry.getColumn<WORD>(EquipmentSTB::DEFENSE_PHYISCAL);
		} catch( std::exception& ex) {
			std::cout << ex.what() << "\n";
		}
	}
	this->stats.defensePhysical = defense;
}

void Player::updateMagicDefense() {
	this->stats.defenseMagical = 100;
}

void Player::updateHitrate() {
	WORD newHitrate = 0x00;
	if(this->isWeaponEquipped()) {
		WORD statPart = static_cast<WORD>((this->attributes.getConcentrationTotal()+10)*0.8f);
		WORD qualityPart = static_cast<WORD>(mainServer->getQuality(ItemType::WEAPON, this->inventory[Inventory::WEAPON].id) * 0.6);
		WORD durabilityPart = static_cast<WORD>(this->inventory[Inventory::WEAPON].durability * 0.8);

		newHitrate = statPart + qualityPart + durabilityPart;
	} else {
		newHitrate = static_cast<WORD>(((this->attributes.getConcentrationTotal() + 10)*0.5) + 15);
	}
	newHitrate += this->checkClothesForStats(1, StatType::HIT_RATE);
	newHitrate += this->getBuffAmount( Buffs::Visuality::HITRATE_UP );
	newHitrate -= this->getBuffAmount( Buffs::Visuality::HITRATE_DOWN );

	this->stats.hitRate = newHitrate;
}

void Player::updateMaxHP() {
	float multiplier = 2.36f;
	int additionPart1 = 26;
	int additionPart2 = 4;
	switch( this->getJob() ) {
		case JobType::SOLDIER:
			multiplier = 3.5f;
			additionPart1 = 20; additionPart2 = 5;
		break;
		case JobType::KNIGHT:
			multiplier = 3.5f;
			additionPart1 = 28; additionPart2 = 5;
		break;
		case JobType::CHAMPION:
			multiplier = 3.5f;
			additionPart1 = 22; additionPart2 = 5;
		break;

		case JobType::MUSE:
			multiplier = 2.36f;
			additionPart1 = 26; additionPart2 = 4;
		break;
		case JobType::MAGE:
			multiplier = 2.37f;
			additionPart1 = 26; additionPart2 = 5;
		break;
		case JobType::CLERIC:
			multiplier = 2.4f;
			additionPart1 = 26; additionPart2 = 7;
		break;

		case JobType::HAWKER:
			multiplier = 2.7f;
			additionPart1 = 20; additionPart2 = 5;
		break;
		case JobType::RAIDER:
			multiplier = 3.0f;
			additionPart1 = 23; additionPart2 = 5;
		break;
		case JobType::SCOUT:
			multiplier = 2.7f;
			additionPart1 = 21; additionPart2 = 5;
		break;
		
		case JobType::DEALER:
		case JobType::ARTISAN:
		case JobType::BOURGEOIS:
			multiplier = 2.7f;
			additionPart1 = 20; additionPart2 = 5;
		break;
	}
	WORD maxHp = static_cast<WORD>(::sqrt(static_cast<double>(additionPart1 + this->getLevel())) * (additionPart2 + this->getLevel()) * multiplier);
	maxHp += ( this->attributes.getStrengthTotal() << 1 );

	WORD additionalMaxHP = 0x00;
	if(this->getJob() & 0x17) { //Second Jobs: X2Y = X ClassType, Y = SubClass (e.g. Scout)
		additionalMaxHP = 300;
	}
	additionalMaxHP += this->checkClothesForStats(1, StatType::MAX_HP);
	additionalMaxHP += this->getBuffAmount( Buffs::Visuality::HP_UP);
	maxHp += additionalMaxHP;

	this->stats.maxHP = maxHp;
}	

void Player::updateDodgerate() {
	this->stats.dodgeRate = 100;
}

void Player::updateCritrate() {
	this->stats.critRate = 10;
}

void Player::updateMovementSpeed() {
	this->stats.movementSpeed = 425;
}

float Player::getAttackRange() {
	if(this->inventory[Inventory::WEAPON].amount == 0x00)
		return 100.0f;
	STBEntry& weapon = mainServer->getEquipmentEntry(ItemType::WEAPON, this->inventory[Inventory::WEAPON].id);
	return std::max(100.0f, weapon.getColumn<float>(EquipmentSTB::ATTACK_RANGE));
}

DWORD Player::getExperienceForLevelup() {
	DWORD result = 0x00;
	if(this->getLevel() <= 15) {
		result = static_cast<DWORD>( (this->getLevel() + 10 ) * (this->getLevel() + 5) * (this->getLevel() + 3) * 0.7 );
	} else if(this->getLevel() <= 50) {
		result = static_cast<DWORD>( (this->getLevel() - 5 ) * (this->getLevel() + 2 ) * (this->getLevel() + 2 ) * 2.2 );
	} else if(this->getLevel() <= 100 ) {
		result = static_cast<DWORD>( (this->getLevel() - 38 ) * (this->getLevel() - 5 ) * (this->getLevel() + 2 ) * 9 );
	} else if(this->getLevel() <= 139 ) {
		result = static_cast<DWORD>( (this->getLevel() + 220 ) * (this->getLevel() + 34 ) * (this->getLevel() + 22 ) );
	} else {
		result = static_cast<DWORD>( (this->getLevel() - 126 ) * (this->getLevel() - 15 ) * (this->getLevel() + 7 ) * 41 ); 
	}
	return result;
}

void Player::addExperience(const DWORD additionalExp) {
	this->charInfo.experience += additionalExp;

	Packet pak(PacketID::World::Response::UPDATE_EXPERIENCE);
	pak.addDWord( this->charInfo.experience );
	pak.addWord( this->stats.stamina );
	pak.addWord( 0x00 ); //DWORD Stamina?
	if(!this->sendData(pak))
		return;

	//Level-Up occured
	while(this->charInfo.experience >= this->getExperienceForLevelup()) {
		this->charInfo.experience -= this->getExperienceForLevelup();
		this->charInfo.level++;
		this->charInfo.statPoints += (this->getLevel() * 10 / 8) + 10;
		if(this->getLevel() == 10 || this->getLevel() == 14)
			this->charInfo.skillPoints += 2;
		else if(this->getLevel() == 18)
			this->charInfo.skillPoints += 3;		
		else if(this->getLevel() == 22)
			this->charInfo.skillPoints += 4;
		else if(this->getLevel() == 22 && ((this->getLevel() - 22) % 4) == 0)
			this->charInfo.skillPoints += 5;
		else if(this->getLevel() >= 100 && (this->getLevel() % 2) == 0)
			this->charInfo.skillPoints += 5;

		this->updateStats();
		this->stats.curHP = this->getMaxHPW();

		pak.newPacket( PacketID::World::Response::LEVEL_UP );
		pak.addWord( this->getClientId() );
		pak.addWord( this->getLevel() );
		pak.addDWord( this->getExperience() );
		pak.addWord( this->charInfo.statPoints );
		pak.addWord( this->charInfo.skillPoints );
		if(!this->sendData(pak))
			return;

		pak.newPacket( PacketID::World::Response::LEVEL_UP );
		pak.addWord( this->getClientId() );
		if(!this->sendToVisible( pak, this ))
			return;
	}
}

bool Player::pakExit() {
	Packet pak(PacketID::World::Response::EXIT);
	pak.addWord(0x00);
	this->sendData(pak);
	this->setIsActive(false);
	return true;
}

bool Player::pakPlayerInfos() {
	mainServer->changeToMap(this, this->getMapId());
	
	Packet pak(PacketID::World::Response::PLAYER_INFOS);
	pak.addByte(this->getVisualTraits().sex);
	pak.addWord(this->entityInfo.mapId);
	pak.addFloat(this->position.current.x);
	pak.addFloat(this->position.current.y);
	pak.addWord(0x00); //SAVED POSITION
	pak.addDWord(this->getVisualTraits().faceStyle);
	pak.addDWord(this->getVisualTraits().hairStyle);

	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::HEADGEAR]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::ARMOR]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::GLOVES]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::SHOES]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::FACE]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::BACK]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::WEAPON]));
	pak.addDWord(mainServer->buildItemVisually(this->inventory[Inventory::SHIELD]));

	pak.addByte(0x00); //BIRTHSTONE
	pak.addWord(0x00); //BIRTHPLACE?
	pak.addWord(this->getJob());
	pak.addByte(0x00); //UNION
	pak.addByte(0x00); //RANK
	pak.addByte(0x00); //FAME

	pak.addWord(this->attributes.strength);
	pak.addWord(this->attributes.dexterity);
	pak.addWord(this->attributes.intelligence);
	pak.addWord(this->attributes.concentration);
	pak.addWord(this->attributes.charm);
	pak.addWord(this->attributes.sensibility);
	pak.addWord(this->stats.curHP);
	pak.addWord(this->stats.curMP);

	pak.addDWord(this->charInfo.experience);
	pak.addWord(this->charInfo.level);
	pak.addWord(this->charInfo.statPoints);
	pak.addWord(this->charInfo.skillPoints);
	pak.addByte(0x64); //BodySize
	pak.addByte(0x64); //HeadSize
	pak.addDWord(0x00);
	pak.addDWord(0x00);

	//UNION POINTS for each fraction
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	pak.addWord(0x00);
	//END: UNION POINTS for each fraction

	pak.addDWord(0x00);
	pak.addDWord(0x00);
	pak.addByte(0x00);
	pak.addWord(this->stats.stamina);
	for (short i = 0; i < 0x146; i++) {
		pak.addByte(0x00); //?
	}
	
	//Capacity = 120
	for(unsigned int i=0;i<this->skills.capacity();i++) {
		pak.addWord( this->skills[i] != nullptr ? this->skills[i]->getId() : 0x00 );
	}
	/*
	for (unsigned int i = 0; i < 30; i++) {
		pak.addWord(this->basicSkills[i].id); //BASIC SKILLS
	}
	for (unsigned int i = 0; i < 30; i++) {
		pak.addWord(0x00); //ACTIVE SKILLS
	}
	for (unsigned int i = 0; i < 30; i++) {
		pak.addWord(0x00); //PASSIVE SKILLS
	}
	for (unsigned int i = 0; i < 30; i++) {
		pak.addWord(0x00); //NOT USED
	}
	*/
	for (unsigned int i = 0; i < 32; i++) {
		pak.addWord(0x00); //QUICK BAR
	}

	pak.addDWord(this->charInfo.id);
	pak.addString(this->charInfo.name.c_str());
	pak.addWord(0x00);
	return this->sendData(pak);
}

bool Player::pakInventory() {
	Packet pak(PacketID::World::Response::PLAYER_INVENTORY);
	pak.addQWord(this->inventory[0x00].amount);

	pak.addWord( 0x00 );
	pak.addDWord(0x00);
	for (unsigned int i = 1; i < Inventory::MAXIMUM; i++) {
		pak.addWord(mainServer->buildItemHead(this->inventory[i]));
		pak.addDWord(mainServer->buildItemData(this->inventory[i]));
	}
	return this->sendData(pak);
}

bool Player::pakQuestData() {
	Packet pak(PacketID::World::Response::QUEST_DATA);
	
	for (unsigned int i = 0; i < 5; i++) //EPISODE
		pak.addWord(0x00);

	for (unsigned int i = 0; i < 3; i++) //JOB
		pak.addWord(0x00);

	for (unsigned int i = 0; i < 7; i++) //PLANET
		pak.addWord(0x00);

	for (unsigned int i = 0; i < 10; i++) //UNION
		pak.addWord(0x00);

	for (unsigned int i = 0; i < 10; i++) {
		pak.addWord(0x00); //QuestID
		pak.addDWord(0x00); //QuestTime

		for (unsigned int j = 0; j < 10; j++) 
			pak.addWord(0x00); //VARS
		
		pak.addDWord(0x00); //SWITCHES
		for (unsigned int j = 0; j < 6; j++) {
			pak.addWord(0x00); //ITEM HEAD
			pak.addDWord(0x00); //ITEM DATA
		}
	}
	for (unsigned int i = 0; i < 0x40; i++)
		pak.addByte(0x00);

	for (unsigned int i = 0; i < 30; i++) {
		pak.addWord(0x00); //ITEM HEAD
		pak.addDWord(0x00);
	}
	
	return this->sendData(pak);
}

bool Player::saveInfos() {
	Map *map = mainServer->getMap(this->getMapId());
	WORD respawnId = 0x00;
	if(map) 
		respawnId = map->getRespawnPointId(this->getPositionCurrent());
	if(!mainServer->sqlInsert("UPDATE characters SET level=%i, experience=%i, job=%i, zulies=%i, statPoints=%i, skillPoints=%i, respawnMap=%i, respawnId=%i WHERE id=%i", this->getLevel(), this->getExperience(), this->getJob(), this->inventory[0x00].amount, this->charInfo.statPoints, this->charInfo.skillPoints, this->getMapId(), respawnId, this->charInfo.id))
		return false;
	if(!mainServer->sqlInsert("UPDATE character_stats SET strength=%i, dexterity=%i, intelligence=%i, concentration=%i, charm=%i, sensibility=%i WHERE id=%i", this->attributes.strength, this->attributes.dexterity, this->attributes.intelligence, this->attributes.concentration, this->attributes.charm, this->attributes.sensibility, this->charInfo.id))
		return false;
	if(!mainServer->sqlInsert("DELETE FROM inventory WHERE charId=%i", this->charInfo.id))
		return false;
	for(unsigned int i=1;i<Inventory::MAXIMUM;i++) {
		if(this->inventory[i].isValid())
			mainServer->sqlInsert("INSERT INTO inventory (charId, slot, itemId, durability, lifespan, count, refine) VALUES(%i, %i, %i, %i, %i, %i, %i)", this->charInfo.id, i, this->inventory[i].type * 10000 + this->inventory[i].id, this->inventory[i].durability, this->inventory[i].lifespan, this->inventory[i].amount, this->inventory[i].refine);
	}
	return true;
}

bool Player::loadInfos() {
	if (!mainServer->sqlRequest("SELECT lastChar_Id, accesslevel FROM accounts WHERE id=%i", this->accountInfo.id)) {
		return false;
	}
	MYSQL_ROW row = mainServer->sqlGetNextRow();
	this->charInfo.id = atoi(row[0]);
	this->accountInfo.accessLevel = static_cast<BYTE>(atoi(row[1]));
	mainServer->sqlFinishQuery();

	if (!mainServer->sqlRequest("SELECT name, level, experience, job, face, hair, sex, zulies, statPoints, skillPoints, respawnMap, respawnId FROM characters WHERE id=%i", this->charInfo.id))
		return false;

	row = mainServer->sqlGetNextRow();
	this->charInfo.name = std::string(row[0]);
	this->charInfo.level = atoi(row[1]);
	this->charInfo.experience = atol(row[2]);
	this->charInfo.job = atoi(row[3]);
	this->charInfo.visualTraits.faceStyle = atoi(row[4]);
	this->charInfo.visualTraits.hairStyle = atoi(row[5]);
	this->charInfo.visualTraits.sex = atoi(row[6]);
	this->inventory[0x00].type = ItemType::MONEY;
	this->inventory[0x00].amount = static_cast<QWORD>(::atol(row[7]));
	this->charInfo.statPoints = atoi(row[8]);
	this->charInfo.skillPoints = atoi(row[9]);
	this->entityInfo.mapId = atoi(row[10]);
	this->setPositionCurrent(mainServer->getMap(this->getMapId())->getRespawnPoint(atoi(row[11])));
	this->position.destination = this->position.current;

	mainServer->sqlFinishQuery();
	
	if (!mainServer->sqlRequest("SELECT * FROM character_stats WHERE id=%i", this->charInfo.id))
		return false;
	row = mainServer->sqlGetNextRow();
	this->attributes.strength = atoi(row[1]);
	this->attributes.dexterity = atoi(row[2]);
	this->attributes.intelligence = atoi(row[3]);
	this->attributes.concentration = atoi(row[4]);
	this->attributes.charm = atoi(row[5]);
	this->attributes.sensibility = atoi(row[6]);

	mainServer->sqlFinishQuery();
	
	
	if(!mainServer->sqlRequest("SELECT * FROM inventory WHERE charId=%i", this->charInfo.id))
		return false;
	for(unsigned int i=0;i<mainServer->sqlGetRowCount();i++) {
		row = mainServer->sqlGetNextRow();
		BYTE slotId = atoi(row[1]);
		this->inventory[slotId].type = atoi(row[2]) / 10000;
		this->inventory[slotId].id = atoi(row[2]) % 10000;
		this->inventory[slotId].durability = atoi(row[3]);
		this->inventory[slotId].lifespan = atoi(row[4]);
		this->inventory[slotId].amount = atoi(row[5]);
		this->inventory[slotId].refine = atoi(row[6]);
	}
	mainServer->sqlFinishQuery();
	
	if (!mainServer->sqlRequest("SELECT basicSkills FROM character_skills WHERE id=%i", this->charInfo.id))
		return false;
	if (mainServer->sqlGetRowCount() != 1)
		return false;
	std::string str = std::string(mainServer->sqlGetNextRow()[0]);
	DWORD idx = 0x00;
	WORD skillId = 0x00;
	while (str.find(",") != -1) {
		skillId = atoi(str.substr(0, str.find(",")).c_str());
		this->skills[PlayerSkill::BASIC_BEGIN + idx] = mainServer->getSkill(skillId);
		idx++;

		//min_value of str.find is 0 -> no check necessary
		str = str.substr(str.find(",")+1);
	}
	this->skills[PlayerSkill::BASIC_BEGIN + idx] = mainServer->getSkill(atoi(str.c_str()));

	this->updateStats();

	mainServer->sqlFinishQuery();

	return true;
}

bool Player::pakRespawnAfterDeath() {
	Map* curMap = mainServer->getMap(this->getMapId());

	this->stats.curHP = this->getMaxHPW() * 10 / 100;
	this->status.buffs.clearBuff();

	BYTE respawnType = this->packet.getByte(0x00);
	if(respawnType == 0x01) {
		const Position& pos = curMap->getRespawnPoint(this->getPositionCurrent());
		return this->pakTelegate(this->getMapId(), pos);
	} else {
		//TODO
	}
	return true;
}

bool Player::pakUpdateInventoryVisually( const BYTE slotAmount, const BYTE* slotIds ) {
	Packet pak(PacketID::World::Response::UPDATE_INVENTORY);
	pak.addByte( slotAmount );
	for(unsigned int i=0;i<slotAmount;i++) {
		pak.addByte( slotIds[i] );
		if(this->inventory[ slotIds[i] ].amount == 0)
			this->inventory[ slotIds[i] ].clear();
		pak.addWord( mainServer->buildItemHead( this->inventory[ slotIds[i] ] ) );
		pak.addDWord( mainServer->buildItemData( this->inventory[ slotIds[i] ] ) );
	}
	bool result = this->sendData(pak);

	for(unsigned int j=0;j<slotAmount;j++) {
		if(slotIds[j] == 0 || slotIds[j] >= Inventory::SHIELD)
			continue;
		Packet pak( PacketID::World::Response::EQUIPMENT_CHANGE);
		pak.addWord( this->getClientId() );
		pak.addWord( slotIds[j] );
		pak.addDWord( mainServer->buildItemVisually( this->inventory[slotIds[j]] ) );
		pak.addWord( this->getMovementSpeed() );
		this->sendToVisible( pak );
	}
	return result;
}

bool Player::equipItem(const Item& item) {
	BYTE slotId = Inventory::fromItemType(item.type);
	this->inventory[slotId] = item;
	this->updateStats();
	return this->pakUpdateInventoryVisually(1, &slotId); 
}

bool Player::pakPing() {
	Packet pak(PacketID::World::Response::PING);
	pak.addByte(0x00);
	return this->sendData(pak);
}

bool Player::pakIdentify() {
	this->accountInfo.id = this->packet.getDWord(0x00);

	Packet pak(PacketID::World::Response::IDENFITY);
	pak.addByte(0x00); //??
	pak.addDWord(0x87654321); //Encryption
	pak.addDWord(0x00);
	if (!this->sendData(pak))
		return false;

	if (!this->loadInfos())
		return false;

	this->pakPlayerInfos();
	this->pakInventory();
	this->pakQuestData();

	pak.newPacket(PacketID::World::Response::GAMING_PLAN);
	pak.addWord(0x1001); //?
	pak.addDWord(0x02); //Unlimited plan
	return this->sendData(pak);
}

bool Player::pakReturnToCharServer() {
	Packet pak(PacketID::World::Response::EXIT);
	pak.addWord(0x00);
	if(!this->sendData(pak))
		return false;

	pak.newPacket(PacketID::World::Response::MESSAGE_MANAGER);
	pak.addByte(0xFA);
	pak.addDWord(this->accountInfo.id);
	if(!this->sendData(pak))
		return false;

	this->setIsActive(false);
	return true;
}

bool Player::pakAssignID(){
	//????
	this->updateStats();
	mainServer->assignClientID(this);

	Packet pak(PacketID::World::Response::UNKNOWN);
	pak.addWord(0x22);
	pak.addWord(0x02);
	pak.addWord(0x00);
	if(!this->sendData(pak))
		return false;

	pak.newPacket(PacketID::World::Response::ASSIGN_ID);
	pak.addWord(this->entityInfo.id);
	pak.addWord(this->stats.curHP);
	pak.addWord(this->stats.curMP);
	pak.addDWord(this->charInfo.experience);
	pak.addDWord(0x00); //UNUSED

	//WORLD RATES
	pak.addWord(0x64);
	pak.addDWord(0x0C1F4B79);
	pak.addWord(0x64);
	pak.addDWord(0x3232cd50);
	pak.addDWord(0x32323235);
	pak.addDWord(0x35323232);

	//PVP-MAP (0 = false, 1 = true)
	pak.addDWord(0x00);

	//MAP TIME
	pak.addDWord(mainServer->getMapTime(this->getMapId()));

	//White icon (friendly)
	pak.addWord(0x02);
	pak.addWord(0x00);

	if (!this->sendData(pak))
		return false;

	pak.newPacket(PacketID::World::Response::WEIGHT);
	pak.addWord(this->entityInfo.id);
	pak.addByte(0x00); //WEIGHT PERCENT
	if (!this->sendData(pak))
		return false;

	pak.newPacket(PacketID::World::Response::CHANGE_STANCE);
	pak.addWord(this->entityInfo.id);
	pak.addByte(this->getStance().asBYTE());
	pak.addWord(this->getMovementSpeed());

	this->position.lastCheckTime = clock();
	this->status.updateLastRegen();
	this->entityInfo.ingame = true;
	bool result = this->sendToVisible(pak);

	this->visibleSectors.clear();
	this->checkVisuality();
	return result;
}

bool Player::pakTerrainCollision() {
	this->position.destination = Position( this->packet.getFloat(0x00), this->packet.getFloat(0x04) );
	this->position.current = this->position.destination;
	Packet pak(PacketID::World::Response::TERRAIN_COLLISION);
	pak.addWord(this->entityInfo.id);
	pak.addFloat( this->packet.getFloat(0x00) );
	pak.addFloat( this->packet.getFloat(0x04) );
	pak.addWord( this->packet.getWord(0x0A) );
	return this->sendToVisible(pak);
}

bool Player::pakSetEmotion() {
	Packet pak(PacketID::World::Response::SET_EMOTION);
	pak.addDWord( this->packet.getDWord(0x00) );
	pak.addWord( this->entityInfo.id );
	return this->sendToVisible( pak );
}

bool Player::pakMoveCharacter() {
	this->combat.setTarget( mainServer->getEntity( this->packet.getWord(0x00) ) );
	this->setPositionDest( Position(this->packet.getFloat(0x02), this->packet.getFloat(0x06)) );
	return true;
}

bool Player::pakIncreaseAttribute() {
	BYTE statType = this->packet.getByte(0x00);
	WORD* statPointsREF = nullptr;
	switch(statType) {
		case 0x00:	
			statPointsREF = &this->attributes.strength;
		break;
		case 0x01:	
			statPointsREF = &this->attributes.dexterity;
		break;
		case 0x02:	
			statPointsREF = &this->attributes.intelligence;
		break;
		case 0x03:	
			statPointsREF = &this->attributes.concentration;
		break;
		case 0x04:	
			statPointsREF = &this->attributes.charm;
		break;
		case 0x05:	
			statPointsREF = &this->attributes.sensibility;
		break;
	}
	if(statPointsREF && (*statPointsREF)/5 <= this->charInfo.statPoints) {
		this->charInfo.statPoints -= (*statPointsREF) / 5;
		(*statPointsREF)++;

		this->updateStats();

		Packet pak( PacketID::World::Response::INCREASE_ATTRIBUTE );
		pak.addByte( statType );
		pak.addWord( (*statPointsREF) );
		return this->sendData(pak);
	}
	return true;
}

bool Player::pakChangeStance() {
	BYTE stanceType = this->packet.getByte(0x00);
	switch(stanceType) {
		case 0x00:
			if(this->getStance().asBYTE() == Stance::WALKING) {
				this->setStance(Stance::RUNNING);
			} else if(this->getStance().asBYTE() == Stance::RUNNING) {
				this->setStance(Stance::WALKING);
			}
		break;
		case 0x01:
			if(this->getStance().asBYTE() == Stance::SITTING) {
				this->setStance(Stance::RUNNING);
				break;
			}
			if(this->getStance().asBYTE() != Stance::DRIVING && this->getStance().asBYTE() != Stance::SITTING) {
				this->setStance(Stance::SITTING);
			}
		break;
		case 0x02:
			//DRIVING
		break;
	}
	stanceType = this->getStance().asBYTE();

	Packet pak(PacketID::World::Response::CHANGE_STANCE);
	pak.addWord( this->getClientId() );
	pak.addByte( stanceType );
	pak.addWord( this->getMovementSpeed() );
	return this->sendData(pak);
}

bool Player::pakLocalChat() {
	ChatService::sendMessage(this, this->packet.getString(0x00));
	return true;
}

bool Player::pakShoutChat() {
	ChatService::sendShout(this, this->packet.getString(0x00), nullptr);
	return true;
}

bool Player::pakSpawnPlayer(Player* player) {
	Packet pak(PacketID::World::Response::SPAWN_PLAYER);
	pak.addWord( player->getClientId() );
	pak.addFloat( player->getCurrentX() );
	pak.addFloat( player->getCurrentY() );
	pak.addFloat( player->getDestinationX() );
	pak.addFloat( player->getDestinationY() );
	if(player->getCurrentHP() == 0x00) {
		pak.addWord( EntitySpawnsVisually::IS_DEAD );
	} else if(player->getPositionCurrent() != player->getPositionDest()) {
		pak.addWord( EntitySpawnsVisually::IS_MOVING );
	} else if(player->getTarget() != nullptr) {
		pak.addWord( EntitySpawnsVisually::IS_ATTACKING );
	} else {
		pak.addWord( EntitySpawnsVisually::IS_STANDING );
	}
	pak.addWord( (player->getTarget() == nullptr ? 0x00 : player->getTarget()->getClientId() ) );
	switch(player->getStance().asBYTE()) {
		case Stance::WALKING:
			pak.addByte( EntitySpawnsVisually::STANCE_WALKING );
		break;
		case Stance::RUNNING:
			pak.addByte( EntitySpawnsVisually::STANCE_RUNNING );
		break;
		case Stance::DRIVING:
			pak.addByte( EntitySpawnsVisually::STANCE_DRIVING );
		break;
		default:
			pak.addByte( EntitySpawnsVisually::STANCE_HITCHHIKER );
	}
	pak.addDWord( player->getBuffsVisuality() );
	BYTE pvpMapType = 0x00; //mainServer->isPVPMap(this->getMapId())
	if(pvpMapType) {
		//TODO:
		pak.addWord( EntitySpawnsVisually::IS_FRIENDLY );
	} else {
		pak.addWord( EntitySpawnsVisually::IS_FRIENDLY );
	}
	pak.addWord(0x00);
	pak.addDWord(0x00);
	pak.addByte( player->getVisualTraits().sex );
	pak.addWord( player->getMovementSpeed() );
	pak.addWord( player->getAttackSpeed() );
	pak.addByte( 0x01 ); //Weight?

	pak.addDWord( player->getVisualTraits().faceStyle );
	pak.addDWord( player->getVisualTraits().hairStyle );
	
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::HEADGEAR] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::ARMOR] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::GLOVES] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::SHOES] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::FACE] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::BACK] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::WEAPON] ) );
	pak.addDWord( mainServer->buildItemVisually( player->inventory[Inventory::SHIELD] ) );
	
	for(unsigned int i=Inventory::ARROWS;i<=Inventory::CANNONSHELLS;i++)
		pak.addWord( mainServer->buildItemHead( player->inventory[i] ) );

	pak.addWord( player->getJob() );
	pak.addByte( player->getLevel() );

	for(unsigned int i=Inventory::CART_FRAME;i<=Inventory::CART_ABILITY;i++)
		pak.addDWord( player->inventory[i].id | 0x400 );

	pak.addWord( 0xcdcd ); //Z-Coord?
	pak.addDWord( 0x00 ); //weird DWORD, additional buffs/status?

	pak.addString( player->getName().c_str() );
	pak.addByte(0x00);
	//TODO: IF player has clan
	/*
		if(player->getClan()) {
			pak.addDWord( player->getClan()->getId() );
			pak.addWord( player->getClan()->getBackground() );
			pak.addWord( player->getClan()->getLogo() );
			pak.addByte( player->getClan()->getGrade() );
			pak.addByte( player->getClan()->getRank(player) );
			pak.addString( player->getClan()->getName().c_str() );
			pak.addByte( 0x00 );
		}
	*/

	return player->sendData(pak);
}

bool Player::pakSpawnNPC( NPC* npc ) {
	Packet pak(PacketID::World::Response::SPAWN_NPC);
	pak.addWord(npc->getClientId());
	pak.addFloat(npc->getCurrentX());
	pak.addFloat(npc->getCurrentY());
	pak.addFloat(npc->getDestinationX()); 
	pak.addFloat(npc->getDestinationY()); 
	if(npc->getCurrentHP() == 0x00) {
		pak.addWord( EntitySpawnsVisually::IS_DEAD );
	} else if(npc->getPositionCurrent() != npc->getPositionDest()) {
		pak.addWord( EntitySpawnsVisually::IS_MOVING );
	} else if(npc->getTarget() != nullptr) {
		pak.addWord( EntitySpawnsVisually::IS_ATTACKING );
	} else {
		pak.addWord( EntitySpawnsVisually::IS_STANDING );
	}
	pak.addWord( npc->getTarget() != nullptr ? npc->getTarget()->getClientId() : 0x00 );
	pak.addByte( 0x00 ); //Stance
	pak.addDWord( npc->getCurrentHP() );
	pak.addWord(0x01);
	pak.addWord(0x00);
	pak.addDWord( npc->getBuffsVisuality() );
	pak.addWord(npc->getTypeId());
	if(npc->hasDialogId())
		pak.addWord(npc->getDialogId());
	else
		pak.addWord(npc->getTypeId() - 900);
	
	pak.addFloat(npc->getDirection());
	pak.addByte(0x00); //CLANFIELD OPEN/CLOSED FOR BURLAND (NPC: 1115)
	pak.addByte(0x00);
	
	return this->sendData(pak);
}

bool Player::pakSpawnMonster(Monster* monster) {
	Packet pak(PacketID::World::Response::SPAWN_MONSTER);
	pak.addWord(monster->getClientId());
	pak.addFloat(monster->getCurrentX());
	pak.addFloat(monster->getCurrentY());
	pak.addFloat(monster->getDestinationX());
	pak.addFloat(monster->getDestinationY());
	if(monster->getCurrentHP() == 0x00) {
		pak.addWord( EntitySpawnsVisually::IS_DEAD );
	} else if(monster->getPositionCurrent() != monster->getPositionDest()) {
		pak.addWord( EntitySpawnsVisually::IS_MOVING );
	} else if(monster->getTarget() != nullptr) {
		pak.addWord( EntitySpawnsVisually::IS_ATTACKING );
	} else {
		pak.addWord( EntitySpawnsVisually::IS_STANDING );
	}
	pak.addWord( monster->getTarget() != nullptr ? monster->getTarget()->getClientId() : 0x00 );
	pak.addByte( monster->getStance().asBYTE() );
	pak.addDWord(monster->getCurrentHP());
	pak.addDWord(0x64); //ENEMY
	pak.addDWord( monster->getBuffsVisuality() );
	pak.addWord(monster->getTypeId());
	pak.addWord(0x00); //Quest?
	return this->sendData(pak);
}

bool Player::pakShowMonsterHP(Monster* mon) {
	Packet pak(PacketID::World::Response::SHOW_MONSTER_HP);
	pak.addWord(mon->getClientId());
	pak.addDWord(mon->getCurrentHP());
	return this->sendData(pak);
}

bool Player::pakInitBasicAttack() {
	WORD clientId = this->packet.getWord(0x00);

	//In case we target ourselves, do nothing
	if(this->getClientId() == clientId)
		return true; 

	Entity* entity = mainServer->getEntity(clientId);
	if(!entity)
		return true; //TODO

	if(entity->getEntityType() == Entity::TYPE_MONSTER) {
		bool tmpResult = this->pakShowMonsterHP(dynamic_cast<Monster*>(entity));
		if(!tmpResult)
			return false;
	}
	this->setTarget(entity);
	return true;
}

bool Player::pakSpawnDrop( Drop* drop ) {
	Packet pak(PacketID::World::Response::SPAWN_DROP);
	pak.addFloat(drop->getCurrentX());
	pak.addFloat(drop->getCurrentY());
	pak.addWord(mainServer->buildItemHead(drop->getItem()) );
	pak.addDWord(drop->getItem().amount);
	pak.addWord(drop->getClientId());
	pak.addDWord( mainServer->buildItemData(drop->getItem()) );
	//pak.addWord( (drop->getOwner() == nullptr ? 0x00 : drop->getOwner()->getClientId()) ); //OwnerClientId (?)
	return this->sendData(pak); 
}

bool Player::pakEquipmentChange() {
	BYTE sourceSlot = static_cast<BYTE>(this->packet.getWord(0x00));
	BYTE destSlot = static_cast<BYTE>(this->packet.getWord(0x02));
	if(destSlot == 0x00) {
		destSlot = this->findSlot( this->inventory[sourceSlot] );
	}
	if(destSlot == std::numeric_limits<BYTE>::max())
		return true;

	Item tmpItem = this->inventory[sourceSlot];
	this->inventory[sourceSlot] = this->inventory[destSlot];
	this->inventory[destSlot] = tmpItem;

	this->updateStats();

	this->pakUpdateInventoryVisually( 0x01, &sourceSlot );
	this->pakUpdateInventoryVisually( 0x01, &destSlot );
	return true; //Safe?
}

bool Player::pakDropFromInventory() {
	BYTE slot = this->packet.getByte(0x00);
	if(!this->inventory[slot].isValid())
		return false;

	DWORD amount = this->packet.getDWord(0x01);
	if(amount > this->inventory[slot].amount)
		return true;

	Item toDrop = this->inventory[slot];
	toDrop.amount = amount;

	new Drop(this, toDrop, true);

	this->inventory[slot].amount -= amount;
	return this->pakUpdateInventoryVisually( 1, &slot );
}

bool Player::pakPickUpDrop() {
	this->setTarget(nullptr);

	WORD dropId = this->packet.getWord(0x00);
	Entity *entityDrop = mainServer->getEntity(dropId);
	if(!entityDrop || entityDrop->getEntityType() != Entity::TYPE_DROP)
		return true;
	Drop* drop = dynamic_cast<Drop*>(entityDrop);
	if(!drop)
		return false;
	
	Packet pak(PacketID::World::Response::PICK_DROP);
	pak.addWord( drop->getClientId() );

	//In case the drop is not publicly available for everyone
	if(drop->getOwner() != nullptr && drop->getOwner() != this) {
		pak.addWord( PickDropMessage::NOT_OWNER );
		return this->sendData(pak);
	}
	//In case we're the owner, find a fitting inventory slot
	WORD inventorySlotId = 0x00;
	if(!drop->isZulyDrop()) {
		inventorySlotId = this->findSlot( drop->getItem() );

		//In case we don't have a free (suiting) slot, tell the client
		if(inventorySlotId == std::numeric_limits<BYTE>::max()) {
			pak.addByte( PickDropMessage::INVENTORY_FULL );
			return this->sendData(pak);
		}
	}
	DWORD previousAmount = this->inventory[inventorySlotId].amount;
	this->inventory[inventorySlotId] = drop->getItem();
	this->inventory[inventorySlotId].amount += previousAmount;
	
	pak.addByte( PickDropMessage::OKAY );
	pak.addWord( inventorySlotId ); //SlotId
	pak.addWord( mainServer->buildItemHead(this->inventory[inventorySlotId]) );
	if(drop->isZulyDrop())
		pak.addDWord( drop->getItem().amount );
	else
		pak.addDWord( mainServer->buildItemData(this->inventory[inventorySlotId]) );
	
	delete drop;
	drop = nullptr;

	return this->sendData(pak); 

}

bool Player::pakBuyFromNPC() {
	WORD npcType = this->packet.getWord(0x00);
	NPC* npc = mainServer->getMap(this->getMapId())->getNPC(npcType);
	if(!npc)
		return false;

	return true;
}

bool Player::pakSellToNPC() {
	return true;
}

bool Player::pakTelegate() {
	WORD telegateId = this->packet.getWord(0x00);
	Telegate& gate = mainServer->getGate(telegateId);

	float distanceToGate = this->getPositionCurrent().distanceTo(gate.getSourcePosition());

	this->setPositionCurrent(gate.getDestPosition());
	return this->pakTelegate(gate.getDestMap(), gate.getDestPosition());
}

bool Player::pakTelegate(const WORD mapId, const Position& pos) {
	for(unsigned int i=0;i<this->visibleSectors.size();i++) {
		this->removeSectorVisually(this->visibleSectors.getValue(i));
	}
	this->visibleSectors.clear();

	Packet pak(PacketID::World::Response::TELEGATE);
	pak.addWord(this->getClientId());
	pak.addWord(mapId);
	pak.addFloat(pos.x);
	pak.addFloat(pos.y);
	pak.addWord(0x01);
	if(!this->sendData(pak))
		return false;

	this->entityInfo.ingame = false;

	this->setPositionCurrent(pos);
	this->setPositionDest(pos);
	mainServer->changeToMap(this, mapId);

	return true;
}

bool Player::handlePacket() {
	std::cout << "[IN] New Packet: " << std::hex << this->packet.getCommand() << " with Length " << std::dec << this->packet.getLength() << "\n";
	switch (this->packet.getCommand()) {
		case PacketID::World::Request::PING:
			return true;

		case PacketID::World::Request::INIT_BASIC_ATTACK:
			return this->pakInitBasicAttack();

		case PacketID::World::Request::CHANGE_STANCE:
			return this->pakChangeStance();

		case PacketID::World::Request::DROP_FROM_INVENTORY:
			return this->pakDropFromInventory();

		case PacketID::World::Request::EQUIPMENT_CHANGE:
			return this->pakEquipmentChange();

		case PacketID::World::Request::EXIT:
			return this->pakExit(); //DISCONNECT

		case PacketID::World::Request::GET_ID:
			return this->pakAssignID();

		case PacketID::World::Request::IDENFITY:
			return this->pakIdentify();

		case PacketID::World::Request::INCREASE_ATTRIBUTE:
			return this->pakIncreaseAttribute();

		case PacketID::World::Request::LOCAL_CHAT:
			return this->pakLocalChat();

		case PacketID::World::Request::MOVEMENT_PLAYER:
			return this->pakMoveCharacter();

		case PacketID::World::Request::RESPAWN_AFTER_DEATH:
			return this->pakRespawnAfterDeath();

		case PacketID::World::Request::RETURN_TO_CHARSERVER:
			return this->pakReturnToCharServer();

		case PacketID::World::Request::PICK_DROP:
			return this->pakPickUpDrop();

		case PacketID::World::Request::SET_EMOTION:
			return this->pakSetEmotion();

		case PacketID::World::Request::SHOUT_CHAT:
			return this->pakShoutChat();

		case PacketID::World::Request::TELEGATE:
			return this->pakTelegate();

		case PacketID::World::Request::TERRAIN_COLLISION:
			return this->pakTerrainCollision();
	}
	return true;
}