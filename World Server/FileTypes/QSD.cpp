#include "QSD.h"
#include "..\WorldServer.h"
#include "..\Entity\Monster.h"

#ifdef __ROSE_USE_VFS__

#ifdef __ROSE_DEBUG__
QSD::QSD(word_t newId, VFS* pVFS, const char* filePath) {
	this->id = newId;
#else
QSD::QSD(VFS* pVFS, const char* filePath) {
	this->id = 0x00;

#endif
	if (strlen(filePath) > 0) {
		this->filePath = filePath;
		VFSData vfsData; pVFS->readFile(filePath, vfsData);
		CMyBufferedFileReader<char> reader(vfsData.data, vfsData.data.size());
		this->read(reader);
	}
}
#else
QSD::QSD(const char* filePath) {
	this->id = 0x00;
	this->filePath = filePath;
	CMyFile file(this->filePath.c_str(), "rb");
	if (file.exists()) {
		this->read(file);
	}
}
#endif

template<class FileType> void QSD::read(FileType& file) {
	file.read<DWORD>(); //FileVersion
	dword_t blockCount = file.read<DWORD>();

	char name[0x200] = { 0x00 };
	file.readLengthThenString<WORD>(name);

	this->internalName = name;

	for (unsigned int i = 0; i < blockCount; i++) {

		dword_t recordCount = file.read<DWORD>();

		file.readLengthThenString<WORD>(name);
		QuestEntry* previousEntry = nullptr;
		byte_t checkNext = 0x00;
		for (unsigned int j = 0; j < recordCount; j++) {

			checkNext = file.read<BYTE>() > 0; //CheckNEXT ? 
			dword_t conditionCount = file.read<DWORD>();
			dword_t actionCount = file.read<DWORD>();

			file.readLengthThenString<WORD>(name);
			QuestEntry* newEntry = new QuestEntry(::makeQuestHash(name), conditionCount, actionCount);
#ifdef __ROSE_DEBUG__
			newEntry->parent = this;
			newEntry->qsdName = this->filePath;
#endif
			newEntry->questName = std::string(name);
			newEntry->checkNextTrigger = checkNext;
			char* tmpBuf = nullptr;
			for (unsigned int k = 0; k < conditionCount; k++) {
				dword_t condLen = file.read<DWORD>();
				file.setPosition(file.getPosition() - 4);

				file.readAndAlloc(&tmpBuf, condLen);

				AbstractQuestInfo* info = reinterpret_cast<AbstractQuestInfo*>(tmpBuf);
				switch (info->header.operationType) {
					case 0x06:
						(reinterpret_cast<QuestCondition006*>(info))->radius *= 100;
					break;
					case 0x1E:
						(reinterpret_cast<QuestCondition021*>(info))->radius *= 100;
					break;
				}
				Trackable<char> newCond(tmpBuf, condLen);
				newEntry->addCondition(&newCond);

				delete[] tmpBuf;
				tmpBuf = nullptr;
			}
			for (unsigned int k = 0; k < actionCount; k++) {
				dword_t actLen = file.read<DWORD>();
				file.setPosition(file.getPosition() - 4);
				file.readAndAlloc(&tmpBuf, actLen);

				AbstractQuestInfo* info = reinterpret_cast<AbstractQuestInfo*>(tmpBuf);
				switch (info->header.operationType) {
					case 0x08:
						(reinterpret_cast<QuestReward008*>(info))->radius *= 100;
					break;
				}

				Trackable<char> newAct(tmpBuf, actLen);
				newEntry->addAction(&newAct);

				delete[] tmpBuf;
				tmpBuf = nullptr;
			}
			newEntry->questId = this->id;
			this->questData.insert( std::pair<const DWORD, QuestEntry*>(newEntry->getQuestHash(), newEntry) );
			newEntry->previousQuest = previousEntry;
			if (previousEntry) {
				previousEntry->nextQuest = newEntry;
			}
			previousEntry = newEntry;
		}
	}
}

const dword_t makeQuestHash(const char* qsdname) {
	//Thanks to Drakia / Brett19 / ExJam for this particular codepiece :>
	const unsigned long keys[256] = {
		0x697A5, 0x6045C, 0xAB4E2, 0x409E4, 0x71209, 0x32392, 0xA7292, 0xB09FC, 0x4B658, 0xAAAD5, 0x9B9CF, 0xA326A, 0x8DD12, 0x38150, 0x8E14D, 0x2EB7F,
		0xE0A56, 0x7E6FA, 0xDFC27, 0xB1301, 0x8B4F7, 0xA7F70, 0xAA713, 0x6CC0F, 0x6FEDF, 0x2EC87, 0xC0F1C, 0x45CA4, 0x30DF8, 0x60E99, 0xBC13E, 0x4E0B5,
		0x6318B, 0x82679, 0x26EF2, 0x79C95, 0x86DDC, 0x99BC0, 0xB7167, 0x72532, 0x68765, 0xC7446, 0xDA70D, 0x9D132, 0xE5038, 0x2F755, 0x9171F, 0xCB49E,
		0x6F925, 0x601D3, 0x5BD8A, 0x2A4F4, 0x9B022, 0x706C3, 0x28C10, 0x2B24B, 0x7CD55, 0xCA355, 0xD95F4, 0x727BC, 0xB1138, 0x9AD21, 0xC0ACA, 0xCD928,
		0x953E5, 0x97A20, 0x345F3, 0xBDC03, 0x7E157, 0x96C99, 0x968EF, 0x92AA9, 0xC2276, 0xA695D, 0x6743B, 0x2723B, 0x58980, 0x66E08, 0x51D1B, 0xB97D2,
		0x6CAEE, 0xCC80F, 0x3BA6C, 0xB0BF5, 0x9E27B, 0xD122C, 0x48611, 0x8C326, 0xD2AF8, 0xBB3B7, 0xDED7F, 0x4B236, 0xD298F, 0xBE912, 0xDC926, 0xC873F,
		0xD0716, 0x9E1D3, 0x48D94, 0x9BD91, 0x5825D, 0x55637, 0xB2057, 0xBCC6C, 0x460DE, 0xAE7FB, 0x81B03, 0x34D8F, 0xC0528, 0xC9B59, 0x3D260, 0x6051D,
		0x93757, 0x8027F, 0xB7C34, 0x4A14E, 0xB12B8, 0xE4945, 0x28203, 0xA1C0F, 0xAA382, 0x46ABB, 0x330B9, 0x5A114, 0xA754B, 0xC68D0, 0x9040E, 0x6C955,
		0xBB1EF, 0x51E6B, 0x9FF21, 0x51BCA, 0x4C879, 0xDFF70, 0x5B5EE, 0x29936, 0xB9247, 0x42611, 0x2E353, 0x26F3A, 0x683A3, 0xA1082, 0x67333, 0x74EB7,
		0x754BA, 0x369D5, 0x8E0BC, 0xABAFD, 0x6630B, 0xA3A7E, 0xCDBB1, 0x8C2DE, 0x92D32, 0x2F8ED, 0x7EC54, 0x572F5, 0x77461, 0xCB3F5, 0x82C64, 0x35FE0,
		0x9203B, 0xADA2D, 0xBAEBD, 0xCB6AF, 0xC8C9A, 0x5D897, 0xCB727, 0xA13B3, 0xB4D6D, 0xC4929, 0xB8732, 0xCCE5A, 0xD3E69, 0xD4B60, 0x89941, 0x79D85,
		0x39E0F, 0x6945B, 0xC37F8, 0x77733, 0x45D7D, 0x25565, 0xA3A4E, 0xB9F9E, 0x316E4, 0x36734, 0x6F5C3, 0xA8BA6, 0xC0871, 0x42D05, 0x40A74, 0x2E7ED,
		0x67C1F, 0x28BE0, 0xE162B, 0xA1C0F, 0x2F7E5, 0xD505A, 0x9FCC8, 0x78381, 0x29394, 0x53D6B, 0x7091D, 0xA2FB1, 0xBB942, 0x29906, 0xC412D, 0x3FCD5,
		0x9F2EB, 0x8F0CC, 0xE25C3, 0x7E519, 0x4E7D9, 0x5F043, 0xBBA1B, 0x6710A, 0x819FB, 0x9A223, 0x38E47, 0xE28AD, 0xB690B, 0x42328, 0x7CF7E, 0xAE108,
		0xE54BA, 0xBA5A1, 0xA09A6, 0x9CAB7, 0xDB2B3, 0xA98CC, 0x5CEBA, 0x9245D, 0x5D083, 0x8EA21, 0xAE349, 0x54940, 0x8E557, 0x83EFD, 0xDC504, 0xA6059,
		0xB85C9, 0x9D162, 0x7AEB6, 0xBED34, 0xB4963, 0xE367B, 0x4C891, 0x9E42C, 0xD4304, 0x96EAA, 0xD5D69, 0x866B8, 0x83508, 0x7BAEC, 0xD03FD, 0xDA122
	};
	register unsigned long result = 0xDEADC0DE;
	register unsigned long tkey = 0x7FED7FED;
	register char c = 0;
	while (*(qsdname))
	{
		c = (*(qsdname)<'a' || *(qsdname)>'z') ? *(qsdname) : *(qsdname)-32;
		result += tkey;
		tkey *= 0x21;
		result ^= keys[c];
		tkey += result + c + 3;
		qsdname++;
	}
	return result;
}

void PlayerQuest::setSwitch(const word_t switchBit, const dword_t value) {
	if (switchBit >= sizeof(DWORD)* 8) //32 Bits
		return;
	if (value == 0x00) {
		this->leverBYTE[switchBit >> 3] &= ~(1 << (switchBit & 0x07));
	}
	else {
		this->leverBYTE[switchBit >> 3] |= (1 << (switchBit & 0x07));
	}
}

template<class _Ty> _Ty QuestService::rewardOperation(Entity* entity, const _Ty& basicValue, const byte_t operation) {
	_Ty result = 0;
	switch (operation) {
		case 0x00:
			result = (basicValue + 0x1E);
			result *= ((entity->getCharm() + 10) * 100 * 20);
			result = result / (entity->getLevel() + 70) / 30000;
			result += basicValue;
		break;
		case 0x01:
			result = (entity->getLevel() + 3) * basicValue;
			result *= (entity->getCharm() >> 1) + entity->getLevel() + 0x28;
			result *= 100;
			result /= 10000;
		break;
		case 0x02:
			return basicValue;
		break;
		case 0x03:
		case 0x05:
			result += (basicValue + 0x14);
			result *= (entity->getCharm() + 10) * 100;
			result *= 20; //FAME ?
			result /= (entity->getLevel() + 0x46);
			result /= 30000;
			result += basicValue;
		break;
		case 0x04:
			result = basicValue + 2;
			result *= (entity->getCharm() + entity->getLevel() + 0x28);
			result *= 100 * 40;
			result /= 0x222E0;
		break;
		case 0x06:
			result = basicValue + 0x1E;
			result *= (entity->getCharm() + entity->getLevel());
			result *= 100 * 20;
			result /= 0x2DC6C0;
			result += basicValue;
		break;
	}
	return result;
}

const char* QuestService::getAbilityTypeName(byte_t abilityType) {
	switch (abilityType) {
		case StatType::LEVEL:
			return "Level";
		case StatType::ATTACK_POWER:
			return "Attackpower";
		case StatType::ATTACK_SPEED:
			return "Attackspeed";
		case StatType::DEFENSE_PHYSICAL:
			return "Defense";
		case StatType::DEFENSE_MAGICAL:
			return "Magic Defense";
		case StatType::CHARM:
			return "Charm";
	}
	return "UNKNOWN!";
}

dword_t QuestService::currentQuestId;

const dword_t QuestService::runQuest(Entity* entity, const dword_t questHash) {
#ifndef __ROSE_QSD_DEBUG__
	QuestEntry* quest = mainServer->getQuest(questHash);
	if (!quest)
		return 0x00;
	//CHECK 0x1dde1a7e == "5033-32"
	bool continueWithActions = true;
	bool actionSuccess = false;
	do {
		continueWithActions = true;
		QuestTrans trans(quest->getQuestHash(), entity);
		QuestService::currentQuestId = quest->getQuestId(); //Just for debugging purposes
		for (unsigned int i = 0; i < quest->conditions.size(); i++) {
			Trackable<char>& data = quest->conditions[i];
			const AbstractQuestInfo* conditionHeader = reinterpret_cast<const AbstractQuestInfo*>(data.getData());
			if (!QuestService::checkCondition(&trans, conditionHeader)) {
				continueWithActions = false;
				break;
			}
		}
		if (continueWithActions) {
			actionSuccess = true;
			for (unsigned int i = 0; i < quest->actions.size(); i++) {
				Trackable<char>& data = quest->actions[i];
				const AbstractQuestInfo* actionHeader = reinterpret_cast<const AbstractQuestInfo*>(data.getData());
				if (!QuestService::applyActions(&trans, actionHeader, true)) {
					actionSuccess = false;
					break;
				}
			}
			if (actionSuccess) {
				for (unsigned int i = 0; i < quest->actions.size(); i++) {
					Trackable<char>& data = quest->actions[i];
					const AbstractQuestInfo* actionHeader = reinterpret_cast<const AbstractQuestInfo*>(data.getData());
					QuestService::applyActions(&trans, actionHeader, false);
				}
				return quest->getQuestHash();
			}
		}
	} while ((quest = quest->getNextQuest()) != nullptr && !continueWithActions);
#endif //__ROSE_QSD_DEBUG__
	return 0x00;
}


const QuestService::CONDITION_FUNCTION_PTR QuestService::conditions[50] {
	checkSelectedQuest,
	checkQuestVariables,
	checkQuestVariables,
	checkUserVariables,
	checkItemAmount,
	checkPartyLeaderAndLevel, //0x05
	checkDistanceFromPoint,
	checkWorldTime,
	checkRemainingTime,
	checkSkill,
	checkRandomPercentage, //0x0A (10)
	checkObjectVar,
	checkEventObject,
	checkNPCVar,
	checkSwitch,
	checkPartyMemberCount, // 0x0F (15)
	checkMapTime,
	checkNPCVarDifferences,
	checkServerTimeMonth,
	checkServerTimeWeekday,
	checkTeamId, // 0x14 (20)
	checkDistanceFromCenter,
	checkChannelNumber,
	checkIsClanMember,
	checkClanInternalPosition,
	checkClanContribution, //0x19 (25)
	checkClanLevel,
	checkClanScore,
	checkClanMoney,
	checkClanMemberCount,
	checkClanSkill, //0x1E (30)
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN,
	checkUNKNOWN
};

#ifndef __ROSE_QSD_DEBUG__
bool QuestService::checkCondition(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (!trans || !conditionHeader)
		return false;

	return QuestService::conditions[conditionHeader->header.operationType & 0xFF](trans, conditionHeader);
	
	
	//Oldschool
	/*
	switch (conditionHeader->header.operationType) {
		case 0x00:
			return QuestService::checkSelectedQuest(trans, conditionHeader);
		break;
		case 0x01:
		case 0x02:
			return QuestService::checkQuestVariables(trans, conditionHeader);
		case 0x03:
			return QuestService::checkUserVariables(trans, conditionHeader);
		case 0x04:
			return QuestService::checkItemAmount(trans, conditionHeader);
		case 0x05:
			return QuestService::checkPartyLeaderAndLevel(trans, conditionHeader);
		case 0x06:
			return QuestService::checkDistanceFromPoint(trans, conditionHeader);
		case 0x07:
			return QuestService::checkWorldTime(trans, conditionHeader);
		case 0x08:
			return QuestService::checkRemainingTime(trans, conditionHeader);
		case 0x09:
			return QuestService::checkSkill(trans, conditionHeader);
		case 0x0A:
			return QuestService::checkRandomPercentage(trans, conditionHeader);
		case 0x0B:
			return QuestService::checkObjectVar(trans, conditionHeader);
		case 0x0C:
			return QuestService::checkEventObject(trans, conditionHeader);
		case 0x0D:
			return QuestService::checkNPCVar(trans, conditionHeader);
		case 0x0E:
			return QuestService::checkSwitch(trans, conditionHeader);
		case 0x0F:
			return QuestService::checkPartyMemberCount(trans, conditionHeader);
		case 0x10:
			return QuestService::checkMapTime(trans, conditionHeader);
		case 0x11:
			return QuestService::checkNPCVarDifferences(trans, conditionHeader);
		case 0x12:
			return QuestService::checkServerTimeMonth(trans, conditionHeader);
		case 0x13:
			return QuestService::checkServerTimeWeekday(trans, conditionHeader);
		case 0x14:
			return QuestService::checkTeamId(trans, conditionHeader);
		case 0x15:
			return QuestService::checkDistanceFromCenter(trans, conditionHeader);
		case 0x16:
			return QuestService::checkChannelNumber(trans, conditionHeader);
		case 0x17:
			return QuestService::checkIsClanMember(trans, conditionHeader);
		case 0x18:
			return QuestService::checkClanInternalPosition(trans, conditionHeader);
		case 0x19:
			return QuestService::checkClanContribution(trans, conditionHeader);
		case 0x1A:
			return QuestService::checkClanLevel(trans, conditionHeader);
		case 0x1B:
			return QuestService::checkClanScore(trans, conditionHeader);
		case 0x1C:
			return QuestService::checkClanMoney(trans, conditionHeader);
		case 0x1D:
			return QuestService::checkClanMemberCount(trans, conditionHeader);
		case 0x1E:
			return QuestService::checkClanSkill(trans, conditionHeader);
	}
	*/
}

bool QuestService::checkSelectedQuest(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition000* cond = reinterpret_cast<const QuestCondition000*>(conditionHeader);
	if (player->searchAndSelectQuest(cond->questId)) {
		trans->selectedQuest = player->getSelectedQuest();
	}
	return (trans->selectedQuest != nullptr);
}

bool QuestService::checkQuestVariables(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition001* cond = reinterpret_cast<const QuestCondition001*>(conditionHeader);
	for (unsigned int i = 0; i < cond->dataCount; i++) {
		const QuestCheckVar* data = reinterpret_cast<const QuestCheckVar*>(&cond->data[i]);
		word_t questVar = player->getQuestVariable(data->varType, data->varNum);
		if (!OperationService::checkOperation(questVar, data->amount, data->operation))
			return false;
	}
	return true;
}

bool QuestService::checkUserVariables(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	const QuestCondition003* cond = reinterpret_cast<const QuestCondition003*>(conditionHeader);
	for (unsigned int i = 0; i < cond->dataCount; i++) {
		const QuestCheckAbility* data = reinterpret_cast<const QuestCheckAbility*>(&cond->data[i]);
		dword_t value = trans->questTriggerCauser->getStatType<DWORD>(static_cast<WORD>(data->abilityType));
		if (!OperationService::checkOperation(value, data->amount, data->operation))
			return false;
	}
	return true;
}

bool QuestService::checkItemAmount(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (!trans->questTriggerCauser || trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition004* cond = reinterpret_cast<const QuestCondition004*>(conditionHeader);
	for (unsigned int i = 0; i < cond->dataCount; i++) {
		const QuestCheckItem* data = reinterpret_cast<const QuestCheckItem*>(&cond->data[i]);
		Item tmpItem(data->itemId);
		if (data->itemSlot >= ItemType::FACE && data->itemSlot <= ItemType::SHIELD) {
			Item eqItem = player->getItemFromInventory(static_cast<WORD>(data->itemSlot));
			if (eqItem.getPakHeader() != tmpItem.getPakHeader())
				return false;
		}
		if (tmpItem.type == ItemType::QUEST) {
			Item item = player->getQuestItem(data->itemId);
			//If there's no item in our current quest, treat it as if there were 0; otherwise nothing happens 
			//if (!item.isValid())
			//	return false;
			if (!OperationService::checkOperation(item.amount, data->amount, data->operation))
				return false;
		}
		else {
			//TODO
			try {
				throw TraceableException("[QUEST: %i] ITEMTYPE NOT IMPLEMENTED: %i", QuestService::currentQuestId, data->itemId);
			}
			catch (std::exception& ex) { std::cout << ex.what() << "\n"; }
		}
	}
	return true;
}

bool QuestService::checkPartyLeaderAndLevel(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (!trans->questTriggerCauser || trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition005* cond = reinterpret_cast<const QuestCondition005*>(conditionHeader);
	try {
		throw TraceableException("[QUEST: %i] PARTY NOT IMPLEMENTED: isLeader = %i, PartyLevel = %i", QuestService::currentQuestId, cond->isLeader, cond->level);
	}
	catch (std::exception& ex) { std::cout << ex.what() << "\n"; }
	return false;
}


bool QuestService::checkDistanceFromPoint(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (!trans->questTriggerCauser)
		return false;
	const QuestCondition006* cond = reinterpret_cast<const QuestCondition006*>(conditionHeader);
	if (trans->questTriggerCauser->getMapId() != cond->mapId)
		return false;
	position_t wantedPos(cond->x, cond->y);
	if (wantedPos.distanceTo(trans->questTriggerCauser->getPositionCurrent()) > cond->radius)
		return false;
	return true;
}

bool QuestService::checkWorldTime(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	//I guess it would trigger only once if we didn't do this.
	//byte_t would be too little as value (-> once every 4-5 minutes)
	word_t worldTime = static_cast<WORD>(mainServer->getWorldTime());
	const QuestCondition007* cond = reinterpret_cast<const QuestCondition007*>(conditionHeader);
	if (worldTime >= cond->startTime && worldTime <= cond->endTime)
		return true;
	return false;
}

bool QuestService::checkRemainingTime(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (!trans->selectedQuest)
		return false;
	const QuestCondition008* cond = reinterpret_cast<const QuestCondition008*>(conditionHeader);
	const dword_t passedTime = trans->selectedQuest->getPassedTime();
	return OperationService::checkOperation(passedTime, cond->totalTime, cond->operation);
}

bool QuestService::checkSkill(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (!trans->questTriggerCauser || trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition009* cond = reinterpret_cast<const QuestCondition009*>(conditionHeader);
	bool success = cond->operation == 0;
	for (unsigned int i = 0; i < PlayerSkill::PLAYER_MAX_SKILLS; i++) {
		Skill* curSkill = player->getSkill(i);
		if (!curSkill)
			continue;
#ifdef __ROSE_DEBUG__
		word_t currentSkillId = curSkill->getId();
		if (currentSkillId >= cond->skillIdFirst && currentSkillId <= cond->skillIdSecond) {
#else
		if (curSkill->getId() >= cond->skillIdFirst && curSkill->getId() <= cond->skillIdSecond) {
#endif
			if (cond->operation == 0)
				return false;
			else
				return true;
		}
	}
	return success;
}

bool QuestService::checkRandomPercentage(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	const QuestCondition010* cond = reinterpret_cast<const QuestCondition010*>(conditionHeader);
	const dword_t randomChance = rand() % 101;
	if (randomChance >= cond->percentageLow && randomChance <= cond->percentageHigh)
		return true;
	return false;
}

bool QuestService::checkObjectVar(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	const QuestCondition011* cond = reinterpret_cast<const QuestCondition011*>(conditionHeader);

	dword_t objValue = 0x00;
		
	NPC* npc = dynamic_cast<NPC*>(trans->questTarget);
	if (!npc)
		return false;
	objValue = npc->getObjVar(cond->varNum);

	return OperationService::checkOperation(objValue, cond->amount, cond->operation);
}

bool QuestService::checkEventObject(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	const QuestCondition012* cond = reinterpret_cast<const QuestCondition012*>(conditionHeader);
	//TODO: IMPLEMENT EVENT OBJECTS
	return false;
}

bool QuestService::checkNPCVar(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	const QuestCondition013* cond = reinterpret_cast<const QuestCondition013*>(conditionHeader);
	trans->questTarget = mainServer->getNPCGlobal(cond->npcId);
	return (trans->questTarget != nullptr);
}

bool QuestService::checkSwitch(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (!trans->questTriggerCauser || trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition014* cond = reinterpret_cast<const QuestCondition014*>(conditionHeader);
	return player->getQuestFlag(cond->switchNum) == cond->isSwitchOn;
}

bool QuestService::checkPartyMemberCount(QuestTrans* trans, const AbstractQuestInfo* conditionHeader){
	if (!trans->questTriggerCauser || trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition015* cond = reinterpret_cast<const QuestCondition015*>(conditionHeader);
	//TODO: IMPLEMENT PARTY
	//if(player->getParty()->size() >= cond->firstNum && player->getParty()->size() <= cond->secondNum)
		//return true;
	return false;
}

bool QuestService::checkMapTime(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	const QuestCondition016* cond = reinterpret_cast<const QuestCondition016*>(conditionHeader);
	Entity* realTarget = cond->who == 2 ? trans->questTriggerCauser : trans->questTarget;
	if (!realTarget)
		return false;
	const dword_t zoneTime = mainServer->getMapTime(realTarget->getMapId());
	return (zoneTime >= cond->timeStart && zoneTime <= cond->timeEnd);
}

bool QuestService::checkNPCVarDifferences(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	const QuestCondition017* cond = reinterpret_cast<const QuestCondition017*>(conditionHeader);
	try {
		NPC* first = mainServer->getNPCGlobal(cond->firstVar.npcId);
		if (!first)
			throw TraceableException("Couldn't find first NPC %i", cond->firstVar.npcId);
		NPC* second = mainServer->getNPCGlobal(cond->secondVar.npcId);
		if (!second)
			throw TraceableException("Couldn't find second NPC %i", cond->secondVar.npcId);
		int npcVarOne = first->getObjVar(cond->firstVar.varType);
		int npcVarTwo = second->getObjVar(cond->secondVar.varType);
		return OperationService::checkOperation(npcVarOne, npcVarTwo, cond->operation);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return false;
}

bool QuestService::checkServerTimeMonth(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	const QuestCondition018* cond = reinterpret_cast<const QuestCondition018*>(conditionHeader);

	SYSTEMTIME localTime; GetSystemTime(&localTime);
	if (cond->day && cond->day != localTime.wDay)
		return false;
	word_t wCurTime = localTime.wHour * 60 + localTime.wMinute;
	if (wCurTime >= (cond->hourStart * 60 + cond->minuteStart) && wCurTime <= (cond->hourEnd * 60 + cond->minuteEnd))
		return true;
	return false;
}

bool QuestService::checkServerTimeWeekday(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	const QuestCondition019* cond = reinterpret_cast<const QuestCondition019*>(conditionHeader);

	SYSTEMTIME localTime; GetSystemTime(&localTime);
	if (cond->weekDay != localTime.wDayOfWeek)
		return false;
	word_t wCurTime = localTime.wHour * 60 + localTime.wMinute;
	if (wCurTime >= (cond->hourStart * 60 + cond->minuteStart) && wCurTime <= (cond->hourEnd * 60 + cond->minuteEnd))
		return true;
	return false;
}

bool QuestService::checkDistanceFromCenter(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	const QuestCondition021* cond = reinterpret_cast<const QuestCondition021*>(conditionHeader);
	
	if (trans->questTriggerCauser->getMapId() != trans->questTarget->getMapId())
		return false;
	//TODO:
	if (trans->questTriggerCauser->getPositionCurrent().distanceTo(trans->questTarget->getPositionCurrent()) <= cond->radius)
		return true;
	return false;
}

bool QuestService::checkIsClanMember(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition023* cond = reinterpret_cast<const QuestCondition023*>(conditionHeader);
	//TODO: IS CLAN MEMBER?
	bool result = (false ^ cond->isRegistered) > 0;
	return result;
}

bool QuestService::checkClanInternalPosition(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition024* cond = reinterpret_cast<const QuestCondition024*>(conditionHeader);
	//TODO: CLAN -- GET INTERNAL_position_t IN CLAN --> CHECK
	return false;
}

bool QuestService::checkClanContribution(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition025* cond = reinterpret_cast<const QuestCondition025*>(conditionHeader);
	//TODO: CLAN -- GET CLAN_POINTS IN CLAN --> CHECK
	return false;
}

bool QuestService::checkClanLevel(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition026* cond = reinterpret_cast<const QuestCondition026*>(conditionHeader);
	//TODO: CLAN -- GET CLAN_LEVEL IN CLAN --> CHECK
	return false;
}

bool QuestService::checkClanScore(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition027* cond = reinterpret_cast<const QuestCondition027*>(conditionHeader);
	//TODO: CLAN -- GET CLAN_SCORE IN CLAN --> CHECK
	return false;
}
bool QuestService::checkClanMoney(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition028* cond = reinterpret_cast<const QuestCondition028*>(conditionHeader);
	//TODO: CLAN -- GET CLAN_MONEY IN CLAN --> CHECK
	return false;
}
bool QuestService::checkClanMemberCount(QuestTrans* trans, const AbstractQuestInfo* conditionHeader) {
	if (trans->questTriggerCauser->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestCondition029* cond = reinterpret_cast<const QuestCondition029*>(conditionHeader);
	//TODO: CLAN -- GET CLAN_MEMBER_AMOUNT IN CLAN --> CHECK
	return false;
}

bool QuestService::checkClanSkill(QuestTrans* trans, const AbstractQuestInfo* header) {
	//TODO: IMPLEMENT CLAN SKILL
	return false;
}

bool QuestService::checkUNKNOWN(QuestTrans* trans, const AbstractQuestInfo* basicInfo) {
	try {
		QuestEntry* quest = mainServer->getQuest(trans->questHash);
		if (quest)
			throw TraceableException("[Quest %s (Hash: 0x%x) by %s] RewardID: 0x%x with length %i\n", quest->getQuestName().c_str(), trans->questHash, trans->questTriggerCauser->getName().c_str(), basicInfo->header.operationType, basicInfo->header.length);
		throw TraceableException("[QuestHash: 0x%x by %s] RewardID: 0x%x with length %i\n", trans->questHash, trans->questTriggerCauser->getName().c_str(), basicInfo->header.operationType, basicInfo->header.length);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return false;
}

const QuestService::REWARD_FUNCTION_PTR QuestService::rewards[50] = {
	rewardNewQuest,
	rewardQuestItem,
	rewardQuestVar,
	rewardAbility,
	rewardQuestVar,
	rewardExpMoneyOrItem,
	rewardRegeneration,
	rewardWarp,
	rewardSpawnMonster,
	rewardNextQuestHash,
	rewardResetStats,
	rewardObjectVar,
	rewardNPCMessage,
	rewardQuestTriggerWhenTimeExpired,
	rewardNewSkill,
	rewardQuestSwitch,
	rewardQuestClearSwitch,
	rewardQuestClearAllSwitches,
	rewardServerAnnouncement,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardClanLevelIncrease,
	rewardClanMoneyChange,
	rewardClanScoreChange,
	rewardClanSkillChange,
	rewardClanScoreChange,
	rewardWarpNearbyClanMember,
	rewardScriptTrigger,
	rewardSkillReset,
	rewardSingleQuestVar,
	rewardItem,
	rewardUnknown33,
	rewardNPCVisuality,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN,
	rewardUNKNOWN
};

bool QuestService::applyActions(QuestTrans *trans, const AbstractQuestInfo *actionHeader, bool isTryoutRun) {
	if (!trans->questTriggerCauser || !actionHeader)
		return false;

	return QuestService::rewards[actionHeader->header.operationType & 0xFF](trans, actionHeader, isTryoutRun);

	//Pretty oldschool
	/*
	switch (actionHeader->header.operationType) {
		case 0x01000000:
			return QuestService::rewardNewQuest(trans, actionHeader, isTryoutRun);
		break;
		case 0x01000001:
			return QuestService::rewardQuestItem(trans, actionHeader, isTryoutRun);
		case 0x01000002:
		case 0x01000004:
			return QuestService::rewardQuestVar(trans, actionHeader, isTryoutRun);
		case 0x01000003:
			return QuestService::rewardAbility(trans, actionHeader, isTryoutRun);
		case 0x01000005:
			return QuestService::rewardExpMoneyOrItem(trans, actionHeader, isTryoutRun);
		case 0x01000006:
			return QuestService::rewardRegeneration(trans, actionHeader, isTryoutRun);
		case 0x01000007:
			return QuestService::rewardWarp(trans, actionHeader, isTryoutRun);
		case 0x01000008:
			return QuestService::rewardSpawnMonster(trans, actionHeader, isTryoutRun);
		case 0x01000009:
			return QuestService::rewardNextQuestHash(trans, actionHeader, isTryoutRun);
		case 0x0100000A:
			return QuestService::rewardResetStats(trans, actionHeader, isTryoutRun);
		case 0x0100000B:
			return QuestService::rewardObjectVar(trans, actionHeader, isTryoutRun);
		case 0x0100000C:
			return QuestService::rewardNPCMessage(trans, actionHeader, isTryoutRun);
		case 0x0100000D:
			return QuestService::rewardQuestTriggerWhenTimeExpired(trans, actionHeader, isTryoutRun);
		case 0x0100000E:
			return QuestService::rewardNewSkill(trans, actionHeader, isTryoutRun);
		case 0x0100000F:
			return QuestService::rewardQuestSwitch(trans, actionHeader, isTryoutRun);
		case 0x01000010:
			return QuestService::rewardQuestClearSwitch(trans, actionHeader, isTryoutRun);
		case 0x01000011:
			return QuestService::rewardQuestClearAllSwitches(trans, actionHeader, isTryoutRun);
		case 0x01000012:
			return QuestService::rewardServerAnnouncement(trans, actionHeader, isTryoutRun);
		case 0x01000017:
			return QuestService::rewardClanLevelIncrease(trans, actionHeader, isTryoutRun);
		case 0x01000018:
			return QuestService::rewardClanMoneyChange(trans, actionHeader, isTryoutRun);
		case 0x01000019:
		case 0x0100001B:
			return QuestService::rewardClanScoreChange(trans, actionHeader, isTryoutRun);
		case 0x0100001A:
			return QuestService::rewardClanSkillChange(trans, actionHeader, isTryoutRun);
		case 0x0100001C:
			return QuestService::rewardWarpNearbyClanMember(trans, actionHeader, isTryoutRun);
		case 0x0100001D:
			return QuestService::rewardScriptTrigger(trans, actionHeader, isTryoutRun);
		case 0x0100001E:
			return QuestService::rewardSkillReset(trans, actionHeader, isTryoutRun);
		case 0x0100001F:
			return QuestService::rewardSingleQuestVar(trans, actionHeader, isTryoutRun);
		case 0x01000020:
			return QuestService::rewardItem(trans, actionHeader, isTryoutRun);
		case 0x01000021:
			return QuestService::rewardUnknown33(trans, actionHeader, isTryoutRun);
		case 0x01000022:
			return QuestService::rewardNPCVisuality(trans, actionHeader, isTryoutRun);
	}
	*/
	return false;
}

bool QuestService::rewardNewQuest(QuestTrans *trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	const QuestReward000* reward = reinterpret_cast<const QuestReward000*>(header);
	switch (reward->operation) {
	case 0x00: //remove quest
			if (!trans->selectedQuest)
				return true;
			if (isTryoutRun)
				return true;
			trans->selectedQuest->reset();
		break;
		case 0x01: //Start quest
			if (trans->questTriggerCauser && trans->questTriggerCauser->isPlayer()) {
				Player* questPlayer = dynamic_cast<Player*>(trans->questTriggerCauser);
				if (isTryoutRun) {
					trans->wasPreviouslyEstablishingQuest = true;
					return (questPlayer->getEmptyQuestSlot() != nullptr);
				}
				return questPlayer->addQuest(reward->questId);
			}
		break;
		case 0x02:
			if (isTryoutRun) {
				trans->wasPreviouslyEstablishingQuest = true;
				return true;
			}
			trans->selectedQuest->setQuest(reward->questId);
		break;
		case 0x03:
			if (isTryoutRun) {
				trans->wasPreviouslyEstablishingQuest = true;
				return true;
			}
			trans->selectedQuest->reset();
			trans->selectedQuest->setQuest(reward->questId);
		break;
		case 0x04:
			if (trans->questTriggerCauser && trans->questTriggerCauser->isPlayer()) {
				Player* questPlayer = dynamic_cast<Player*>(trans->questTriggerCauser);
				if (isTryoutRun) {
					//Work-Around when quests are established beforehand; in tryout-runs, they aren't really established -> they cannot be selected.
					//This flag enables the check to skip the selection-part
					if (trans->wasPreviouslyEstablishingQuest) {
						return true;
					}
					dword_t currentlySelectedId = (questPlayer->getSelectedQuest() == nullptr ? 0x00 : questPlayer->getSelectedQuest()->getQuestId());
					bool result = questPlayer->searchAndSelectQuest(reward->questId);
					questPlayer->searchAndSelectQuest(currentlySelectedId);
					return result;
				}
				if (!questPlayer->searchAndSelectQuest(reward->questId))
					return false;
				trans->selectedQuest = questPlayer->getSelectedQuest();
			}
			else {
				return false;
			}
		break;
	}
	return true;
}

bool QuestService::rewardQuestItem(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser || !trans->questTriggerCauser->isPlayer())
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestReward001* reward = reinterpret_cast<const QuestReward001*>(header);
	Item rewardItem(reward->itemId); rewardItem.amount = reward->amount;
	switch (reward->operation) {
		case 0x00: //Remove items
			try {
				if (rewardItem.type == ItemType::QUEST) {
					if (isTryoutRun) {
						bool success = trans->selectedQuest->removeItemIfSufficient(rewardItem);
						trans->selectedQuest->addItem(rewardItem);
						return success;
					}
					return trans->selectedQuest->removeItemIfSufficient(rewardItem);
				}
				throw TraceableException("Player is supposed to wear/get the item %i", reward->itemId);
			}
			catch (std::exception& ex) { std::cout << ex.what() << "\n"; }
		break;
		case 0x01: //QuestOwner QuestItem
			try {
				if (reward->partyOption == 1)
					throw TraceableException("Player is supposed to split item %i within party ", reward->itemId);
			}
			catch (std::exception& ex) { std::cout << ex.what() << "\n"; }
			if (isTryoutRun)
				return true;
			trans->selectedQuest->addItem(rewardItem);
			player->updateQuestData();
		break;
	}
	return true;
}

bool QuestService::rewardQuestVar(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser || !trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return true;
	const QuestReward002* reward = reinterpret_cast<const QuestReward002*>(header);
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	for (unsigned int i = 0; i < reward->varAmount; i++) {
		const QuestSetVar* questVar = reinterpret_cast<const QuestSetVar*>(&reward->vars[i]);
		word_t currentQuestVarValue = player->getQuestVariable(questVar->varType, questVar->varNum);
		word_t newQuestVarValue = OperationService::resultOperation(currentQuestVarValue, questVar->amount, questVar->operation);
		player->setQuestVariable(questVar->varType, questVar->varNum, newQuestVarValue);
	}
	return true;
}

bool QuestService::rewardAbility(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser || !trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return true;
	const QuestReward003* reward = reinterpret_cast<const QuestReward003*>(header);
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);

	bool success = true;
	for (unsigned int i = 0; i < reward->varAmount; i++) {
		const QuestSetVar* abilityVar = reinterpret_cast<const QuestSetVar*>(&reward->vars[i]);
		success &= player->changeAbility(abilityVar->varNum, static_cast<DWORD>(abilityVar->amount), (abilityVar->operation == 0 ? reward->partyOption : abilityVar->operation));
	}
	return success;
}

bool QuestService::rewardExpMoneyOrItem(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser || !trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return true;
	bool wasSuccessful = false;
	const QuestReward005* reward = reinterpret_cast<const QuestReward005*>(header);
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	if (reward->rewardType == 0x00) {
		const dword_t curExp = player->getExperience();
		dword_t newExp = QuestService::rewardOperation(player, reward->amount, reward->equate);
		player->addExperience(newExp);
		wasSuccessful = true;
	}
	else if (reward->rewardType == 0x01) {
		const qword_t curZulies = player->getZulies();
		const qword_t newZulies = QuestService::rewardOperation(player, reward->amount, reward->equate) * (trans->selectedQuest->getQuestVar(PlayerQuest::QUEST_VAR_MAX - 1)+1);
		wasSuccessful = player->updateZulies(curZulies + newZulies);
		if (wasSuccessful)
			trans->selectedQuest->setQuestVar(PlayerQuest::QUEST_VAR_MAX - 1, 0);
	}
	else {
		//TODO:
		Item item(reward->itemId);
		wasSuccessful = player->addItemToInventory(item);
		if (reward->partyOption || reward->itemOption) {
			try {
				throw TraceableException("Item %i should be rewarded [Options: Item %i, Party %i]", reward->itemId, reward->itemOption, reward->partyOption);
			}
			catch (std::exception& ex) {
				std::cout << ex.what() << "\n";
			}
		}
	}
	return wasSuccessful;
}

bool QuestService::rewardRegeneration(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (isTryoutRun)
		return true;
	const QuestReward006* reward = reinterpret_cast<const QuestReward006*>(header);
	dword_t newHp = trans->questTriggerCauser->getMaxHP() * reward->percentHP / 100;
	dword_t newMp = trans->questTriggerCauser->getMaxMP() * reward->percentMP / 100;

	trans->questTriggerCauser->setCurrentHP(newHp);
	trans->questTriggerCauser->setCurrentMP(newMp);

	return true;
}

bool QuestService::rewardWarp(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (isTryoutRun)
		return true;
	const QuestReward007* reward = reinterpret_cast<const QuestReward007*>(header);
	if (reward->partyOption) {
		try {
			throw TraceableException("[QUEST %i] Reward should warp entire party to %i [%i, %i]!", QuestService::currentQuestId, reward->mapId, reward->x, reward->y);
		}
		catch (std::exception& ex) {
			std::cout << ex.what() << "\n";
		}
	}
	else  {
		if (trans->questTriggerCauser->isPlayer()) {
			Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
			return player->pakTelegate(reward->mapId, position_t(reward->x, reward->y));
		}
	}
	return true;
}

bool QuestService::rewardSpawnMonster(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (isTryoutRun)
		return true;
	const QuestReward008* reward = reinterpret_cast<const QuestReward008*>(header);
	word_t mapId; position_t newPos;
	switch (reward->targetType) {
		case 0x00:
			mapId = trans->questTriggerCauser->getMapId();
			newPos = trans->questTriggerCauser->getPositionCurrent();
		break;
		case 0x01:
		case 0x02:
			mapId = trans->questTarget->getMapId();
			newPos = trans->questTarget->getPositionCurrent();
		break;
		case 0x03:
			mapId = reward->mapId;
			newPos = position_t(reward->x, reward->y);
		break;
	}
	for (unsigned int i = 0; i < reward->amount; i++) {
		new Monster(mainServer->getNPCData(reward->monsterId), mainServer->getAIData(reward->monsterId), mapId, newPos.calcNewPositionWithinRadius(static_cast<float>(reward->radius)));
	}
	return true;
}

bool QuestService::rewardNextQuestHash(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (isTryoutRun)
		return true;
	const QuestReward009* reward = reinterpret_cast<const QuestReward009*>(header);
	dword_t questHash = ::makeQuestHash(reward->triggerName);
	try {
		throw TraceableException("Trigger %s [0x%x] wants to be triggered!", reward->triggerName, questHash);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	}

	if (QuestService::runQuest(trans->questTriggerCauser, questHash) == 0x00)
		return false;
	return true; //... :|
}

bool QuestService::rewardResetStats(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (trans->questTriggerCauser->isPlayer()) {
		if (isTryoutRun)
			return true;
		Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
		player->resetAttributes();
		return true;
	}
	return false;
}

bool QuestService::rewardObjectVar(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	const QuestReward011* reward = reinterpret_cast<const QuestReward011*>(header);
	NPC *npc = reward->isTargetSelected ? dynamic_cast<NPC*>(trans->questTarget) : dynamic_cast<NPC*>(trans->questTriggerCauser);
	if (!npc)
		return false;
	if (isTryoutRun)
		return true;
	word_t currentValue = npc->getObjVar(reward->varType);
	word_t newValue = OperationService::resultOperation(currentValue, reward->value, reward->operation);
	npc->setObjVar(reward->varType, newValue);
	return true;
}

bool QuestService::rewardNPCMessage(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	const QuestReward012* reward = reinterpret_cast<const QuestReward012*>(header);
	if (isTryoutRun) {
		if (reward->messageType <= 0x02) {
			return true;
		}
		return false;
	}
	switch (reward->messageType) {
		case 0x00:
			return ChatService::sendMessage(trans->questTarget, reward->message);
		break;
		case 0x01:
			return ChatService::sendShout(trans->questTarget, "%s:  %s", trans->questTarget->getName().c_str(), reward->message);
		break;
		case 0x02:
			return ChatService::sendAnnouncement(trans->questTarget, "%s:  %s", trans->questTarget->getName().c_str(), reward->message);
		break;
	}
	return false;
}

bool QuestService::rewardQuestTriggerWhenTimeExpired(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	const QuestReward013* reward = reinterpret_cast<const QuestReward013*>(header);
	if (isTryoutRun)
		return false; //TODO
	try {
		throw TraceableException("[QUEST %i] Trigger %s wants to be triggered after %i seconds!", QuestService::currentQuestId, reward->triggerName, reward->seconds);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return false;
}

bool QuestService::rewardNewSkill(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	const QuestReward014* reward = reinterpret_cast<const QuestReward014*>(header);
	if (!trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return true;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	return player->addSkill(mainServer->getSkill(reward->skillId));
}

bool QuestService::rewardQuestSwitch(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return true;
	const QuestReward015* reward = reinterpret_cast<const QuestReward015*>(header);
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	player->setQuestFlag(reward->switchId, reward->operation);
	return true;
}

bool QuestService::rewardQuestClearSwitch(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return true;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestReward016* reward = reinterpret_cast<const QuestReward016*>(header);
	try {
		throw TraceableException("[QUEST %i] SwitchGroup %i wants to be cleared", QuestService::currentQuestId, reward->groupId);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	} 
	player->clearQuestFlag(reward->groupId);
	return true;
}

bool QuestService::rewardQuestClearAllSwitches(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return true;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestReward017* reward = reinterpret_cast<const QuestReward017*>(header);
	try {
		throw TraceableException("[QUEST %i] All questSwitches want to be cleared", QuestService::currentQuestId);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	for (unsigned int i = 0; i < 0x40;i++)
		player->clearQuestFlag(i);
	return true;
}

bool QuestService::rewardServerAnnouncement(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	//????
	if (isTryoutRun)
		return false; //TODO
	const QuestReward018* reward = reinterpret_cast<const QuestReward018*>(header);
	try {
		throw TraceableException("[QUEST %i] ServerAnnouncement not possible! DataCount: %i", QuestService::currentQuestId, reward->dataCount);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return false;
}

bool QuestService::rewardClanLevelIncrease(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);

	//TODO:
	//player->getClan()->increaseLevel();
	return false;
}

bool QuestService::rewardClanMoneyChange(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	//TODO: CLAN_MONEY
	//if(player->isClanMaster()) 

	return false;
}

bool QuestService::rewardClanScoreChange(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	//TODO: CLAN_SCORE
	//if(player->isClanMaster()) 

	return false;
}

bool QuestService::rewardClanSkillChange(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	//TODO: CLAN_SKILL
	//if(player->isClanMaster()) 

	return false;
}

bool QuestService::rewardWarpNearbyClanMember(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return false;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	//TODO: ITERATE CLAN MEMBER AND WARP THEM
	//if(player->getClan()) 

	return false;
} 

bool QuestService::rewardScriptTrigger(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	/*if (isTryoutRun)
		return true;
	const QuestReward029* reward = reinterpret_cast<const QuestReward029*>(header);
	std::vector<char> script(reward->scriptLength+1);
	memcpy(script.data(), reward->script, reward->scriptLength);
	const dword_t newQuestHash = ::makeQuestHash(script.data());
	return QuestService::runQuest(trans->questTriggerCauser, newQuestHash, isTryoutRun);
	*/
	//Seems to be something internal in the ROSE-client (trigger for other events?)
	return true; 
}

bool QuestService::rewardSkillReset(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return true;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	player->resetSkills();
	return true;
}

bool QuestService::rewardSingleQuestVar(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return true;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestReward031* reward = reinterpret_cast<const QuestReward031*>(header);
	word_t varValue = player->getQuestVariable(reward->var.varType, reward->var.varNum);
	word_t newValue = OperationService::resultOperation(varValue, reward->var.amount, reward->var.operation);
	player->setQuestVariable(reward->var.varType, reward->var.varNum, newValue);
	return true;
}

bool QuestService::rewardItem(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTriggerCauser->isPlayer())
		return false;
	if (isTryoutRun)
		return true;
	Player* player = dynamic_cast<Player*>(trans->questTriggerCauser);
	const QuestReward032* reward = reinterpret_cast<const QuestReward032*>(header);
	Item itemToReward(reward->itemId);
	player->addItemToInventory(itemToReward);
	return true;
}

bool QuestService::rewardNPCVisuality(QuestTrans* trans, const AbstractQuestInfo* header, bool isTryoutRun) {
	if (!trans->questTarget && !trans->questTarget->isNPC())
		return false;
	if (isTryoutRun)
		return false;
	NPC *npc = dynamic_cast<NPC*>(trans->questTarget);
	const QuestReward033* reward = reinterpret_cast<const QuestReward033*>(header);
	try {
		throw TraceableException("[QUEST: %i] NPC %s needs a visuality change to %i!\n", QuestService::currentQuestId, npc->getName().c_str(), reward->unknown);
	}
	catch (std::exception& ex) { std::cout << ex.what() << "\n"; }
	return false;
}

bool QuestService::rewardUNKNOWN(QuestTrans* trans, const AbstractQuestInfo* basicInfo, bool isTryoutRun) {
	try {
		QuestEntry* quest = mainServer->getQuest(trans->questHash);
		if (quest)
			throw TraceableException("[Quest %s (Hash: 0x%x) by %s] RewardID: 0x%x with length %i\n", quest->getQuestName().c_str(), trans->questHash, trans->questTriggerCauser->getName().c_str(), basicInfo->header.operationType, basicInfo->header.length);
		throw TraceableException("[QuestHash: 0x%x by %s] RewardID: 0x%x with length %i\n", trans->questHash, trans->questTriggerCauser->getName().c_str(), basicInfo->header.operationType, basicInfo->header.length);
	}
	catch (std::exception& ex) {
		std::cout << ex.what() << "\n";
	}
	return false;
}
#endif //#__ROSE_QSD_DEBUG__

std::string QuestService::conditionToString(const char* data) {
	const AbstractQuestInfo *header = reinterpret_cast<const AbstractQuestInfo*>(data);
	char buffer[0x500] = { 0x00 };
	std::string result = "";
	sprintf(buffer, "OperationType: 0x%x\n", header->header.operationType);
	switch (header->header.operationType) {
		case 0x00:
		{
			const QuestCondition000* cond = reinterpret_cast<const QuestCondition000*>(data);
			sprintf(&buffer[strlen(buffer)], "Check QuestJourney for the given questId %i\n", cond->questId);
		}
		break;
		case 0x01:
		case 0x02:
		{
					 const QuestCondition001* cond = reinterpret_cast<const QuestCondition001*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check for QuestVariable (Amount: %i)\n", cond->dataCount);
					 for (unsigned int i = 0; i < cond->dataCount; i++) {
						 const QuestCheckVar* data = reinterpret_cast<const QuestCheckVar*>(&cond->data[i]);
						 sprintf(&buffer[strlen(buffer)], "Var[0x%x, %i] %s %i\n", data->varType, data->varNum, OperationService::operationName(data->operation), data->amount);
					 }
		}
		break;
		case 0x03:
		{
					 const QuestCondition003* cond = reinterpret_cast<const QuestCondition003*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check for Ability of Player (Amount: %i)\n", cond->dataCount);
					 for (unsigned int i = 0; i < cond->dataCount; i++) {
						 const QuestCheckAbility* data = reinterpret_cast<const QuestCheckAbility*>(&cond->data[i]);
						 sprintf(&buffer[strlen(buffer)], "VarType: %i %s Amount: %i\n", data->abilityType, OperationService::operationName(data->operation), data->amount);
					 }
		}
		break;
		case 0x04:
		{
					 const QuestCondition004* cond = reinterpret_cast<const QuestCondition004*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check for QuestItems of Player (Amount: %i)\n", cond->dataCount);
					 for (unsigned int i = 0; i < cond->dataCount; i++) {
						 const QuestCheckItem* data = reinterpret_cast<const QuestCheckItem*>(&cond->data[i]);
						 sprintf(&buffer[strlen(buffer)], "Item: %i at slot %i %s Amount: %i\n", data->itemId, data->itemSlot, OperationService::operationName(data->operation), data->amount);
					 }

		}
		break;
		case 0x05:
		{
					 const QuestCondition005* cond = reinterpret_cast<const QuestCondition005*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check for Party (Level: %i, isPlayerLeader: %i, Reverse(?): %i)\n", cond->level, cond->isLeader, cond->reverse);
		}
		break;
		case 0x06:
		{
					 const QuestCondition006* cond = reinterpret_cast<const QuestCondition006*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check allowed distance (%i) to point (%i, %i) on map %i\n", cond->radius, cond->x, cond->y, cond->mapId);
		}
		break;
		case 0x07:
		{
					 const QuestCondition007* cond = reinterpret_cast<const QuestCondition007*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if worldTime is between %i and %i\n", cond->startTime, cond->endTime);
		}
		break;
		case 0x08:
		{
					 const QuestCondition008* cond = reinterpret_cast<const QuestCondition008*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if CurrentTime - QuestStartTime %s %i\n", OperationService::operationName(cond->operation), cond->totalTime);
		}
		break;
		case 0x09:
		{
					 const QuestCondition009* cond = reinterpret_cast<const QuestCondition009*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check for skill (between %i and %i); Reverse result %i\n", cond->skillIdFirst, cond->skillIdSecond, cond->operation);
		}
		break;
		case 0x0A:
		{
					 const QuestCondition010* cond = reinterpret_cast<const QuestCondition010*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check random percentage between %i and %i percent\n", cond->percentageLow, cond->percentageHigh);
		}
		break;
		case 0x0B:
		{
					 const QuestCondition011* cond = reinterpret_cast<const QuestCondition011*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check ObjectVar of %s - Type %i %s Amount %i\n", cond->who == 0x00 ? "NPC" : "Player?", cond->varNum, OperationService::operationName(cond->operation), cond->amount);
		}
		break;
		case 0x0C:
		{
					 const QuestCondition012* cond = reinterpret_cast<const QuestCondition012*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check Event %i at position_t (%i, %i) on map %i\n", cond->eventId, cond->x, cond->y, cond->mapId);
		}
		break;
		case 0x0D:
		{
					 const QuestCondition013* cond = reinterpret_cast<const QuestCondition013*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if NPC %i exists (and select it)\n", cond->npcId);
		}
		break;
		case 0x0E:
		{
					 const QuestCondition014* cond = reinterpret_cast<const QuestCondition014*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if QuestSwitch [%i] == %i\n", cond->switchNum>>3, cond->isSwitchOn);
		}
		break;
		case 0x0F:
		{
					 const QuestCondition015* cond = reinterpret_cast<const QuestCondition015*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if party member amount >= %i && <= %i\n", cond->firstNum, cond->secondNum);
		}
		break;
		case 0x10:
		{
					 const QuestCondition016* cond = reinterpret_cast<const QuestCondition016*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if mapTime >= %i && <= %i on %s-map\n", cond->timeStart, cond->timeEnd, cond->who == 0x00 ? "NPC" : cond->who == 0x01 ? "EventObject" : "Player");
		}
			break;
		case 0x11:
		{
					 const QuestCondition017* cond = reinterpret_cast<const QuestCondition017*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check VarDifference between NPCs [%i;%i] and VarTypes [%i;%i] via Operator %s\n", cond->firstVar.npcId, cond->secondVar.npcId, cond->firstVar.varType, cond->secondVar.varType, OperationService::operationName(cond->operation) );
		}
			break;
		case 0x12:
		{
					 const QuestCondition018* cond = reinterpret_cast<const QuestCondition018*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if serverTime Day == %i, and Time between %i:%i and %i:%i\n", cond->day, cond->hourStart, cond->minuteStart, cond->hourEnd, cond->minuteEnd);
		}
		break;
		case 0x13:
		{
					 const QuestCondition019* cond = reinterpret_cast<const QuestCondition019*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if serverTime weekDay == %i, and Time between %i:%i and %i:%i\n", cond->weekDay, cond->hourStart, cond->minuteStart, cond->hourEnd, cond->minuteEnd);
		}
		break;
		case 0x14:
		{
					 const QuestCondition020* cond = reinterpret_cast<const QuestCondition020*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if teamId >= %i && <= %i\n", cond->firstNum, cond->secondNum);
		}
			break;
		case 0x15:
		{
					 const QuestCondition021* cond = reinterpret_cast<const QuestCondition021*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if distance to Object %i <= %i\n", cond->selectedObjType, cond->radius);
		}
		break;
		case 0x16:
		{
					 const QuestCondition022* cond = reinterpret_cast<const QuestCondition022*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check for point [%i, %i]\n", cond->x, cond->y);
		}
			break;
		case 0x17:
		{
					 const QuestCondition023* cond = reinterpret_cast<const QuestCondition023*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if player %s\n", cond->isRegistered > 0 ? "is a ClanMember" : "is not a ClanMember");
		}
		break;
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		{
					 const QuestCondition024* cond = reinterpret_cast<const QuestCondition024*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if clan stat %s %i\n", OperationService::operationName(cond->operation), cond->pointType);
		}
		break;
		case 0x1C:
		{
					 const QuestCondition028* cond = reinterpret_cast<const QuestCondition028*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check clan money %s %i\n", OperationService::operationName(cond->operation), cond->moneyAmount);
		}
		break;
		case 0x1D:
		{
					 const QuestCondition029* cond = reinterpret_cast<const QuestCondition029*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if party member amount %s %i\n", OperationService::operationName(cond->operation), cond->memberAmount);
		}
		break;
		case 0x1E:
		{
					 const QuestCondition030* cond = reinterpret_cast<const QuestCondition030*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if clan skill %s %i (%i)\n", OperationService::operationName(cond->operation), cond->skillIdFirst, cond->skillIdSecond);
		}
		break;
		default:
			sprintf(&buffer[strlen(buffer)], "Unknown condition format\n");
	}
	result = std::string(buffer);
	result += std::string("\n");
	return result;
}

std::string QuestService::actionToString(const char* data) {
	const AbstractQuestInfo *header = reinterpret_cast<const AbstractQuestInfo*>(data);
	char buffer[0x300] = { 0x00 };
	std::string result = "";
	sprintf(buffer, "OperationType: 0x%x\n", header->header.operationType);
	switch ((header->header.operationType & 0xFF)) {
		case 0x00:
		{
			const QuestReward000* reward = reinterpret_cast<const QuestReward000*>(data);
			sprintf(&buffer[strlen(buffer)], "%s quest with ID %i\n", (reward->operation == 0x00 ? "Delete" : reward->operation == 0x01 ? "Create" : reward->operation == 0x02 ? "Keep items and replaced selected" : reward->operation == 0x03 ? "Reset items and replace selected" : "Select quest"), reward->questId);
		}
		break;
		case 0x01:
		{
			const QuestReward001* reward = reinterpret_cast<const QuestReward001*>(data);
			sprintf(&buffer[strlen(buffer)], "%s questItem %i %ix time(s) - [PartyOption %i]\n", reward->operation == 0x00 ? "Remove" : "Give", reward->itemId, reward->amount, reward->partyOption);
		}
		break;
		case 0x02:
		case 0x04:
		{
			const QuestReward002* reward = reinterpret_cast<const QuestReward002*>(data);
			sprintf(&buffer[strlen(buffer)], "Change QuestVariable[s] (Amount: %i)\n", reward->varAmount);
			for (unsigned int j = 0; j < reward->varAmount; j++)
				sprintf(&buffer[strlen(buffer)], "Var[0x%x,%i] %s %i\n", reward->vars[j].varType, reward->vars[j].varNum, OperationService::operationName(reward->vars[j].operation), reward->vars[j].amount);
		}
		break;
		case 0x03:
		{
			const QuestReward003* reward = reinterpret_cast<const QuestReward003*>(data);
			sprintf(&buffer[strlen(buffer)], "Change Ability of Player (Amount: %i)\n", reward->varAmount);
			for (unsigned int j = 0; j < reward->varAmount; j++)
				sprintf(&buffer[strlen(buffer)], "AbilityID[%i] %s= %i\n", reward->vars[j].varNum, OperationService::operationName(reward->vars[j].operation), reward->vars[j].amount);
		}
		break;
		case 0x05:
		{
			const QuestReward005* reward = reinterpret_cast<const QuestReward005*>(data);
			sprintf(&buffer[strlen(buffer)], "Reward Player with %s: ", reward->rewardType == 0x00 ? "Experience" : reward->rewardType == 0x01 ? "Money" : "Item");
			if (reward->rewardType == 0x02) {
				sprintf(&buffer[strlen(buffer)], "%i [%ix]\n", reward->itemId, reward->amount);
			}
			else {
				sprintf(&buffer[strlen(buffer)], "%i\n", reward->amount);
			}
		}
		break;
		case 0x06:
		{
			const QuestReward006* reward = reinterpret_cast<const QuestReward006*>(data);
			sprintf(&buffer[strlen(buffer)], "Reward Player with regeneration [%i% HP, %i% MP]%s\n", reward->percentHP, reward->percentMP, reward->partyOption > 0 ? " including his party" : "");
		}
		break;
		case 0x07:
		{
			const QuestReward007* reward = reinterpret_cast<const QuestReward007*>(data);
			sprintf(&buffer[strlen(buffer)], "Reward Player with a Warp to Map %i; Coordinates: [%i, %i]%s\n", reward->mapId, reward->x, reward->y, reward->partyOption > 0 ? " including his party" : "");
		}
		break;
		case 0x08:
		{
			const QuestReward008* reward = reinterpret_cast<const QuestReward008*>(data);
			sprintf(&buffer[strlen(buffer)], "Reward Player with freshly spawned monster[%i]", reward->monsterId);
			if (reward->targetType == 0x00) {
				sprintf(&buffer[strlen(buffer)], " around the trigger of the quest (i.e. Player); Amount: %i\n", reward->amount);
			}
			else if (reward->targetType == 0x01) {
				sprintf(&buffer[strlen(buffer)], " around the quest-selected NPC/EventObject; Amount: %i\n", reward->amount);
			}
			else {
				sprintf(&buffer[strlen(buffer)], " on Map %i; Coordinates:[%i, %i]; Amount: %i\n", reward->mapId, reward->x, reward->y, reward->amount);
			}
		}
		break;
		case 0x09:
		{
			const QuestReward009* reward = reinterpret_cast<const QuestReward009*>(data);
			sprintf(&buffer[strlen(buffer)], "Reward new questTrigger [Length: %i => ", reward->triggerLength);
			for (unsigned int j = 0; j < reward->triggerLength; j++)
				sprintf(&buffer[strlen(buffer)], "%c", reward->triggerName[j]);
			sprintf(&buffer[strlen(buffer)], "]\n");
		}
		break;
		case 0x0A:
		{
					 sprintf(&buffer[strlen(buffer)], "Reward player with a stat reset\n");
		}
		break;
		case 0x0B:
		{
					 const QuestReward011* reward = reinterpret_cast<const QuestReward011*>(data);
					 sprintf(&buffer[strlen(buffer)], "Reward %sNPC with a VarChange; Var[0x%x] %s= %i\n", (reward->isTargetSelected > 0 ? "selected " : ""), reward->varType, OperationService::operationName(reward->operation), reward->value);
		}
		break;
		case 0x0C:
		{
			const QuestReward012* reward = reinterpret_cast<const QuestReward012*>(data);
			sprintf(&buffer[strlen(buffer)], "Reward NPC-%s\n", reward->messageType == 0x00 ? "LocalMessage" : reward->messageType == 0x01 ? "Shout" : "Announcement");
		}
		break;
		case 0x0D:
		{
			const QuestReward013* reward = reinterpret_cast<const QuestReward013*>(data);
			sprintf(&buffer[strlen(buffer)], "Reward QuestTrigger [%i => \n", reward->triggerLength);
			for (unsigned int j = 0; j < reward->triggerLength; j++) {
				sprintf(&buffer[strlen(buffer)], "%c", reward->triggerName[j]);
			}
			sprintf(&buffer[strlen(buffer)], "] after %i seconds", reward->seconds);
		}
		break;
		case 0x0E:
		{
			const QuestReward014* reward = reinterpret_cast<const QuestReward014*>(data);
			sprintf(&buffer[strlen(buffer)], "Reward change to SkillId %i [Operation(?): %i]\n", reward->skillId, reward->operation);
		}
		break;
		case 0x0F:
		{
			const QuestReward015* reward = reinterpret_cast<const QuestReward015*>(data);
			sprintf(&buffer[strlen(buffer)], "Reward player with %s switch %i\n", reward->operation == 0x00 ? "reset" : "set", reward->switchId>>3);
		}
		break;
		case 0x10:
		{
			const QuestReward016* reward = reinterpret_cast<const QuestReward016*>(data);
			sprintf(&buffer[strlen(buffer)], "Reward player with reset switch group %i\n", reward->groupId);
		}
		break;
		case 0x11:
		{
			sprintf(&buffer[strlen(buffer)], "Reset all switches\n");
		}
		break;
		case 0x12:
		{
			const QuestReward018* reward = reinterpret_cast<const QuestReward018*>(data);
			sprintf(&buffer[strlen(buffer)], "Reward player [UNKNOWN]; DataCount %i with total length %i\n", reward->dataCount, reward->header.length);
			for (unsigned int i = 0; i < reward->header.length - 0x08; i++) {
				sprintf(&buffer[strlen(buffer)], "%c", data[i + 0x08]);
			}
			sprintf(&buffer[strlen(buffer)], "\n");
		}
		break;
		/*
		case 0x13:
		{
					 const QuestCondition019* cond = reinterpret_cast<const QuestCondition019*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if serverTime weekDay == %i, and Time between %i:%i and %i:%i\n", cond->weekDay, cond->hourStart, cond->minuteStart, cond->hourEnd, cond->minuteEnd);
		}
			break;
		case 0x14:
		{
					 const QuestCondition020* cond = reinterpret_cast<const QuestCondition020*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if teamId >= %i && <= %i\n", cond->firstNum, cond->secondNum);
		}
			break;
		case 0x15:
		{
					 const QuestCondition021* cond = reinterpret_cast<const QuestCondition021*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if distance to Object %i <= %i\n", cond->selectedObjType, cond->radius);
		}
			break;
		case 0x16:
		{
					 const QuestCondition022* cond = reinterpret_cast<const QuestCondition022*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check for point [%i, %i]\n", cond->x, cond->y);
		}
			break;
		case 0x17:
		{
					 const QuestCondition023* cond = reinterpret_cast<const QuestCondition023*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if player %s\n", cond->isRegistered > 0 ? "is a ClanMember" : "is not a ClanMember");
		}
			break;
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		{
					 const QuestCondition024* cond = reinterpret_cast<const QuestCondition024*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if clan stat %s %i\n", OperationService::operationName(cond->operation), cond->pointType);
		}
			break;
		case 0x1C:
		{
					 const QuestCondition028* cond = reinterpret_cast<const QuestCondition028*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check clan money %s %i\n", OperationService::operationName(cond->operation), cond->moneyAmount);
		}
			break;
		*/
		//0x880be03.log
		case 0x1D:
		{
			const QuestReward029* reward = reinterpret_cast<const QuestReward029*>(data);
			sprintf(&buffer[strlen(buffer)], "Reward new Trigger/Script(?): [Length %i => ", reward->scriptLength);
			for (unsigned int j = 0; j < reward->scriptLength; j++) {
				sprintf(&buffer[strlen(buffer)], "%c", reward->script[j]);
			}
			sprintf(&buffer[strlen(buffer)], "]\n");
		}
		break;
		/*
		case 0x1E:
		{
					 const QuestCondition030* cond = reinterpret_cast<const QuestCondition030*>(data);
					 sprintf(&buffer[strlen(buffer)], "Check if clan skill %s %i (%i)\n", OperationService::operationName(cond->operation), cond->skillIdFirst, cond->skillIdSecond);
		}
		break;
		*/
		default:
			sprintf(&buffer[strlen(buffer)], "Unknown action format\n");
	}
	result = std::string(buffer);
	result += std::string("\n");
	return result;
}