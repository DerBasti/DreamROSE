#include "QSD.h"
#include "..\WorldServer.h"

#ifdef __ROSE_USE_VFS__
QSD::QSD(const DWORD id, VFS* pVFS, const char* filePath) {
	this->id = id;
	VFSData vfsData; pVFS->readFile(filePath, vfsData);
	CMyBufferedReader reader(vfsData.data, vfsData.data.size());
	this->read(reader);
}
#else
QSD::QSD(const DWORD id, const char* filePath) {
	this->id = id;
	this->filePath = filePath;
	CMyFile file(this->filePath.c_str(), "rb");
	if (file.exists())
		this->read(file);
}
#endif

template<class FileType> void QSD::read(FileType& file) {
	file.read<DWORD>(); //FileVersion
	DWORD blockCount = file.read<DWORD>();

	char name[0x200] = { 0x00 };
	file.readStringT<WORD, char*>(name);

	this->internalName = name;

	for (unsigned int i = 0; i < blockCount; i++) {

		DWORD recordCount = file.read<DWORD>();

		file.readStringT<WORD, char*>(name);
		QuestEntry* previousEntry = nullptr;
		for (unsigned int j = 0; j < recordCount; j++) {

			bool checkNext = file.read<BYTE>() > 0; //CheckNEXT ? 
			DWORD conditionCount = file.read<DWORD>();
			DWORD actionCount = file.read<DWORD>();

			file.readStringT<WORD, char*>(name);
			QuestEntry* newEntry = new QuestEntry(QSD::makeQuestHash(name), conditionCount, actionCount);
			for (unsigned int k = 0; k < conditionCount; k++) {
				DWORD condLen = file.read<DWORD>();
				Trackable<char> newCond;
				file.readAndAlloc(&newCond, condLen - 4);

				newEntry->addCondition(&newCond);
			}
			for (unsigned int k = 0; k < actionCount; k++) {
				DWORD actLen = file.read<DWORD>();
				Trackable<char> newAct;
				file.readAndAlloc(&newAct, actLen - 4);

				newEntry->addAction(&newAct);
			}
			newEntry->questId = this->id;
			this->questData.insert( std::pair<const DWORD, QuestEntry*>(newEntry->getQuestId(), newEntry) );
			newEntry->previousQuest = previousEntry;
			if (checkNext && previousEntry) {
				previousEntry->nextQuest = newEntry;
			}
			previousEntry = newEntry;
		}
	}
}

