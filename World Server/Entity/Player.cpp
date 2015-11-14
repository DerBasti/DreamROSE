#include "Player.h"
#include "Monster.h"
#include "Drop.h"
#include "..\WorldServer.h"
#include "D:\Programmieren\GlobalLogger\GlobalLogger.h"
#include <stdlib.h>

long PLAYER_ATTACK_INTERVAL = 125000;

Player::UnionInfo::UnionInfo() {
	id = rank = fame = 1;
	points.reserve(UNION_AMOUNT);
	for (int i = 0; i < 10; i++) {
		points.addValue((i+1)*5);
	}
}

Player::Player(SOCKET sock, ServerSocket* server){ 
	this->socket = sock;
	this->serverDelegate = server;
	this->accountInfo.debugMode = false;
	this->entityInfo.type = Entity::TYPE_PLAYER;
	this->entityInfo.ingame = false;
	this->entityInfo.pkFlagId = 0x02;
	this->status.updateLastRegen();

	for (unsigned int i = 0; i < PlayerInventory::Slots::MAXIMUM; i++) {
		this->inventory[i].clear();
	}
	this->skills.reserve(PlayerSkill::PLAYER_MAX_SKILLS);
	for (unsigned int i = 0; i < this->skills.capacity(); i++)
		this->skills.addValue(nullptr);

	this->quickbar.reserve(PlayerSkill::PLAYER_MAX_QUICKBAR);
	for (unsigned int i = 0; i < this->quickbar.capacity(); i++) {
		this->quickbar.addValue(0);
	}

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

		this->quest.journey.addValue(new PlayerQuest(i));
	}
}

Player::~Player() {
	this->saveInfos();
	for(unsigned int i=0;i<this->visibleSectors.size();i++) {
		this->removeSectorVisually(this->visibleSectors.getValueAtPosition(i));
	}
	for (unsigned int i = 0; i < this->quest.journey.capacity(); i++)
		delete this->quest.journey.getValue(i);
}

void Player::setPositionCurrent(const position_t& newPos) {
	this->position.current = newPos;
}

void Player::setPositionDest(const position_t& newPos) {
	this->position.destination = newPos;
	this->setPositionVisually(newPos);
}

bool Player::setPositionVisually(const position_t& newPos) {
	Entity* target = this->combat.getTarget();

	Packet pak(PacketID::World::Response::MOVEMENT_PLAYER);
	pak.addWord(this->getLocalId());
	pak.addWord(target != nullptr ? target->getLocalId() : 0x00);
	pak.addWord( this->getMovementSpeed() );
	pak.addFloat( newPos.x );
	pak.addFloat( newPos.y );
	pak.addWord( 0xcdcd ); //Z
	
	this->position.lastCheckTime.update();

	return this->sendToVisible(pak);
}

const byte_t Player::isSkillLearned(const Skill* skillToFind) {
	for (unsigned int i = 0; i < PlayerSkill::PLAYER_MAX_SKILLS; i++) {
		if (this->skills[i] == nullptr)
			continue;
		if (this->skills[i]->getIdBasic() == skillToFind->getIdBasic()) {
			if (this->skills[i]->getLevel() >= skillToFind->getLevel()) {
				return 2;
			}
			return 1;
		}
	}
	return 0;
}

byte_t Player::getSlotOfLearnedSkill(const word_t skillId) const {
	Skill* skillToFind = mainServer->getSkill(skillId);
	for (unsigned int i = 0; i < PlayerSkill::PLAYER_MAX_SKILLS; i++) {
		if (this->skills[i] == nullptr)
			continue;
		if (this->skills[i]->getIdBasic() == skillToFind->getIdBasic() &&
			this->skills[i]->getLevel() >= skillToFind->getLevel()) {
			return i;
		}
	}
	return std::numeric_limits<byte_t>::max();
}

bool Player::executeSkill(const byte_t skillSlot) {
	Skill* skill = this->skills[skillSlot];
	if (!skill)
		return false;
	return true;
}

const byte_t Player::getFreeInventorySlot(const Item& item) {
	byte_t inventoryTab = 0x00;
	switch(item.type) {
		case ItemType::CONSUMABLES:
			inventoryTab = 0x01;
		break;
		case ItemType::JEWELS:
		case ItemType::OTHER:
			inventoryTab = 0x02;
		break;
		case ItemType::PAT:
			inventoryTab = 0x03;
		break;
		default:
			if(item.type == 0x00 || item.type >= ItemType::PAT)
				return std::numeric_limits<byte_t>::max();
	}	
	byte_t slotId = 12 + (PlayerInventory::Slots::TAB_SIZE * inventoryTab);
	if(inventoryTab == 0x01 || inventoryTab == 0x02) {
		dword_t totalCount = 0x00;
		for (unsigned int i = 0; i<PlayerInventory::Slots::TAB_SIZE; i++) {
			if(this->inventory[slotId].id == item.id && this->inventory[slotId].type == item.type) {
				totalCount = this->inventory[slotId].amount + item.amount;
				if(totalCount < 1000)
					return slotId;
			}
			slotId++;
		}
		slotId -= PlayerInventory::Slots::TAB_SIZE;
		for (unsigned int i = 0; i<PlayerInventory::Slots::TAB_SIZE; i++) {
			if(!this->inventory[slotId].isValid()) {
				return slotId;
			}
			slotId++;
		}
	} else {
		for (unsigned int i = 0; i<PlayerInventory::Slots::TAB_SIZE; i++) {
			if(this->inventory[slotId].type == 0x00 && this->inventory[slotId].amount == 0x00)
				return slotId;
			slotId++;
		}
	}
	return std::numeric_limits<byte_t>::max();
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
		default:
			throw TraceableException("Invalid Entity Type: %i", entity->getEntityType());
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
	Packet pak(PacketID::World::Response::REMOVE_VISIBLE_ENTITY);
	pak.addWord(entity->getLocalId());
	return this->sendData(pak);
}