const DWORD QSD::makeQuestHash(const char* qsdname) {
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

const DWORD QuestService::makeQuestHash(const char* str) { 
	return QSD::makeQuestHash(str); 
}

template<class _Ty1, class _Ty2> bool QuestService::checkOperation(_Ty1& first, _Ty2& second, BYTE operation) {
	switch (operation) {
		case QuestService::OPERATION_EQUAL:
			return (first == second);
		case QuestService::OPERATION_BIGGER:
			return (first > second);
		case QuestService::OPERATION_BIGGER_EQUAL:
			return (first >= second);
		case QuestService::OPERATION_SMALLER:
			return (first < second);
		case QuestService::OPERATION_SMALLER_EQUAL:
			return (first <= second);
		case QuestService::OPERATION_NOT_EQUAL:
			return (first != second);
	}
	return false;
}

template<class _Ty> static _Ty QuestService::resultOperation(_Ty& first, _Ty& second, BYTE operation) {
	switch (operation) {
		case QuestService::OPERATION_ADDITION:
			return _Ty(first + second);
		case QuestService::OPERATION_SUBTRACTION:
			return _Ty(first - second);
		case QuestService::OPERATION_MULTIPLICATION:
			return _Ty(first * second);
		case QuestService::OPERATION_DIVISION:
			return _Ty(first / second);
		case QuestService::OPERATION_RETURN_RHS:
			return _Ty(second);
	}
	return _Ty(0);
}


DWORD QuestService::currentQuestId;
bool QuestService::runQuest(Entity* entity, const DWORD questId) {
	QuestEntry* quest = mainServer->getQuest(questId);
	if (!quest)
		return false;
	QuestService::currentQuestId = quest->getQuestId(); //Just for debugging purposes
	for (unsigned int i = 0; i < quest->conditions.size(); i++) {
		Trackable<char>& data = quest->conditions[i];
		const QuestHeader* conditionHeader = reinterpret_cast<const QuestHeader*>(data.getData());
		if (!QuestService::checkCondition(entity, conditionHeader))
			return false;
	}
	for (unsigned int i = 0; i < quest->actions.size(); i++) {
		Trackable<char>& data = quest->conditions[i];
		const QuestHeader* actionHeader = reinterpret_cast<const QuestHeader*>(data.getData());
		QuestService::applyActions(entity, actionHeader);
	}
	return true;
}

bool QuestService::checkCondition(Entity* entity, const QuestHeader* conditionHeader) {
	if (!entity || conditionHeader)
		return false;
	switch (conditionHeader->operationType) {
		case 0x00:
			return QuestService::checkSelectedQuest(entity, conditionHeader);
		break;
		case 0x01:
		case 0x02:
			return QuestService::checkQuestVariables(entity, conditionHeader);
		case 0x03:
			return QuestService::checkUserVariables(entity, conditionHeader);
		case 0x04:
			return QuestService::checkItemAmount(entity, conditionHeader);
		case 0x05:
			return QuestService::checkPartyLeaderAndLevel(entity, conditionHeader);
		case 0x06:
			return QuestService::checkDistanceFromPoint(entity, conditionHeader);
		case 0x07:
			return QuestService::checkWorldTime(entity, conditionHeader);
		case 0x08:
			return QuestService::checkRemainingTime(entity, conditionHeader);
		case 0x09:
			return QuestService::checkSkill(entity, conditionHeader);
		//case 0x0A:

	}
	return false;
}

bool QuestService::checkSelectedQuest(Entity* entity, const QuestHeader* conditionHeader) {
	if (entity->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(entity);
	const QuestCondition000* cond = reinterpret_cast<const QuestCondition000*>(conditionHeader);
	return player->searchAndSelectQuest(cond->questId);
}

bool QuestService::checkQuestVariables(Entity* entity, const QuestHeader* conditionHeader) {
	if (entity->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(entity);
	const QuestCondition001* cond = reinterpret_cast<const QuestCondition001*>(conditionHeader);
	for (unsigned int i = 0; i < cond->dataCount; i++) {
		WORD questVar = player->getQuestVariable(cond->data[i].varType, cond->data[i].varNum);
		if (!QuestService::checkOperation(questVar, cond->data[i].amount, cond->data[i].operation))
			return false;
	}
	return true;
}

bool QuestService::checkUserVariables(Entity* entity, const QuestHeader* conditionHeader) {
	const QuestCondition003* cond = reinterpret_cast<const QuestCondition003*>(conditionHeader);
	for (unsigned int i = 0; i < cond->dataCount; i++) {
		DWORD value = entity->getStatType<DWORD>(static_cast<WORD>(cond->data[i].abilityType));
		if (!value)
			return false;
		if (!QuestService::checkOperation(value, cond->data[i].amount, cond->data[i].operation))
			return false;
	}
	return true;
}

bool QuestService::checkItemAmount(Entity* entity, const QuestHeader* conditionHeader) {
	if (!entity || entity->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(entity);
	const QuestCondition004* cond = reinterpret_cast<const QuestCondition004*>(conditionHeader);
	for (unsigned int i = 0; i < cond->dataCount; i++) {
		Item tmpItem(cond->data[i].itemId);
		if (cond->data[i].itemSlot >= ItemType::FACE && cond->data[i].itemSlot <= ItemType::SHIELD) {
			Item eqItem = player->getItemFromInventory(static_cast<WORD>(cond->data[i].itemSlot));
			if (mainServer->buildItemHead(eqItem) != mainServer->buildItemHead(tmpItem))
				return false;
		}
		if (tmpItem.type == ItemType::QUEST) {
			Item item = player->getQuestItem(cond->data[i].itemId);
			if (!QuestService::checkOperation(item.amount, cond->data[i].amount, cond->data[i].operation))
				return false;
		}
		else {
			//TODO
			try {
				throw TraceableExceptionARGS("[QUEST: %i] ITEMTYPE NOT IMPLEMENTED: %i", QuestService::currentQuestId, cond->data[i].itemId);
			}
			catch (std::exception& ex) { std::cout << ex.what() << "\n"; }
		}
	}
	return true;
}

bool QuestService::checkPartyLeaderAndLevel(Entity* entity, const QuestHeader* conditionHeader) {
	if (!entity || entity->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(entity);
	const QuestCondition005* cond = reinterpret_cast<const QuestCondition005*>(conditionHeader);
	try {
		throw TraceableExceptionARGS("[QUEST: %i] PARTY NOT IMPLEMENTED: isLeader = %i, PartyLevel = %i", QuestService::currentQuestId, cond->isLeader, cond->level);
	}
	catch (std::exception& ex) { std::cout << ex.what() << "\n"; }
	return false;
}


bool QuestService::checkDistanceFromPoint(Entity* entity, const QuestHeader* conditionHeader) {
	if (!entity)
		return false;
	const QuestCondition006* cond = reinterpret_cast<const QuestCondition006*>(conditionHeader);
	if (entity->getMapId() != cond->mapId)
		return false;
	Position wantedPos(cond->x, cond->y);
	if (wantedPos.distanceTo(entity->getPositionCurrent()) > cond->radius)
		return false;
	return true;
}

bool QuestService::checkWorldTime(Entity* entity, const QuestHeader* conditionHeader) {
	//I guess it would trigger only once if we didn't do this.
	//BYTE would be too little as value (-> once every 4-5 minutes)
	WORD worldTime = static_cast<WORD>(mainServer->getWorldTime());
	const QuestCondition007* cond = reinterpret_cast<const QuestCondition007*>(conditionHeader);
	if (worldTime >= cond->startTime && worldTime <= cond->endTime)
		return true;
	return false;
}

bool QuestService::checkRemainingTime(Entity* entity, const QuestHeader* conditionHeader) {
	if (!entity || entity->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(entity);
	const QuestCondition008* cond = reinterpret_cast<const QuestCondition008*>(conditionHeader);
	const DWORD passedTime = player->getSelectedQuest()->getPassedTime();
	return QuestService::checkOperation(passedTime, cond->totalTime, cond->operation);
}

bool QuestService::checkSkill(Entity* entity, const QuestHeader* conditionHeader) {
	if (!entity || entity->getEntityType() != Entity::TYPE_PLAYER)
		return false;
	Player* player = dynamic_cast<Player*>(entity);
	const QuestCondition009* cond = reinterpret_cast<const QuestCondition009*>(conditionHeader);
	return true;
}

void QuestService::applyActions(Entity *entity, const QuestHeader *actionHeader) {
	if (!entity || actionHeader)
		return;
	switch (actionHeader->operationType) {
		case 0x01000000:

		break;
	}
}

std::string QuestService::conditionToString(const char* data) {
	const QuestHeader *header = reinterpret_cast<const QuestHeader*>(data);
	char buffer[0x300] = { 0x00 };
	std::string result = "";
	sprintf(buffer, "OperationType: 0x%x\n", header->operationType);
	switch (header->operationType) {
		case 0x00:
		break;
		default:
			sprintf(&buffer[strlen(buffer)], "Unknown condition format\n");
	}
	result = std::string(buffer);
	return result;
}

std::string QuestService::actionToString(const char* data) {
	const QuestHeader *header = reinterpret_cast<const QuestHeader*>(data);
	char buffer[0x300] = { 0x00 };
	std::string result = "";
	sprintf(buffer, "OperationType: 0x%x\n", header->operationType);
	switch (header->operationType) {
		case 0x00:

		break;
		default:
			sprintf(&buffer[strlen(buffer)], "Unknown action format\n");
	}
	result = std::string(buffer);
	return result;
}