void Player::checkRegeneration() {
	if (this->consumedItems.size() > 0) {
		for (unsigned int i = 0; i < this->consumedItems.size(); i++) {
			ConsumedItem& item = this->consumedItems.at(i);
			long long lastTick = item.timeStamp.getDuration();
			dword_t amount = static_cast<dword_t>(item.valuePerSecond * lastTick / 1000.0f); //100% / 1000ms
			if (amount == 0x00)
				continue;
			item.timeStamp.update();
			bool eraseFlag = false;
			if (amount + item.valueConsumed >= item.maxRate) {
				amount = item.maxRate - item.valueConsumed;
				eraseFlag = true;
			}
#ifdef __ROSE_DEBUG__
			ChatService::sendDebugMessage(this, "Regenerated %i [%i] out of %i [%i per second] to a total of %i", amount, item.valueConsumed + amount, item.maxRate, item.valuePerSecond, this->getStatType<word_t>(item.influencedAbility) + amount);
#endif
			item.valueConsumed += amount;
			this->changeAbility(item.influencedAbility, amount, OperationService::OPERATION_ADDITION, false);
			if (eraseFlag) {
				Packet pak(PacketID::World::Response::BUFFS);
				pak.addWord(this->getLocalId());
				pak.addDWord(this->getBuffsVisuality());
				pak.addWord(this->getStatType<word_t>(item.influencedAbility));
				this->sendToVisible(pak);

				this->consumedItems.erase(this->consumedItems.begin() + i);
				i--;
			}
		}
	}
	if(time(NULL) - this->status.lastRegenCheck >= Status::DEFAULT_CHECK_TIME) {
		if(this->getCurrentHP() != this->getMaxHPW() || this->getCurrentMP() != this->getMaxMPW()) {
			float hpRegenAmountF = static_cast<float>(ceil(this->getMaxHP() * 2.0f / 100.0f));
			float mpRegenAmountF = static_cast<float>(ceil(this->getMaxMP() * 2.0f / 100.0f));
			if(this->getStance().asBYTE() == Stance::SITTING) {
				hpRegenAmountF *= 4;
				mpRegenAmountF *= 4;
			}
			//Just to be sure we add enough HP when one sits
			word_t hpRegenAmount = static_cast<word_t>(hpRegenAmountF);
			word_t mpRegenAmount = static_cast<word_t>(mpRegenAmountF);

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

bool Player::addQuest(const word_t questId) {
	PlayerQuest* journeyEntry = this->getEmptyQuestSlot();

	Packet pak(PacketID::World::Response::QUEST_ACTION);
	if (journeyEntry) {
		journeyEntry->setQuest(questId);

		pak.addByte(QuestReply::ADD_OKAY);
		pak.addByte(journeyEntry->getSlotId());
	}
	else {
		pak.addByte(QuestReply::ADD_FAILED);
		pak.addByte(0x00);
	}
	pak.addDWord(0x00); //QuestHash
	pak.addDWord(questId);
	if (!this->sendData(pak))
		return false;

	return (journeyEntry != nullptr);
}

bool Player::sendQuestTriggerViaMonster(const word_t monType) {
	Packet pak(PacketID::World::Response::QUEST_ITEM);
	pak.addWord(monType);
	return this->sendData(pak);
}

bool Player::updateQuestData() {
	Packet pak(PacketID::World::Response::UPDATE_QUEST_DATA);

	for (unsigned int i = 0; i < this->quest.var.episode.capacity(); i++)
		pak.addWord(this->quest.var.episode[i]);

	for (unsigned int i = 0; i < this->quest.var.job.capacity(); i++) //JOB
		pak.addWord(this->quest.var.job[i]);

	for (unsigned int i = 0; i < this->quest.var.planet.capacity(); i++) //PLANET
		pak.addWord(this->quest.var.planet[i]);

	for (unsigned int i = 0; i < this->quest.var.fraction.capacity(); i++) //UNION
		pak.addWord(this->quest.var.fraction[i]);

	for (unsigned int i = 0; i < this->quest.journey.capacity(); i++) {
		pak.addWord(this->quest.journey[i]->getQuestId()); //QuestID
		pak.addDWord(this->quest.journey[i]->getPassedTime()); //QuestTime

		for (unsigned int j = 0; j < PlayerQuest::QUEST_VAR_MAX; j++)
			pak.addWord(this->quest.journey[i]->getVar(j)); //VARS

		pak.addDWord(this->quest.journey[i]->getAllSwitches()); //SWITCHES
		for (unsigned int j = 0; j < PlayerQuest::QUEST_ITEMS_MAX; j++) {
			pak.addWord(this->quest.journey[i]->getItem(j).getPakHeader()); //ITEM HEAD
			pak.addDWord(this->quest.journey[i]->getItem(j).getPakData()); //ITEM DATA
		}
	}
	for (unsigned int i = 0; i < 0x10; i++)
		pak.addDWord(this->quest.dwFlag[i]);

	return this->sendData(pak);
}

bool Player::searchAndSelectQuest(const dword_t questId) {
	this->quest.selected = nullptr;
	for (unsigned int i = 0; i < this->quest.journey.size(); i++) {
		PlayerQuest* entry = this->quest.journey[i];
		if (entry && entry->getQuestId() == questId) {
			this->quest.selected = this->quest.journey[i];
			break;
		}
	}
	return this->quest.selected != nullptr;
}

const word_t Player::getQuestVariable(word_t varType, const word_t varId) {
	varType >>= 8;
	try {
		switch (varType) {
			case 0x00:
				if (this->quest.selected)
					return this->quest.selected->getQuestVar(varId);
				return 0;
			case 0x01:
				return this->quest.selected->getQuestVar(varId);
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
				throw TraceableException("varType %i is illegal!", varType);
		}
	} catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return 0x00;
}

void Player::setQuestVariable(word_t varType, const word_t varId, const word_t value) {
	varType >>= 8;
	try {
		switch (varType) {
		case 0x00:
			if (this->quest.selected)
				this->quest.selected->setQuestVar(varId, value);
		case 0x01:
			this->quest.selected->setSwitch(varId, value);
		case 0x02: //Time?
			return;
		case 0x03:
			this->quest.var.episode[varId] = value;
		case 0x04:
			this->quest.var.job[varId] = value;
		case 0x05:
			this->quest.var.planet[varId] = value;
		case 0x06:
			this->quest.var.fraction[varId] = value;
		default:
			throw TraceableException("varType %i is illegal!", varType);
		}
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
}

void Player::setQuestFlag(const word_t flagGroup, byte_t newValue) {
	if (newValue) {
		this->quest.flag[flagGroup >> 3] |= (1 << (flagGroup & 0x07));
	}
	else {
		this->quest.flag[flagGroup >> 3] &= ~(1 << (flagGroup & 0x07));
	}
}

bool Player::sendQuestTrigger(const dword_t hash, bool success) {
	Packet pak(PacketID::World::Response::QUEST_ACTION);
	pak.addByte(success == true ? QuestReply::TRIGGER_OKAY : QuestReply::TRIGGER_FAILED);
	pak.addByte(0x00); //SLOT
	pak.addDWord(hash);
	if (this->getSelectedQuest() != nullptr){
		pak.addDWord(this->getSelectedQuest()->getQuestId());
	}
	return this->sendData(pak);
}

PlayerQuest* Player::getQuestByID(const word_t questId) {
	for (unsigned int i = 0; i < this->quest.journey.size(); i++) {
		PlayerQuest* quest = this->quest.journey[i];
		if (!quest || quest->getQuestId() != questId)
			continue;
		return quest;
	}
	return nullptr;
}

PlayerQuest* Player::getEmptyQuestSlot() {
	for (unsigned int i = 0; i < this->quest.journey.size(); i++) {
		PlayerQuest* quest = this->quest.journey[i];
		if (quest->getQuestId() == 0x00)
			return quest;
	}
	return nullptr;
}

Item Player::getItemFromInventory(const word_t itemSlot) {
	if (itemSlot == 0 || itemSlot >= PlayerInventory::Slots::MAXIMUM)
		return Item();
	return this->inventory[itemSlot];
}

bool Player::addItemToInventory(const Item& item, byte_t itemSlot) {
	if (itemSlot == std::numeric_limits<byte_t>::max())
		itemSlot = this->getFreeInventorySlot(item);
	if (itemSlot < PlayerInventory::Slots::MAXIMUM && item.isValid()) {
		if (!item.isSingleSlot()) {
			this->inventory[itemSlot].amount += item.amount;
		}
		else {
			this->inventory[itemSlot] = item;
		}
		this->pakUpdateInventoryVisually(1, &itemSlot);
		Packet pak(PacketID::World::Response::REWARD_ITEM);
		pak.addByte(0x01); //Amount of rewarded items
		pak.addByte(itemSlot);
		pak.addWord(this->inventory[itemSlot].getPakHeader());
		pak.addDWord(this->inventory[itemSlot].getPakData());
		return this->sendData(pak);
	}
	return false;
}

Item Player::getQuestItem(const dword_t itemId) {
	Item wantedItem(itemId);
	for (unsigned int i = 0; i < PlayerQuest::QUEST_ITEMS_MAX; i++) {
		Item currentItem = this->quest.selected->getItem(i);
		if (wantedItem.type == currentItem.type && wantedItem.id == currentItem.id)
			return currentItem;
	}
	return Item();
}

dword_t Player::getSpecialStatType(const word_t statType) {
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
			return this->getPlayerKillFlag();
		case StatType::RANKING:
			return this->unionInfo.rank;
		case StatType::REPUTATION:
			return this->unionInfo.fame;
		case StatType::TENDENCY:
			return 0;
		case StatType::SENSIBILITY:
			return this->getSensibilityTotal();
		case StatType::STAT_POINT:
			return this->charInfo.statPoints;
		case StatType::SKILL_POINTS:
			return this->charInfo.skillPoints;
		case StatType::STRENGTH:
			return this->getStrengthTotal();
		case StatType::UNION_FACTION:
			return this->unionInfo.id;
	}
	return 0;
}

bool Player::changeAbility(const word_t abilityType, const dword_t amount, const byte_t operation, bool sendPacket) {
	word_t packetID = 0x00;
	switch (operation) {
		case OperationService::OPERATION_RETURN_RHS:
			packetID = PacketID::World::Response::CHANGE_ABILITY;
		break;
		case OperationService::OPERATION_ADDITION:
			packetID = PacketID::World::Response::ADD_TO_ABILITY;
		break;
	}

	dword_t result = 0x00;
	switch (abilityType) {
		case StatType::CHARM:
			this->attributes.charm = OperationService::resultOperation(this->attributes.charm, static_cast<const word_t>(amount), operation);
			result = this->attributes.charm;
		break;
		case StatType::CONCENTRATION:
			this->attributes.concentration = OperationService::resultOperation(this->attributes.concentration, static_cast<const word_t>(amount), operation);
			result = this->attributes.concentration;
		break;
		case StatType::CURRENT_HP:
			this->stats.curHP = OperationService::resultOperation(this->stats.curHP, static_cast<const word_t>(amount), operation);
			if (this->stats.curHP > this->getMaxHPW())
				this->stats.curHP = this->getMaxHPW();
			result = this->stats.curHP;
		break;
		case StatType::CURRENT_MP:
			this->stats.curMP = OperationService::resultOperation(this->stats.curMP, static_cast<const word_t>(amount), operation);
			if (this->stats.curMP > this->getMaxMPW())
				this->stats.curMP = this->getMaxMPW();
			result = this->stats.curMP;
		break;
		case StatType::DEXTERITY:
			this->attributes.dexterity = OperationService::resultOperation(this->attributes.dexterity, static_cast<const word_t>(amount), operation);
			result = this->attributes.dexterity;
		break;
		case StatType::FACE_STYLE:
			this->charInfo.visualTraits.faceStyle = OperationService::resultOperation(this->charInfo.visualTraits.faceStyle, static_cast<const word_t>(amount), operation);
			result = this->charInfo.visualTraits.faceStyle;
		break;
		case StatType::HAIR_STYLE:
			this->charInfo.visualTraits.hairStyle = OperationService::resultOperation(this->charInfo.visualTraits.hairStyle, static_cast<const word_t>(amount), operation);
			result = this->charInfo.visualTraits.hairStyle;
		break;
		case StatType::HEAD_SIZE:
		case StatType::HP_RECOVERY_RATE:
			return false;
		case StatType::INTELLIGENCE:
			this->attributes.intelligence = OperationService::resultOperation(this->attributes.intelligence, static_cast<const word_t>(amount), operation);
			result = this->attributes.intelligence;
		break;
		case StatType::JOB:
			this->charInfo.job = OperationService::resultOperation(this->charInfo.job, static_cast<const word_t>(amount), operation);
			result = this->charInfo.job;
		break;
		case StatType::LEVEL:
			this->charInfo.level = OperationService::resultOperation(this->charInfo.level, static_cast<const byte_t>(amount), operation);
			result = this->charInfo.level;
		break;
		case StatType::MONEY:
			//this->inventory[0x00].amount = QuestService::resultOperation(this->inventory[0x00].amount, static_cast<const dword_t>(amount), operation);
			this->inventory[0x00].amount = operation == 0x06 ? this->inventory[0x00].amount + amount : this->inventory[0x00].amount - amount;
			packetID = PacketID::World::Response::REWARD_ZULIES;
		break;
		case StatType::MP_CONSUMPTION_RATE:
		case StatType::MP_RECOVERY_RATE:
			return false;
		case StatType::PK_LEVEL:
			this->setPlayerKillFlag(OperationService::resultOperation(this->getPlayerKillFlag(), static_cast<const word_t>(amount), operation));
			result = this->getPlayerKillFlag();
		case StatType::RANKING:
			this->unionInfo.rank = OperationService::resultOperation(this->unionInfo.rank, static_cast<const byte_t>(amount), operation);
			result = this->unionInfo.rank;
		break;
		case StatType::REPUTATION:
			this->unionInfo.fame = OperationService::resultOperation(this->unionInfo.fame, static_cast<const byte_t>(amount), operation);
			result = this->unionInfo.fame;
		case StatType::TENDENCY:
			return false;
		case StatType::SENSIBILITY:
			this->attributes.sensibility = OperationService::resultOperation(this->attributes.sensibility, static_cast<const word_t>(amount), operation);
			result = this->attributes.sensibility;
		break;
		case StatType::STAT_POINT:
			this->charInfo.statPoints = OperationService::resultOperation(this->charInfo.statPoints, static_cast<const word_t>(amount), operation);
			result = this->charInfo.statPoints;
		break;
		case StatType::SKILL_POINTS:
			this->charInfo.skillPoints = OperationService::resultOperation(this->charInfo.skillPoints, static_cast<const word_t>(amount), operation);
			result = this->charInfo.skillPoints;
		break;
		case StatType::STRENGTH:
			this->attributes.strength = OperationService::resultOperation(this->attributes.strength, static_cast<const word_t>(amount), operation);
			result = this->attributes.strength;
		break;
		case StatType::UNION_FACTION:
			this->unionInfo.id = OperationService::resultOperation(this->unionInfo.id, static_cast<const byte_t>(amount), operation);
			result = this->unionInfo.id;
		break;
	}
	if (sendPacket) {
		Packet pak(packetID);
		if (packetID != PacketID::World::Response::REWARD_ZULIES) {
			pak.addWord(abilityType);
			pak.addDWord(amount);
		}
		else {
			pak.addQWord(this->inventory[0x00].amount);
			pak.addByte(0x00);
		}
		return this->sendData(pak);
	}
	return true;
}

bool Player::addConsumableToList(const word_t abilityToInfluence, const dword_t maxValue, const dword_t valuePerSecond) {
	const dword_t consumedSoFar = this->consumedItems.size();
	ConsumedItem item(abilityToInfluence, maxValue, valuePerSecond);
	this->consumedItems.push_back(item);
	return (this->consumedItems.size() > consumedSoFar);
}

//TODO: ADD TO STAT CALCULATION
word_t Player::checkClothesForStats(const word_t statAmount, ...) {
	std::vector<dword_t> stats;
	va_list ap;
	va_start(ap, statAmount);

	word_t result = 0x00;
	for(unsigned int i=0;i<statAmount;i++) {
		stats.push_back(va_arg(ap, dword_t));
	}
	for(unsigned int i=1;i<=PlayerInventory::Slots::SHIELD;i++) {
		if (!this->inventory[i].isValid()) {
			continue;
		}
		try {
			STBEntry& entry = mainServer->getEquipmentEntry( this->inventory[i].type, this->inventory[i].id );
			word_t firstStatType = entry.getColumn<word_t>( EquipmentSTB::STAT_FIRST_TYPE );
			word_t secondStatType = entry.getColumn<word_t>( EquipmentSTB::STAT_SECOND_TYPE );
			for(unsigned int k=0;k<stats.size();k++) {
				if(firstStatType == stats.at(k)) {
					result += entry.getColumn<word_t>( EquipmentSTB::STAT_FIRST_AMOUNT );
				}
				if(secondStatType == stats.at(k)) {
					result += entry.getColumn<word_t>( EquipmentSTB::STAT_SECOND_AMOUNT );
				}
			}
		} catch(std::exception& ex) {
			std::cout << ex.what() << "\n";
		}
	}
	return result;
}

word_t Player::checkSkillsForStats(const word_t basicAmount, const word_t statAmount, ...) {
	std::vector<dword_t> stats;
	va_list ap;
	va_start(ap, statAmount);

	word_t result = 0x00;
	for(unsigned int i=0;i<statAmount;i++) {
		stats.push_back(va_arg(ap, dword_t));
	}
	for(unsigned int i=0;i<this->skills.size();i++) {
		if (!this->skills[i]) {
			continue;
		}
		
		Skill* currentSkill = this->skills[i];
		if (currentSkill->getType() != SkillType::PASSIVE) {
			continue;
		}
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
	word_t totalAttackpower = 0x00;
	word_t weaponAtkPower = 0x00;
	word_t weaponType = 0x00;
	if (this->inventory[PlayerInventory::Slots::WEAPON].isValid()) {
		weaponAtkPower += mainServer->getWeaponAttackpower(this->inventory[PlayerInventory::Slots::WEAPON].id);
		weaponType = mainServer->getSubType(ItemType::WEAPON, this->inventory[PlayerInventory::Slots::WEAPON].id);
	} else {
		word_t dexPart = static_cast<word_t>(this->attributes.getDexterityTotal() * 0.3);
		word_t strPart = static_cast<word_t>(this->attributes.getStrengthTotal() * 0.5);
		word_t lvlPart = static_cast<word_t>(this->charInfo.level * 0.2);

		totalAttackpower = dexPart + strPart + lvlPart;
	}
	word_t weaponAtkParts[5] = { 0x00 };
	switch( weaponType ) {
		case WeaponType::MELEE_ONE_HANDED_SWORD:
		case WeaponType::MELEE_ONE_HANDED_BLUNT:
		case WeaponType::MELEE_TWO_HANDED_AXE:
		case WeaponType::MELEE_TWO_HANDED_SPEAR:
		case WeaponType::MELEE_TWO_HANDED_SWORD:
			weaponAtkParts[0] = static_cast<word_t>(this->attributes.getStrengthTotal() * 0.75);
			weaponAtkParts[1] = static_cast<word_t>(this->getLevel() * 0.2);
			weaponAtkParts[2] = static_cast<word_t>(((this->attributes.getStrengthTotal() * 0.05) + 29) * weaponAtkPower / 30.0);
		break;
		case WeaponType::MELEE_DOUBLE_SWORD:
			weaponAtkParts[0] = static_cast<word_t>(this->getLevel() * 0.2);
			weaponAtkParts[1] = static_cast<word_t>(this->attributes.getStrengthTotal() * 0.63);
			weaponAtkParts[2] = static_cast<word_t>(this->attributes.getDexterityTotal() * 0.45);
			weaponAtkParts[3] = static_cast<word_t>((this->attributes.getDexterityTotal() * 0.05 + 25) * weaponAtkPower / 26.0);
		break;
		case WeaponType::MELEE_KATAR:
			weaponAtkParts[0] = static_cast<word_t>(this->getLevel() * 0.2);
			weaponAtkParts[1] = static_cast<word_t>(this->attributes.getStrengthTotal() * 0.42);
			weaponAtkParts[2] = static_cast<word_t>(this->attributes.getDexterityTotal() * 0.55);
			weaponAtkParts[3] = static_cast<word_t>((this->attributes.getDexterityTotal() * 0.05 + 20) * weaponAtkPower / 21);
		break;
		case WeaponType::RANGE_BOW:
			weaponAtkParts[0] = static_cast<word_t>((this->attributes.getStrengthTotal() + this->getLevel()) * 0.1);
			weaponAtkParts[1] = static_cast<word_t>(((this->attributes.getDexterityTotal() * 0.04) + (this->attributes.getSensibilityTotal() * 0.03 + 29)) * weaponAtkPower / 30.0);
			weaponAtkParts[2] = static_cast<word_t>(this->attributes.getDexterityTotal() * 0.52);
			weaponAtkParts[3] = static_cast<word_t>(mainServer->getQuality(ItemType::OTHER, this->inventory[PlayerInventory::Slots::ARROWS].id) * 0.5);
		break;
		case WeaponType::RANGE_GUN:
		case WeaponType::RANGE_DUAL_GUN:
			weaponAtkParts[0] = static_cast<word_t>(this->attributes.getSensibilityTotal() * 0.47);
			weaponAtkParts[1] = static_cast<word_t>(mainServer->getQuality(ItemType::OTHER, this->inventory[PlayerInventory::Slots::BULLETS].id) * 0.8);
			weaponAtkParts[2] = static_cast<word_t>(this->getLevel() * 0.1);
			weaponAtkParts[3] = static_cast<word_t>(this->attributes.getDexterityTotal() * 0.3);
			weaponAtkParts[4] = static_cast<word_t>((this->attributes.getConcentrationTotal() * 0.04 + this->attributes.getSensibilityTotal() * 0.05 + 29) * weaponAtkPower / 30.0);
		break;
		case WeaponType::RANGE_LAUNCHER:
			weaponAtkParts[0] = static_cast<word_t>(this->attributes.getConcentrationTotal() * 0.47);
			weaponAtkParts[1] = static_cast<word_t>(mainServer->getQuality(ItemType::OTHER, this->inventory[PlayerInventory::Slots::CANNONSHELLS].id));
			weaponAtkParts[2] = static_cast<word_t>(this->attributes.getStrengthTotal() * 0.32);
			weaponAtkParts[3] = static_cast<word_t>(this->attributes.getConcentrationTotal() * 0.45);
			weaponAtkParts[4] = static_cast<word_t>((this->attributes.getConcentrationTotal() * 0.04 +
				this->attributes.getSensibilityTotal() * 0.05 + 29) * weaponAtkPower / 30.0);
		break;
		case WeaponType::RANGE_CROSSBOW:
			weaponAtkParts[0] = static_cast<word_t>((this->attributes.getStrengthTotal() + this->getLevel()) * 0.1);
			weaponAtkParts[1] = static_cast<word_t>(this->attributes.getDexterityTotal() * 0.04);
			weaponAtkParts[2] = static_cast<word_t>((this->attributes.getSensibilityTotal() * 0.03 + 29) * weaponAtkPower / 30);
			weaponAtkParts[3] = static_cast<word_t>(this->attributes.getDexterityTotal() * 0.52);
			weaponAtkParts[4] = static_cast<word_t>(mainServer->getQuality(ItemType::OTHER, this->inventory[PlayerInventory::Slots::ARROWS].id) * 0.5);
		break;
		case WeaponType::MAGIC_WAND:
			weaponAtkParts[0] = static_cast<word_t>(this->getLevel() * 0.2);
			weaponAtkParts[1] = static_cast<word_t>(this->attributes.getIntelligenceTotal() * 0.6);
			weaponAtkParts[2] = static_cast<word_t>((this->attributes.getSensibilityTotal() * 0.1 + 26) * weaponAtkPower / 27);
		break;
		case WeaponType::MAGIC_STAFF:
			weaponAtkParts[0] = static_cast<word_t>(this->getLevel() * 0.2);
			weaponAtkParts[1] = static_cast<word_t>((this->attributes.getIntelligenceTotal() + this->attributes.getStrengthTotal()) * 0.4);
			weaponAtkParts[2] = static_cast<word_t>((this->attributes.getIntelligenceTotal() * 0.05 + 29) * weaponAtkPower / 30.0);
		break;
	}
	for (unsigned int i = 0; i < 5; i++) {
		totalAttackpower += weaponAtkParts[i];
	}

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
	word_t atkSpeed = 115;
	if(this->isWeaponEquipped()) {
		int speedType = mainServer->getWeaponAttackspeed(this->inventory[PlayerInventory::Slots::WEAPON].id);
		atkSpeed = 1500 / (speedType + 5);
	}
	atkSpeed += this->checkClothesForStats(1, StatType::ATTACK_SPEED);
	atkSpeed += this->getBuffAmount( Buffs::Visuality::ATTACKSPEED_UP );
	atkSpeed -= this->getBuffAmount( Buffs::Visuality::ATTACKSPEED_DOWN );

	this->stats.attackSpeed = atkSpeed;
}

bool Player::getAttackAnimation() {
	word_t motion = 0;
	byte_t motionType = mainServer->getMotionTypeForWeapon(this->inventory[PlayerInventory::Slots::WEAPON].id);
	if (this->combat.type == Combat::NORMAL) {
		motion = mainServer->getMotionId(ItemType::WEAPON, motionType);
	}
	else if (this->combat.type == Combat::SKILL) {
		motion = mainServer->getMotionId(this->combat.skill->getAnimationId(), motionType);
	}
	this->animation = mainServer->getAttackMotionPlayer(motion);
	return this->animation != nullptr;
}

void Player::updateDefense() {
	word_t defense = 0x00;
	if (this->getJob() & 0x10) {//Second tier jobs (x21/x22)
		defense += 25;
	}

	defense += static_cast<word_t>((this->attributes.getStrengthTotal() + 5) * 0.35);
	defense += static_cast<word_t>((this->getLevel() + 15) * 0.7);

	defense += this->checkClothesForStats(1, StatType::DEFENSE_PHYSICAL);
	defense += this->getBuffAmount( Buffs::Visuality::DEFENSE_UP );
	defense -= this->getBuffAmount( Buffs::Visuality::DEFENSE_DOWN );
	for (unsigned int i = PlayerInventory::Slots::FACE; i<PlayerInventory::Slots::SHIELD; i++) {
		if (i == PlayerInventory::Slots::WEAPON || !this->inventory[i].isValid())
			continue;
		try {
			STBEntry& armorEntry = mainServer->getEquipmentEntry(this->inventory[i].type, this->inventory[i].id);
			defense += armorEntry.getColumn<word_t>(EquipmentSTB::DEFENSE_PHYISCAL);
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
	word_t newHitrate = 0x00;
	if(this->isWeaponEquipped()) {
		word_t statPart = static_cast<word_t>((this->attributes.getConcentrationTotal()+10)*0.8f);
		word_t qualityPart = static_cast<word_t>(mainServer->getQuality(ItemType::WEAPON, this->inventory[PlayerInventory::Slots::WEAPON].id) * 0.6);
		word_t durabilityPart = static_cast<word_t>(this->inventory[PlayerInventory::Slots::WEAPON].durability * 0.8);

		newHitrate = statPart + qualityPart + durabilityPart;
	} else {
		newHitrate = static_cast<word_t>(((this->attributes.getConcentrationTotal() + 10)*0.5) + 15);
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
	word_t maxHp = static_cast<word_t>(::sqrt(static_cast<double>(additionPart1 + this->getLevel())) * (additionPart2 + this->getLevel()) * multiplier);
	maxHp += ( this->attributes.getStrengthTotal() << 1 );

	word_t additionalMaxHP = 0x00;
	//BUGGED
	if(this->getJob() & 0x10) { //Second Jobs: X2Y = X ClassType, Y = SubClass (e.g. Scout)
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
	Item& shoes = this->inventory[PlayerInventory::fromItemType(ItemType::SHOES)];
	switch (this->getStance().asBYTE()) {
		case Stance::WALKING:
			this->stats.movementSpeed = 200;
		break;
		case Stance::RUNNING:
			this->stats.movementSpeed = 425;
			if (shoes.isValid()) {
				STBEntry& entry = mainServer->getEquipmentEntry(ItemType::SHOES, shoes.id);
				word_t shoeSpeed = entry.getColumn<word_t>(EquipmentSTB::MOVEMENT_SPEED) - 50;
				this->stats.movementSpeed += shoeSpeed;
			}
		break;
		case Stance::DRIVING:
			this->stats.movementSpeed = 1200;
		break;
		default:
			GlobalLogger::warning("'%s' has selected an invalid stance: %i", this->getName().c_str(), this->getStance().asBYTE());
	}
}

float Player::getAttackRange() const {
	//Basic range
	float range = 150.0f;
	if (this->getTarget() != nullptr && this->getTarget()->getEntityType() == Entity::TYPE_DROP) {
		return range;
	}

	//Weapon range enhances the basic range
	switch (this->combat.type) {
		case Combat::NORMAL:
			if (this->inventory[PlayerInventory::Slots::WEAPON].isValid()) {
				STBEntry& weapon = mainServer->getEquipmentEntry(ItemType::WEAPON, this->inventory[PlayerInventory::Slots::WEAPON].id);
				range += weapon.getColumn<float>(EquipmentSTB::ATTACK_RANGE);
			}
		break;
		case Combat::SKILL:
			if (this->combat.skill == nullptr) {
				throw NullpointerException("Skill is not set!");
			}
			if (this->combat.skill->getInitRange() > 0) {
				range += static_cast<float>(this->combat.skill->getInitRange());
			}
		break;
	}
	return range;
}

void Player::resetAttributes() { 
	for (unsigned int i = StatType::STRENGTH; i <= StatType::SENSIBILITY; i++) {
		this->changeAbility(i, i <= StatType::CONCENTRATION ? 15 : 10, OperationService::OPERATION_RETURN_RHS);
	}
	this->charInfo.statPoints = 0;
	for (unsigned int i = 1; i < this->getLevel(); i++) {
		this->charInfo.statPoints += (i * 10 / 8) + 10;
	}
}

void Player::resetSkills() {
	for (unsigned int i = PlayerSkill::ACTIVE_BEGIN; i < PlayerSkill::PASSIVE_BEGIN + PlayerSkill::PAGE_SIZE; i++) {
		this->skills[i] = nullptr;
	}
	this->charInfo.skillPoints = 0;
	for (unsigned int i = 1; i < this->getLevel(); i++){
		if (i == 10 || i == 14)
			this->charInfo.skillPoints += 2;
		else if (i == 18)
			this->charInfo.skillPoints += 3;
		else if (i == 22)
			this->charInfo.skillPoints += 4;
		else if (i == 22 && ((i - 22) % 4) == 0)
			this->charInfo.skillPoints += 5;
		else if (i >= 100 && (i % 2) == 0)
			this->charInfo.skillPoints += 5;
	}
}

bool Player::changeSkill(const word_t totalId) {
	Skill* wantedSkill = mainServer->getSkill(totalId);
	word_t skillPageStart = Skill::getPage(wantedSkill);
	word_t lastSkill = (skillPageStart + PlayerSkill::PAGE_SIZE);
	for (unsigned int i = skillPageStart; i < lastSkill; i++) {
		if (this->skills[i] && this->skills[i]->getIdBasic() == wantedSkill->getIdBasic()) {
			this->skills[i] = wantedSkill;

			Packet pak(PacketID::World::Response::LEARN_SKILL);
			pak.addByte(PlayerSkill::UPGRADE_SUCCESS);
			pak.addByte(i); //SLOT
			pak.addWord(this->skills[i]->getId());
			pak.addWord(this->charInfo.skillPoints);
			return this->sendData(pak);
		}
	}
	return false;
}

bool Player::addSkill(Skill* skillToAdd) {
	if (!skillToAdd)
		return false;
	word_t skillPageStart = Skill::getPage(skillToAdd);
	word_t lastSkill = (skillPageStart + PlayerSkill::PAGE_SIZE);
	for (unsigned int i = skillPageStart; i < lastSkill; i++) {
		if (!this->skills[i]) {
			this->skills[i] = skillToAdd;
			Packet pak(PacketID::World::Response::LEARN_SKILL);
			pak.addByte(PlayerSkill::LEARN_SUCCESS);
			pak.addByte(i); //SLOT
			pak.addWord(this->skills[i]->getId());
			pak.addWord(this->charInfo.skillPoints);
			return this->sendData(pak);
		}
	}
	return false;
}

dword_t Player::getExperienceForLevelup() {
	dword_t result = 0x00;
	if(this->getLevel() <= 15) {
		result = static_cast<dword_t>( (this->getLevel() + 10 ) * (this->getLevel() + 5) * (this->getLevel() + 3) * 0.7 );
	} else if(this->getLevel() <= 50) {
		result = static_cast<dword_t>( (this->getLevel() - 5 ) * (this->getLevel() + 2 ) * (this->getLevel() + 2 ) * 2.2 );
	} else if(this->getLevel() <= 100 ) {
		result = static_cast<dword_t>( (this->getLevel() - 38 ) * (this->getLevel() - 5 ) * (this->getLevel() + 2 ) * 9 );
	} else if(this->getLevel() <= 139 ) {
		result = static_cast<dword_t>( (this->getLevel() + 220 ) * (this->getLevel() + 34 ) * (this->getLevel() + 22 ) );
	} else {
		result = static_cast<dword_t>( (this->getLevel() - 126 ) * (this->getLevel() - 15 ) * (this->getLevel() + 7 ) * 41 ); 
	}
	return result;
}

void Player::addExperience(const dword_t additionalExp) {
	this->charInfo.experience += additionalExp;

	Packet pak(PacketID::World::Response::UPDATE_EXPERIENCE);
	pak.addDWord( this->charInfo.experience );
	pak.addWord( this->stats.stamina );
	pak.addWord( 0x00 ); //dword_t Stamina?
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
		pak.addWord(this->getLocalId());
		pak.addWord( this->getLevel() );
		pak.addDWord( this->getExperience() );
		pak.addWord( this->charInfo.statPoints );
		pak.addWord( this->charInfo.skillPoints );
		if(!this->sendData(pak))
			return;

		pak.newPacket( PacketID::World::Response::LEVEL_UP );
		pak.addWord(this->getLocalId());
		if(!this->sendToVisible( pak, this ))
			return;
	}
}

bool Player::pakExit() {
	Packet pak(PacketID::World::Response::EXIT);
	pak.addWord(0x00);
	if (!this->sendData(pak))
		return false;

	mainServer->notifyCharServer(PacketID::Character::Custom::CLIENT_EXIT, this->accountInfo.id);
	this->setIsActive(false);

	return true;
}

bool Player::pakPlayerInfos() {
	this->setMapId(this->getMapId());
	mainServer->changeToMap(this, this->getMapId());
	
	Packet pak(PacketID::World::Response::PLAYER_INFOS);
	pak.addByte(this->getVisualTraits().sex);
	pak.addWord(this->entityInfo.mapId);
	pak.addFloat(this->position.current.x);
	pak.addFloat(this->position.current.y);
	pak.addWord(this->charInfo.respawnTownId); //SAVED POSITION
	pak.addDWord(this->getVisualTraits().faceStyle);
	pak.addDWord(this->getVisualTraits().hairStyle);

	pak.addDWord(this->inventory[PlayerInventory::Slots::HEADGEAR].getPakVisuality());
	pak.addDWord(this->inventory[PlayerInventory::Slots::ARMOR].getPakVisuality());
	pak.addDWord(this->inventory[PlayerInventory::Slots::GLOVES].getPakVisuality());
	pak.addDWord(this->inventory[PlayerInventory::Slots::SHOES].getPakVisuality());
	pak.addDWord(this->inventory[PlayerInventory::Slots::FACE].getPakVisuality());
	pak.addDWord(this->inventory[PlayerInventory::Slots::BACK].getPakVisuality());
	pak.addDWord(this->inventory[PlayerInventory::Slots::WEAPON].getPakVisuality());
	pak.addDWord(this->inventory[PlayerInventory::Slots::SHIELD].getPakVisuality());

	pak.addByte(0x00); //BIRTHSTONE
	pak.addWord(0x00); //BIRTHPLACE?
	pak.addWord(this->getJob());
	pak.addByte(this->unionInfo.id); //UNION
	pak.addByte(this->unionInfo.rank); //RANK
	pak.addByte(this->unionInfo.fame); //FAME

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

	for (dword_t i = 0; i < UnionInfo::UNION_AMOUNT; i++) {
		pak.addWord(this->unionInfo.points[i]);
	}

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
		pak.addWord(this->quickbar[i]); //QUICK BAR
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
	for (unsigned int i = 1; i < PlayerInventory::Slots::MAXIMUM; i++) {
		pak.addWord(this->inventory[i].getPakHeader());
		pak.addDWord(this->inventory[i].getPakData());
	}
	return this->sendData(pak);
}

bool Player::pakQuestData() {
	Packet pak(PacketID::World::Response::QUEST_DATA);
	
	for (unsigned int i = 0; i < this->quest.var.episode.capacity(); i++)
		pak.addWord(this->quest.var.episode[i]);

	for (unsigned int i = 0; i < this->quest.var.job.capacity(); i++) //JOB
		pak.addWord(this->quest.var.job[i]);

	for (unsigned int i = 0; i < this->quest.var.planet.capacity(); i++) //PLANET
		pak.addWord(this->quest.var.planet[i]);

	for (unsigned int i = 0; i < this->quest.var.fraction.capacity(); i++) //UNION
		pak.addWord(this->quest.var.fraction[i]);

	for (unsigned int i = 0; i < this->quest.journey.capacity(); i++) {
		PlayerQuest* quest = this->quest.journey[i];
		pak.addWord(quest->getQuestId()); //QuestID
		pak.addDWord(quest->getPassedTime()); //QuestTime

		for (unsigned int j = 0; j < PlayerQuest::QUEST_VAR_MAX; j++) {
			pak.addWord(quest->getVar(j)); //VARS
		}
		
		pak.addDWord(quest->getAllSwitches()); //SWITCHES
		for (unsigned int j = 0; j < PlayerQuest::QUEST_ITEMS_MAX; j++) {
			pak.addWord(quest->getItem(j).getPakHeader()); //ITEM HEAD
			pak.addDWord(quest->getItem(j).getPakData()); //ITEM DATA
		}
	}
	for (unsigned int i = 0; i < 0x10; i++) {
		pak.addDWord(this->quest.dwFlag[i]);
	}

	for (unsigned int i = 0; i < 30; i++) {
		pak.addWord(0x00); //ITEM HEAD WISHLIST
		pak.addDWord(0x00);
	}
	//220.452kb
	return this->sendData(pak);
}

bool Player::pakQuestAction() {
	byte_t action = this->packet.getByte(0x00);
	byte_t questPart = this->packet.getByte(0x01);
	dword_t questHash = this->packet.getDWord(0x02);

	//questHash = 0xc47bcb25
	dword_t executedQuestHash = 0x00;
	switch (action) {
		case 0x01:
			GlobalLogger::fatal("QuestAction(): REQUEST_ADD - questPart %i, questHash = %i", questPart, questHash);
		break;
		case 0x02:
			if (this->searchAndSelectQuest(questHash)) {
				this->getSelectedQuest()->reset();
				executedQuestHash = questHash;
			}
			action = (executedQuestHash > 0 ? QuestReply::DELETE_OKAY : QuestReply::DELETE_FAILED);
		break;
		case 0x03:
			executedQuestHash = QuestService::runQuest(this, questHash);
			action = (executedQuestHash > 0 ? QuestReply::TRIGGER_OKAY : QuestReply::TRIGGER_FAILED);
		break;
	}
	Packet pak(PacketID::World::Response::QUEST_ACTION);
	pak.addByte(action);
	pak.addByte(0x00);
	if (executedQuestHash > 0x00) {
		pak.addDWord(executedQuestHash);
	} else {
		pak.addDWord(questHash);
	}
	return this->sendData(pak);
}

bool Player::saveQuests() {
	if (!mainServer->sqlInsert("DELETE FROM character_quests WHERE charId=%i", this->charInfo.id))
		return false;
	for (unsigned int i = 0; i < this->quest.journey.capacity(); i++) {
		PlayerQuest* quest = this->quest.journey[i];
		if (quest->getQuestId() != 0x00) {
			char varsBuf[0x400] = { 0x00 };
			for (unsigned int j = 0; j < PlayerQuest::QUEST_VAR_MAX; j++) {
				if (j == 0x00) {
					sprintf(varsBuf, "%i", quest->getQuestVar(j));
				}
				else {
					sprintf(&varsBuf[strlen(varsBuf)], ",%i", quest->getQuestVar(j));
				}
			}
			char itemBuf[0x400] = { 0x00 };
			for (unsigned int j = 0; j < PlayerQuest::QUEST_ITEMS_MAX; j++) {
				Item& item = quest->getItem(j);
				if (j == 0x00) {
					sprintf(itemBuf, "%i,%i", item.type * 1000 + item.id, item.amount);
				}
				else {
					sprintf(&itemBuf[strlen(itemBuf)], ";%i,%i", item.type * 1000 + item.id, item.amount);
				}
			}
			if (!mainServer->sqlInsert("INSERT INTO character_quests (charId, questId, questSlot, timeStamp, vars, items) VALUES(%i, %i, %i, %i, '%s', '%s')", this->charInfo.id, quest->getQuestId(), quest->getSlotId(), (quest->getPassedTime() > 0 ? static_cast<dword_t>(time(NULL) - quest->getPassedTime()) : 0x00), varsBuf, itemBuf))
				return false;
		}
	}
	char questString[0x500] = { 0x00 };
	sprintf(questString, "UPDATE character_quest_flags SET questFlags='");
	for (unsigned int i = 0; i < Player::questInfo::FLAGS_MAX_byte_t; i++) {
		if (i == 0) {
			sprintf(&questString[strlen(questString)], "%i", this->quest.flag[i]);
		}
		else {
			sprintf(&questString[strlen(questString)], ",%i", this->quest.flag[i]);
		}
	}
	sprintf(&questString[strlen(questString)], "', episodeFlags='");
	for (unsigned int i = 0; i < Player::questInfo::questVars::EPISODE_MAX; i++) {
		if (i == 0) {
			sprintf(&questString[strlen(questString)], "%i", this->quest.var.episode[i]);
		}
		else {
			sprintf(&questString[strlen(questString)], ",%i", this->quest.var.episode[i]);
		}
	}
	sprintf(&questString[strlen(questString)], "', jobFlags='");
	for (unsigned int i = 0; i < Player::questInfo::questVars::JOB_MAX; i++) {
		if (i == 0) {
			sprintf(&questString[strlen(questString)], "%i", this->quest.var.job[i]);
		}
		else {
			sprintf(&questString[strlen(questString)], ",%i", this->quest.var.job[i]);
		}
	}
	sprintf(&questString[strlen(questString)], "', planetFlags='");
	for (unsigned int i = 0; i < Player::questInfo::questVars::PLANET_MAX; i++) {
		if (i == 0) {
			sprintf(&questString[strlen(questString)], "%i", this->quest.var.planet[i]);
		}
		else {
			sprintf(&questString[strlen(questString)], ",%i", this->quest.var.planet[i]);
		}
	}
	sprintf(&questString[strlen(questString)], "', fractionFlags='");
	for (unsigned int i = 0; i < Player::questInfo::questVars::UNION_MAX; i++) {
		if (i == 0) {
			sprintf(&questString[strlen(questString)], "%i", this->quest.var.fraction[i]);
		}
		else {
			sprintf(&questString[strlen(questString)], ",%i", this->quest.var.fraction[i]);
		}
	}
	sprintf(&questString[strlen(questString)], "' WHERE charId=%i", this->charInfo.id);
	if (!mainServer->sqlInsert(questString))
		return false;
	return true;
}

bool Player::saveInfos() {
	Map *map = mainServer->getMap(this->getMapId());
	word_t respawnId = 0x00;
	if(map) 
		respawnId = map->getRespawnPointId(this->getPositionCurrent());
	if (!mainServer->sqlInsert("UPDATE characters SET level=%i, experience=%i, job=%i, zulies=%i, saveTown=%i, respawnMap=%i, respawnId=%i WHERE id=%i", this->getLevel(), this->getExperience(), this->getJob(), this->inventory[0x00].amount, this->charInfo.respawnTownId, this->getMapId(), respawnId, this->charInfo.id)) {
		GlobalLogger::warning("Couldn't update character infos of client: %s!\n", this->getName().c_str());
		return false;
	}
	if (!mainServer->sqlInsert("UPDATE character_stats SET strength=%i, dexterity=%i, intelligence=%i, concentration=%i, charm=%i, sensibility=%i, statPoints=%i, skillPoints=%i WHERE id=%i", this->attributes.strength, this->attributes.dexterity, this->attributes.intelligence, this->attributes.concentration, this->attributes.charm, this->attributes.sensibility, this->charInfo.statPoints, this->charInfo.skillPoints, this->charInfo.id)) {
		GlobalLogger::warning("Couldn't update character stats of client: %s!\n", this->getName().c_str());
		return false;
	}
	if (!mainServer->sqlInsert("DELETE FROM inventory WHERE charId=%i", this->charInfo.id)) {
		GlobalLogger::warning("Couldn't delete inventory of client: %s!\n", this->getName().c_str());
		return false;
	}
	for (unsigned int i = 1; i<PlayerInventory::Slots::MAXIMUM; i++) {
		if (this->inventory[i].isValid()) {
			if(!mainServer->sqlInsert("INSERT INTO inventory (charId, slot, itemId, durability, lifespan, count, refine) VALUES(%i, %i, %i, %i, %i, %i, %i)", this->charInfo.id, i, this->inventory[i].type * 10000 + this->inventory[i].id, this->inventory[i].durability, this->inventory[i].lifespan, this->inventory[i].amount.get(), this->inventory[i].refine)) {
				GlobalLogger::warning("Couldn't insert inventory slot %i [%i] into the inventory table!\n", i, this->inventory[i].type * 10000 + this->inventory[i].id);
			}
		}
	}

	if (!this->saveQuests())
		return false;

	word_t lastSkill = (PlayerSkill::BASIC_BEGIN + PlayerSkill::PAGE_SIZE);
	char buffer[0x100] = { 0x00 };
	for (unsigned int i = PlayerSkill::BASIC_BEGIN; i < lastSkill; i++) {
		if (i == 0)
			sprintf(&buffer[strlen(buffer)], "%i", this->skills[i] == nullptr ? 0x00 : this->skills[i]->getId());
		else
			sprintf(&buffer[strlen(buffer)], ",%i", this->skills[i] == nullptr ? 0x00 : this->skills[i]->getId());
	}
	mainServer->sqlInsert("UPDATE character_skills SET basicSkills='%s' WHERE id=%i", buffer, this->charInfo.id);

	return true;
}

bool Player::loadQuests() {
	if (!mainServer->sqlRequest("SELECT questId, questslot, timeStamp, vars, items FROM character_quests WHERE charId=%i", this->charInfo.id)) {
		GlobalLogger::warning("Couldn't load quest infos for %s!\n", this->getName().c_str());
		return false;
	}
	dword_t rowCount = mainServer->sqlGetRowCount();
	MYSQL_ROW row = mainServer->sqlGetNextRow();
	for (unsigned int i = 0; i < rowCount; i++) {
		const byte_t currentEntry = atoi(row[1]);
		this->quest.journey[currentEntry]->setQuest(atoi(row[0]));
		const dword_t passedTime = atoi(row[2]);
		if (passedTime > 0)
			this->quest.journey[currentEntry]->setPassedTime(static_cast<dword_t>(time(NULL) - passedTime));

		std::string vars = row[3]; byte_t idx = 0x00;
		while (vars.find(",") != -1) {
			this->quest.journey[currentEntry]->setQuestVar(idx, atoi(vars.substr(0, vars.find(",")).c_str()));
			vars = vars.substr(vars.find(",") + 1);
			idx++;
		}
		this->quest.journey[currentEntry]->setQuestVar(idx, atoi(vars.c_str()));

		std::string items = row[4];
		while (items.find(";") != -1) {
			std::string currentItem = items.substr(0, items.find(";"));
			Item newItem(atoi(currentItem.substr(0, currentItem.find(",")).c_str()));
			newItem.amount = atoi(currentItem.substr(currentItem.find(",") + 1).c_str());
			this->quest.journey[currentEntry]->addItem(newItem);

			items = items.substr(items.find(";") + 1);
		}
		row = mainServer->sqlGetNextRow();
	}
	mainServer->sqlFreeResult();

	//TODO: ALLGEMEINE QUESTVARS
	if (!mainServer->sqlRequest("SELECT questFlags, episodeFlags, jobFlags, planetFlags, fractionFlags FROM character_quest_flags WHERE charId=%i", this->charInfo.id))
		return false;
	row = mainServer->sqlGetNextRow();

#define SPLIT_ROW(flagName) \
	idx = 0x00; \
	while (currentFlag.find(",") != -1) {\
		flagName[idx] = atoi(currentFlag.substr(0, currentFlag.find(",")).c_str()); \
		currentFlag = currentFlag.substr(currentFlag.find(",") + 1);\
		idx++;\
	}\
	flagName[idx] = atoi(currentFlag.c_str());

	std::string currentFlag = std::string(row[0]);
	byte_t idx = 0x00;
	SPLIT_ROW(this->quest.flag);

	currentFlag = std::string(row[1]);
	SPLIT_ROW(this->quest.var.episode);

	currentFlag = std::string(row[2]);
	SPLIT_ROW(this->quest.var.job);

	currentFlag = std::string(row[3]);
	SPLIT_ROW(this->quest.var.planet);

	currentFlag = std::string(row[4]);
	SPLIT_ROW(this->quest.var.fraction);

#undef SPLIT_ROW
	return true;
}

bool Player::loadInfos() {
	if (!mainServer->sqlRequest("SELECT lastChar_Id, accesslevel FROM accounts WHERE id=%i", this->accountInfo.id)) {
		return false;
	}
	MYSQL_ROW row = mainServer->sqlGetNextRow();
	this->charInfo.id = atoi(row[0]);
	this->accountInfo.accessLevel = static_cast<byte_t>(atoi(row[1]));
	mainServer->sqlFinishQuery();

	if (!mainServer->sqlRequest("SELECT name, level, experience, job, face, hair, sex, zulies, saveTown, respawnMap, respawnId FROM characters WHERE id=%i", this->charInfo.id))
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
	this->inventory[0x00].amount = static_cast<qword_t>(::atol(row[7]));
	this->charInfo.respawnTownId = atoi(row[8]);
	this->entityInfo.mapId = atoi(row[9]);
	this->setPositionCurrent(mainServer->getMap(this->getMapId())->getRespawnPoint(atoi(row[10])));
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
	this->charInfo.statPoints = atoi(row[7]);
	this->charInfo.skillPoints = atoi(row[8]);

	mainServer->sqlFinishQuery();
	
	
	if(!mainServer->sqlRequest("SELECT * FROM inventory WHERE charId=%i", this->charInfo.id))
		return false;
	for(unsigned int i=0;i<mainServer->sqlGetRowCount();i++) {
		row = mainServer->sqlGetNextRow();
		byte_t slotId = atoi(row[1]);
		this->inventory[slotId].type = atoi(row[2]) / 10000;
		this->inventory[slotId].id = atoi(row[2]) % 10000;
		this->inventory[slotId].durability = atoi(row[3]);
		this->inventory[slotId].lifespan = atoi(row[4]);
		this->inventory[slotId].amount = atoi(row[5]);
		this->inventory[slotId].refine = atoi(row[6]);
	}
	mainServer->sqlFinishQuery();

	this->loadQuests();
	
	if (!mainServer->sqlRequest("SELECT basicSkills FROM character_skills WHERE id=%i", this->charInfo.id))
		return false;
	if (mainServer->sqlGetRowCount() != 1)
		return false;
	std::string str = std::string(mainServer->sqlGetNextRow()[0]);
	dword_t idx = 0x00;
	word_t skillId = 0x00;
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
	this->stats.curHP = this->getMaxHPW() * 10 / 100;
	this->status.buffs.clearBuff();
	this->combat.clear();

	byte_t respawnType = this->packet.getByte(0x00);
	if (respawnType == 0x01) {
		Map* curMap = mainServer->getMap(this->getMapId());
		const position_t& pos = curMap->getRespawnPoint(this->getPositionCurrent());
		return this->pakTelegate(this->getMapId(), pos);
	} else {
		Map* townMap = mainServer->getMap(this->charInfo.respawnTownId);
		return this->pakTelegate(townMap->getId(), townMap->getRespawnPoint("start"));
	}
	return true;
}

bool Player::pakRespawnTown() {
	this->charInfo.respawnTownId = this->getMapId();
	return true;
}

bool Player::pakUpdateEquipmentVisually() {
	bool result = true;
	for (byte_t i = 1; i <= PlayerInventory::Slots::SHIELD; i++) {
		if (i == 6) { //no item?
			continue;
		}
		result &= this->pakUpdateInventoryVisually(1, &i);
	}
	return result;
}

bool Player::pakUpdateInventoryVisually( const byte_t slotAmount, const byte_t* slotIds ) {
	Packet pak(PacketID::World::Response::UPDATE_INVENTORY);
	pak.addByte( slotAmount );
	for (byte_t i = 0; i<slotAmount; i++) {
		pak.addByte( slotIds[i] );
		pak.addWord( this->inventory[ slotIds[i] ].getPakHeader() );
		pak.addDWord( this->inventory[ slotIds[i] ].getPakData() );
	}
	bool result = this->sendData(pak);

	for(byte_t j=0;j<slotAmount;j++) {
		if (slotIds[j] == 0 || slotIds[j] >= PlayerInventory::Slots::SHIELD)
			continue;
		Packet pak( PacketID::World::Response::EQUIPMENT_CHANGE);
		pak.addWord(this->getLocalId());
		pak.addWord( slotIds[j] );
		pak.addDWord( this->inventory[slotIds[j]].getPakVisuality() );
		pak.addWord( this->getMovementSpeed() );
		this->sendToVisible( pak );
	}

	this->updateStats();

	return result;
}

bool Player::updateZulies(const qword_t newAmount) {
	this->inventory[0x00].amount = static_cast<dword_t>(newAmount);

	Packet pak(PacketID::World::Response::REWARD_ZULIES);
	pak.addQWord(this->inventory[0x00].amount);
	pak.addByte(0x00);
	return this->sendData(pak);
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

	if (!this->pakPlayerInfos() || !this->pakInventory() || !this->pakQuestData())
		return false;

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

bool Player::pakAssignID() {
	this->updateStats();

	QuestService::runQuest(this, ::makeQuestHash(mainServer->getZoneSTB()->getQuestString(this->getMapId()).c_str()));

	/*
	Packet pak(PacketID::World::Response::CHANGE_ABILITY);
	pak.addWord(StatType::PK_LEVEL);
	pak.addDWord(0x02);
	if(!this->sendData(pak))
		return false;
	*/

	Packet pak(PacketID::World::Response::ASSIGN_ID);
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
	pak.addDWord(this->getPlayerKillFlag());

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

	this->position.lastCheckTime.update();
	this->status.updateLastRegen();
	this->entityInfo.ingame = true;
	bool result = this->sendToVisible(pak);

	this->visibleSectors.clear();
	this->checkVisuality();
	this->setSector(mainServer->getMap(this->getMapId())->getSector(this->getPositionCurrent()));
	return result;
}

bool Player::pakTerrainCollision() {
	this->position.destination = position_t(this->packet.getFloat(0x00), this->packet.getFloat(0x04));
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
	this->combat.clear();
	Entity* entity = mainServer->getEntity(this->getMapId(), this->packet.getWord(0x00));
	if (entity != nullptr) {
		this->combat.setTarget(entity);
		this->combat.type = Combat::NORMAL;
	}
	this->setPositionDest(position_t(this->packet.getFloat(0x02), this->packet.getFloat(0x06)));
	return true;
}

bool Player::pakIncreaseAttribute() {
	byte_t statType = this->packet.getByte(0x00);
	word_t* statPointsREF = nullptr;
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
	byte_t stanceType = this->packet.getByte(0x00);
	switch(stanceType) {
		case 0x00:
			if(this->getStance().asBYTE() == Stance::WALKING) {
				this->setStance(Stance::RUNNING);
			}
			else if (this->getStance().asBYTE() == Stance::RUNNING) {
				this->setStance(Stance::WALKING);
			}
		break;
		case 0x01:
			if (this->getStance().asBYTE() == Stance::SITTING) {
				this->setStance(Stance::RUNNING);
				break;
			}
			if (this->getStance().asBYTE() != Stance::DRIVING && this->getStance().asBYTE() != Stance::SITTING) {
				this->setStance(Stance::SITTING);
			}
		break;
		case 0x02:
			//DRIVING
		break;
	}
	stanceType = this->getStance().asBYTE();

	this->updateStats();

	Packet pak(PacketID::World::Response::CHANGE_STANCE);
	pak.addWord(this->getLocalId());
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
	pak.addWord(player->getLocalId());
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
	pak.addWord((player->getTarget() == nullptr ? 0x00 : player->getTarget()->getLocalId()));
	switch (player->getStance().asBYTE()) {
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
	pak.addDWord( player->getCurrentHP() );
	pak.addDWord( player->getPlayerKillFlag() );
	pak.addDWord( player->getBuffsVisuality() );
	pak.addByte( player->getVisualTraits().sex );
	pak.addWord( player->getMovementSpeed() );
	pak.addWord( player->getAttackSpeed() );
	pak.addByte( 0x01 ); //Weight?

	pak.addDWord( player->getVisualTraits().faceStyle );
	pak.addDWord( player->getVisualTraits().hairStyle );
	
	pak.addDWord(player->inventory[PlayerInventory::Slots::HEADGEAR].getPakVisuality());
	pak.addDWord(player->inventory[PlayerInventory::Slots::ARMOR].getPakVisuality());
	pak.addDWord(player->inventory[PlayerInventory::Slots::GLOVES].getPakVisuality());
	pak.addDWord(player->inventory[PlayerInventory::Slots::SHOES].getPakVisuality());
	pak.addDWord(player->inventory[PlayerInventory::Slots::FACE].getPakVisuality());
	pak.addDWord(player->inventory[PlayerInventory::Slots::BACK].getPakVisuality());
	pak.addDWord(player->inventory[PlayerInventory::Slots::WEAPON].getPakVisuality());
	pak.addDWord(player->inventory[PlayerInventory::Slots::SHIELD].getPakVisuality());
	
	for (unsigned int i = PlayerInventory::Slots::ARROWS; i <= PlayerInventory::Slots::CANNONSHELLS; i++)
		pak.addWord( player->inventory[i].getPakHeader() );

	pak.addWord( player->getJob() );
	pak.addByte( player->getLevel() );

	for (unsigned int i = PlayerInventory::Slots::CART_FRAME; i <= PlayerInventory::Slots::CART_ABILITY; i++)
		pak.addDWord( player->inventory[i].id | 0x400 );

	pak.addWord( 0xcdcd ); //Z-Coord?
	pak.addDWord( 0x00 ); //weird dword_t, additional buffs/status?

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
	pak.addWord(npc->getLocalId());
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
	pak.addWord(npc->getTarget() != nullptr ? npc->getTarget()->getLocalId() : 0x00);
	pak.addByte( 0x00 ); //Stance
	pak.addDWord( npc->getCurrentHP() );
	pak.addDWord(npc->getPlayerKillFlag());
	pak.addDWord( npc->getBuffsVisuality() );
	pak.addWord(npc->getTypeId());
	if(npc->hasDialogId())
		pak.addWord(npc->getDialogId());
	else
		pak.addWord(npc->getTypeId() - 900);
	
	pak.addFloat(npc->getDirection());
	pak.addWord(0x00); //CLANFIELD OPEN/CLOSED FOR BURLAND (NPC: 1115)
	
	return this->sendData(pak);
}

bool Player::pakSpawnMonster(Monster* monster) {
	Packet pak(PacketID::World::Response::SPAWN_MONSTER);
	pak.addWord(monster->getLocalId());
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
	pak.addWord(monster->getTarget() != nullptr ? monster->getTarget()->getLocalId() : 0x00);
	pak.addByte(monster->getStance().asBYTE());
	pak.addDWord(monster->getCurrentHP());
	pak.addDWord(monster->getPlayerKillFlag()); //ENEMY
	pak.addDWord( monster->getBuffsVisuality() );
	pak.addWord(monster->getTypeId());
	pak.addWord(0x00); //Quest?
	return this->sendData(pak);
}

bool Player::pakShowMonsterHP() {
	const word_t localMonId = this->packet.getWord(0x00);
	Entity *entity = mainServer->getMap(this->getMapId())->getEntity(localMonId);
	if (!entity) {
		GlobalLogger::fatal("Selected monster was NOT found. Updating visuality...\n");
		this->refreshVisuality();
		ChatService::sendWhisper("Server", this, "Targeted monster wasn't found. Refreshing visuality...");
		return true;
	}
	if (entity->getEntityType() != Entity::TYPE_MONSTER) {
		GlobalLogger::warning("Entity of type %i was selected!\n", entity->getEntityType());
		return true;
	}
	Monster *mon = dynamic_cast<Monster*>(entity);
	if (this->isInDebugMode()) {
		ChatService::sendDebugMessage(this, "[%s] Map: %i, Position: %f, %f", mon->getName().c_str(), mon->getMapId(), mon->getPositionCurrent().x, mon->getPositionCurrent().y);
		if (mon->getSpawn() != nullptr) {
			const IFOSpawn* spawn = mon->getSpawn();
			ChatService::sendDebugMessage(this, "[%s] SpawnInfo(%f, %f) - Mobs spawned: %i/%i, Current Cycle: %i", mon->getName().c_str(), spawn->getPosition().x, spawn->getPosition().y, spawn->getCurrentlySpawned(), spawn->getMaxSimultanouslySpawned(), spawn->getCurrentSpawnId());
		}
	}
	return this->pakShowMonsterHP(mon);
}

bool Player::pakShowMonsterHP(Monster* mon) {
	Packet pak(PacketID::World::Response::SHOW_MONSTER_HP);
	pak.addWord(mon->getLocalId());
	pak.addDWord(mon->getCurrentHP());
	return this->sendData(pak);
}

bool Player::pakInitBasicAttack() {
	word_t localId = this->packet.getWord(0x00);

	//In case we target ourselves, do nothing
	if (this->getLocalId() == localId)
		return true; 

	Entity* entity = mainServer->getEntity(this->getMapId(), localId);
	if (!entity) {
		GlobalLogger::warning("Player %s tried to attack an invalid target!\n", this->getName().c_str());
		return true; //TODO
	}

	if(entity->getEntityType() == Entity::TYPE_MONSTER) {
		Monster* mon = dynamic_cast<Monster*>(entity);
		bool tmpResult = this->pakShowMonsterHP(mon);
		if (!tmpResult) {
			GlobalLogger::fault("Player %s couldn't receive %s (%i) HP!\n", this->getName().c_str(), mon->getName().c_str(), mon->getLocalId());
			return false;
		}
	}
	this->combat.type = Combat::NORMAL;
	this->setTarget(entity);
	return true;
}

bool Player::pakSpawnDrop( Drop* drop ) {
	Packet pak(PacketID::World::Response::SPAWN_DROP);
	pak.addFloat(drop->getCurrentX());
	pak.addFloat(drop->getCurrentY());
	pak.addWord(drop->getItem().getPakHeader());
	pak.addDWord(drop->getItem().amount);
	pak.addWord(drop->getLocalId());
	pak.addDWord(drop->getItem().getPakData() );
	//pak.addWord( (drop->getOwner() == nullptr ? 0x00 : drop->getOwner()->getClientId()) ); //OwnerClientId (?)
	return this->sendData(pak); 
}

bool Player::pakEquipmentChange() {
	byte_t sourceSlot = static_cast<byte_t>(this->packet.getWord(0x00));
	byte_t destSlot = static_cast<byte_t>(this->packet.getWord(0x02));
	if(destSlot == 0x00) {
		destSlot = this->getFreeInventorySlot( this->inventory[sourceSlot] );
	}
	if(destSlot == std::numeric_limits<byte_t>::max())
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
	byte_t slot = this->packet.getByte(0x00);
	if(!this->inventory[slot].isValid())
		return false;

	dword_t amount = this->packet.getDWord(0x01);
	if(amount > this->inventory[slot].amount)
		return true;

	Item toDrop = this->inventory[slot];
	toDrop.amount = amount;

	new Drop(this, toDrop, true);

	this->inventory[slot].amount -= amount;
	return this->pakUpdateInventoryVisually( 1, &slot );
}

bool Player::pakPickUpDrop() {

	word_t dropId = this->packet.getWord(0x00);
	Entity *entityDrop = mainServer->getEntity(this->getMapId(), dropId);
	if (!entityDrop || entityDrop->getEntityType() != Entity::TYPE_DROP) {
		GlobalLogger::warning("Player %s tried to pick a non-existing drop (%i)!\n", this->getName().c_str(), dropId);
		return true;
	}
	Drop* drop = dynamic_cast<Drop*>(entityDrop);

	this->combat.clear();
	this->position.destination = this->position.current;
	Packet pak(PacketID::World::Response::PICK_DROP);
	pak.addWord(drop->getLocalId());

	//In case the drop is not publicly available for everyone
	if(drop->getOwner() != nullptr && drop->getOwner() != this) {
		pak.addWord( PickDropMessage::NOT_OWNER );
		return this->sendData(pak);
	}
	//In case we're the owner, find a fitting inventory slot
	word_t inventorySlotId = 0x00;
	if(!drop->isZulyDrop()) {
		inventorySlotId = this->getFreeInventorySlot( drop->getItem() );

		//In case we don't have a free (suiting) slot, tell the client
		if(inventorySlotId == std::numeric_limits<byte_t>::max()) {
			pak.addByte( PickDropMessage::INVENTORY_FULL );
			return this->sendData(pak);
		}
	}
	dword_t previousAmount = this->inventory[inventorySlotId].amount;
	this->inventory[inventorySlotId] = drop->getItem();
	this->inventory[inventorySlotId].amount += previousAmount;
	
	pak.addByte( PickDropMessage::OKAY );
	pak.addWord( inventorySlotId ); //SlotId
	pak.addWord( this->inventory[inventorySlotId].getPakHeader() );
	if(drop->isZulyDrop())
		pak.addDWord( drop->getItem().amount );
	else
		pak.addDWord(this->inventory[inventorySlotId].getPakData() );
	
	delete drop;
	drop = nullptr;

	return this->sendData(pak); 

}

bool Player::pakNPCTrade() {
	word_t npcType = this->packet.getWord(0x00);
	NPC* npc = mainServer->getMap(this->getMapId())->getNPC(npcType);
	if (!npc || !npc->isNPC()) {
		GlobalLogger::warning("'%s' tried to sell items to an invalid NPC [npcType: %i]", this->getName().c_str(), npcType);
		return false;
	}
	else if (this->getPositionCurrent().distanceTo(npc->getPositionCurrent()) >= 1000) {
		GlobalLogger::warning("'%s' tried to sell items while being too far away from NPC '%s'", this->getName().c_str(), npc->getName().c_str());
	}

	byte_t boughtItemAmount = this->packet.getByte(0x02);
	byte_t soldItemAmount = this->packet.getByte(0x03);

	ChatService::sendWhisper("Server", this, "Buy: %i, Sell: %i", boughtItemAmount, soldItemAmount);

	Packet pak(PacketID::World::Response::NPC_TRADE);
	pak.addQWord(this->inventory[0].amount);
	pak.addByte(0x00);

	for (byte_t i = 0; i < soldItemAmount; i++) {
		word_t itemOffset = 8 + (boughtItemAmount * 4) + (i * 3);
		byte_t slot = this->packet.getByte(itemOffset);
		word_t amount = this->packet.getWord(itemOffset + 1);

		Item itemToSell(this->inventory[slot]);
		if (itemToSell.amount > amount) {
			GlobalLogger::warning("'%s' tried to sell item [%i;%i] %i times, but has only %i of it.", this->getName().c_str(), itemToSell.type, itemToSell.id, amount, itemToSell.amount);
			continue;
		}
		itemToSell.amount = amount;
		dword_t totalPrice = mainServer->getSellPrice(itemToSell);

		this->inventory[0x00].amount += totalPrice;
		this->inventory[slot].amount -= amount;
	}

	return true;
}


bool Player::pakLearnSkill() {
	const word_t skillId = this->packet.getWord(0x00);
	Skill* skillToLearn = mainServer->getSkill(skillId);

	Packet pak(PacketID::World::Response::LEARN_SKILL);

	byte_t state = PlayerSkill::LEARN_SUCCESS;
	if (!skillToLearn) {
		state = PlayerSkill::LEARN_INVALID_SKILL;
	}
	else if (skillToLearn->getRequiredPointsPerLevelup() < this->charInfo.skillPoints) {
		state = PlayerSkill::LEARN_NEEDS_SKILLPOINTS;
	}
	else {
		for (unsigned int i = 0; i < Skill::REQUIRED_SKILL_MAX; i++) {
			Skill* reqSkill = mainServer->getSkill(skillToLearn->getRequiredSkillID(i));
			if (!reqSkill)
				break;
			if (this->isSkillLearned(reqSkill) <= 0x01) {
				state = PlayerSkill::LEARN_REQUIRED_SKILL;
				break;
			}
		}
		if (state == PlayerSkill::LEARN_SUCCESS) {
			for (unsigned int j = 0; j < 2; j++) {
				if (this->getStatType<word_t>(skillToLearn->getRequiredConditionType(j)) < skillToLearn->getRequiredConditionAmount(j)) {
					state = PlayerSkill::LEARN_NEEDS_ABILITY;
					break;
				}
			}
		}
	}
	word_t skillSlot = PlayerSkill::PLAYER_MAX_SKILLS - 1;
	if (state == PlayerSkill::LEARN_SUCCESS) {
		const word_t pageStart = Skill::getPage(skillToLearn);
		const word_t pageEnd = pageStart + PlayerSkill::PAGE_SIZE;
		for (unsigned int i = pageStart; i < pageEnd; i++) {
			if (this->skills[i] == nullptr) {
				this->skills[i] = skillToLearn;
				this->charInfo.skillPoints -= skillToLearn->getRequiredPointsPerLevelup();
				skillSlot = i;
				this->updateStats();
			}
		}
		if (skillSlot == PlayerSkill::PLAYER_MAX_SKILLS - 1) {
			state = PlayerSkill::LEARN_NO_SLOTS_LEFT;
		}
	}
	pak.addByte(state);
	pak.addWord(skillSlot);
	pak.addWord(skillId);
	pak.addWord(this->charInfo.skillPoints);
	return this->sendData(pak);
}

bool Player::pakIncreaseSkillLevel() {
	const byte_t skillPos = this->packet.getByte(0x00);
	Skill* wantedSkill = this->skills[skillPos];
	if (!wantedSkill) {
		GlobalLogger::warning("Player %s tried to level a non-existing skill (Slot: %i)!\n", this->getName().c_str(), skillPos);
		return true;
	}
	Skill* nextSkill = mainServer->getSkill(wantedSkill->getId() + 1);

	Packet pak(PacketID::World::Response::INCREASE_SKILL_LEVEL);
	if (!nextSkill) {
		pak.addByte(PlayerSkill::UPGRADE_FAILED);
		return this->sendData(pak);
	}

	if (this->getZulies() < nextSkill->getRequiredZulies()) {
		pak.addByte(PlayerSkill::UPGRADE_NEEDS_ZULIES);
		return this->sendData(pak);
	}
	
	if (this->charInfo.skillPoints < nextSkill->getRequiredPointsPerLevelup()) {
		pak.addByte(PlayerSkill::UPGRADE_NEEDS_SKILLPOINTS);
		return this->sendData(pak);
	}

	for (unsigned int i = 0; i < Skill::REQUIRED_SKILL_MAX; i++) {
		if (this->getStatType<word_t>(nextSkill->getRequiredConditionType(i)) < nextSkill->getRequiredConditionAmount(i)) {
			pak.addByte(PlayerSkill::UPGRADE_NEEDS_ABILITY);
			return this->sendData(pak);
		}
		if (nextSkill->getRequiredSkillID(i) > 0) {
			Skill* reqSkill = mainServer->getSkill(nextSkill->getRequiredSkillID(i));
			if (this->isSkillLearned(reqSkill) < 0x02) {
				pak.addByte(PlayerSkill::UPGRADE_REQUIRED_SKILL);
				return this->sendData(pak);
			}
		} 
	}
	this->skills[skillPos] = nextSkill;
	this->updateStats();

	pak.addByte(PlayerSkill::UPGRADE_SUCCESS);
	pak.addByte(skillPos);
	pak.addWord(this->packet.getWord(0x01));
	pak.addWord(this->charInfo.skillPoints);
	return this->sendData(pak);
}

bool Player::pakExecuteSkill() {
	word_t entityId = this->packet.getWord(0x00);

	Entity* enemy = this->getVisibleEntity(entityId);
	if (!enemy || enemy->getEntityType() == Entity::TYPE_DROP) {
		GlobalLogger::warning("Player %s tried to execute a skill on an invalid enemy\n", this->getName().c_str());
		return false;
	}

	byte_t skillSlot = this->packet.getByte(0x02);
	if (skillSlot >= this->skills.capacity() || this->skills[skillSlot] == nullptr) {
		GlobalLogger::warning("Player %s tried to execute an invalid skill (SlotID: %i)\n", this->getName().c_str(), skillSlot);
		return false;
	}

	GlobalLogger::debug("%s attacks %s with %s!\n", this->getName().c_str(), enemy->getName().c_str(), this->skills[skillSlot]->getName().c_str());

	if (enemy->getEntityType() == Entity::TYPE_MONSTER) {
		this->pakShowMonsterHP(dynamic_cast<Monster*>(enemy));
	}

	this->combat.target = enemy;
	this->combat.skill = this->skills[skillSlot];
	this->combat.type = Combat::SKILL;

	//To mark it as 'invalid' for further processing work; FOR NOW!
	this->animation = nullptr;

	Packet pak(PacketID::World::Response::SKILL_ATTACK);
	pak.addWord(this->getLocalId());
	pak.addWord(this->getTarget()->getLocalId());
	pak.addWord(this->combat.skill->getIdBasic());
	pak.addWord(0x00);
	pak.addFloat(this->getTarget()->getPositionCurrent().x);
	pak.addFloat(this->getTarget()->getPositionCurrent().y);

	return this->sendToVisible(pak);
}

bool Player::pakQuickbarAction() {
	byte_t slotId = this->packet.getByte(0x00);
	word_t mappedValue = this->packet.getWord(0x01);

	if (slotId >= PlayerSkill::PLAYER_MAX_QUICKBAR) {
		GlobalLogger::warning("Player %s tried to add a skill %i to quickslot %i", this->getName().c_str(), mappedValue, slotId);
		return true;
	}
	this->quickbar[slotId] = mappedValue;
	Packet pak(PacketID::World::Response::QUICKBAR_ACTION);
	pak.addByte(slotId);
	pak.addWord(mappedValue);
	return this->sendData(pak);
}

bool Player::pakConsumeItem() {
	const byte_t inventorySlot = this->packet.getByte(0x00);
	if (!this->inventory[inventorySlot].isValid() || this->inventory[inventorySlot].amount == 0x00 || this->inventory[inventorySlot].amount > 999)
		return true;
	const STBEntry* consumeEntry = mainServer->getConsumable(this->inventory[inventorySlot]);
	
	const dword_t valueNeeded = consumeEntry->getColumnAsInt(ConsumeSTB::STAT_VALUE_NEEDED);

	//Nothing should happen when the requirement is not met.
	if (this->getStatType<dword_t>(consumeEntry->getColumn<word_t>(ConsumeSTB::STAT_TYPE_NEEDED)) < valueNeeded) {
		return true;
	}
	const byte_t executionType = consumeEntry->getColumn<byte_t>(ConsumeSTB::EXECUTION_TYPE);
	
	bool successfulExecution = false;
	switch (executionType) {
		case 0x00: //Consume directly
		{
			//Check whether a local stat needs to be adjusted (e.g. Health via pot)
			word_t abilityType = consumeEntry->getColumn<word_t>(ConsumeSTB::STAT_TYPE_ADD);
			if (abilityType == 0x00) {
				//if there's no ability to change, assume it's a skill
				this->addSkill(mainServer->getSkill(consumeEntry->getColumn<word_t>(ConsumeSTB::STAT_VALUE_ADD)));
			}
			else {
				this->changeAbility(consumeEntry->getColumn<word_t>(ConsumeSTB::STAT_TYPE_ADD), consumeEntry->getColumnAsInt(ConsumeSTB::STAT_VALUE_ADD), OperationService::OPERATION_ADDITION, false);
			}
		}
		break;
		case 0x01: //Consume over time
		{
			byte_t statusRef = consumeEntry->getColumn<byte_t>(ConsumeSTB::STATUS_STB_REFERENCE);
			if (statusRef == 0x00) {
				//if there's no actual status operation, consume it immediately as whole (e.g. pills)
				word_t statAffected = consumeEntry->getColumn<word_t>(ConsumeSTB::STAT_TYPE_ADD);
				if (statAffected == 0) { //Add a skill, I guess.
					this->addSkill(mainServer->getSkill(consumeEntry->getColumn<word_t>(ConsumeSTB::STAT_VALUE_ADD)));
				}
				else {
					this->changeAbility(consumeEntry->getColumn<word_t>(ConsumeSTB::STAT_TYPE_ADD), consumeEntry->getColumnAsInt(ConsumeSTB::STAT_VALUE_ADD), OperationService::OPERATION_ADDITION, false);
				}
			}
			else {
				//If there's a status entry, look up how much of a given stat will be added over each second
				const STBEntry* statusEntry = mainServer->getStatus(statusRef);
				ConsumedItem item(consumeEntry->getColumn<word_t>(ConsumeSTB::STAT_TYPE_ADD), consumeEntry->getColumn<dword_t>(ConsumeSTB::STAT_VALUE_ADD), statusEntry->getColumnAsInt(StatusSTB::COLUMN_VALUE_INCREASE_FIRST));
				this->consumedItems.push_back(item);
			}
		}
		break;
		case 0x05: //Buff
			this->combat.skill = mainServer->getSkill(consumeEntry->getColumn<word_t>(ConsumeSTB::STAT_VALUE_ADD));
		break;
		default:
			std::cout << "ItemID " << this->inventory[inventorySlot].id << " has an unknown execution type: " << executionType << "\n";
	}
	this->inventory[inventorySlot].amount -= 1;

	Packet visualityPak(PacketID::World::Response::USE_CONSUMABLE);
	visualityPak.addWord(this->getLocalId());
	visualityPak.addWord(this->inventory[inventorySlot].id);
	if (!this->sendToVisible(visualityPak))
		return false;

	Packet pak(PacketID::World::Response::USE_CONSUMABLE);
	pak.addWord(this->getLocalId());
	pak.addWord(this->inventory[inventorySlot].id);
	pak.addByte(inventorySlot);
	return this->sendData(pak);
}

bool Player::pakTelegate() {
	word_t telegateId = this->packet.getWord(0x00);
	Telegate& gate = mainServer->getGate(telegateId);

	float distanceToGate = this->getPositionCurrent().distanceTo(gate.getSourcePosition());

	this->setPositionCurrent(gate.getDestPosition());
	return this->pakTelegate(gate.getDestMap(), gate.getDestPosition());
}

bool Player::pakTelegate(const word_t mapId, const position_t& pos) {
	for(unsigned int i=0;i<this->visibleSectors.size();i++) {
		this->removeSectorVisually(this->visibleSectors.getValueAtPosition(i));
	}
	this->visibleSectors.clear();

	Packet pak(PacketID::World::Response::TELEGATE);
	pak.addWord(this->getLocalId());
	pak.addWord(mapId);
	pak.addFloat(pos.x);
	pak.addFloat(pos.y);
	pak.addWord(0x01);
	if(!this->sendData(pak))
		return false;

	this->entityInfo.ingame = false;
	this->combat.clear();

	this->setPositionCurrent(pos);
	this->setPositionDest(pos);
	mainServer->changeToMap(this, mapId);

	return true;
}

bool Player::handlePacket() {
	GlobalLogger::debug("[IN] Packet 0x%x (Length: %i) from '%s'\n", this->packet.getCommand(), this->packet.getLength(), this->getName().c_str());
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

		case PacketID::World::Request::INCREASE_SKILL_LEVEL:
			return this->pakIncreaseSkillLevel();

		case PacketID::World::Request::LEARN_SKILL:
			return this->pakLearnSkill();

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

		case PacketID::World::Response::QUEST_ACTION:
			return this->pakQuestAction();

		case PacketID::World::Request::QUICKBAR_ACTION:
			return this->pakQuickbarAction();

		case PacketID::World::Request::RESPAWN_TOWN:
			return this->pakRespawnTown();

		case PacketID::World::Request::SET_EMOTION:
			return this->pakSetEmotion();

		case PacketID::World::Request::SHOUT_CHAT:
			return this->pakShoutChat();

		case PacketID::World::Request::SHOW_MONSTER_HP:
			return this->pakShowMonsterHP();

		case PacketID::World::Request::SKILL_ATTACK:
			return this->pakExecuteSkill();

		case PacketID::World::Request::TELEGATE:
			return this->pakTelegate();

		case PacketID::World::Request::TERRAIN_COLLISION:
			return this->pakTerrainCollision();

		case PacketID::World::Request::USE_CONSUMABLE:
			return this->pakConsumeItem();
	}
	return true;
}