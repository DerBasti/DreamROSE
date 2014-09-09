#include "Monster.h"
#include "Player.h"
#include "Drop.h"
#include "..\WorldServer.h"

Monster::Monster(const NPCData* newData, const AIP* newAi, const WORD mapId, const Position& pos) {
	this->constructor(newData, newAi, mapId, pos);
	this->entityInfo.type = Entity::TYPE_MONSTER;
	this->updateStats();

	this->owner = nullptr;
}

Monster::~Monster() {
	if(this->spawn != nullptr) {
		this->spawn->setCurrentlySpawned( this->spawn->getCurrentlySpawned()-1 );
	}
	this->data = nullptr;
	this->ai = nullptr;
	this->owner = nullptr;
}

//Give Exp and drop something if suitable
void Monster::onDeath() {
	//Get the total amount of damage to determine the exp percentages
	//Also: get the best dmg dealer
	DWORD totalAmountOfDamage = 0x00;
	WORD highestDealer = 0x00; DWORD highestDmg = 0x00;
	for(unsigned int i=0;i<this->damageDealers.size();i++) {
		DWORD curDmg = this->damageDealers.getValue(i);
		totalAmountOfDamage += curDmg;

		if(curDmg > highestDmg) {
			highestDmg = curDmg;
			highestDealer = this->damageDealers.getKey(i);
		}
	}
	//Calculate dealt percentages
	DWORD basicExp = this->data->getExpPerLevel() * this->data->getLevel();
	float modifier = (totalAmountOfDamage / static_cast<float>(this->data->getMaxHP()+1));
	DWORD totalExp = static_cast<DWORD>(basicExp * modifier);

	Monster* mon = nullptr; Player* player = nullptr;
	for(unsigned int i=0;i<this->damageDealers.size();i++) {
		double expPercent = this->damageDealers.getValue(i) / static_cast<double>(highestDmg);

		Entity* dealer = mainServer->getEntity(this->getMapId(), this->damageDealers.getKey(i));
		try {
			switch(dealer->getEntityType()) {
				case Entity::TYPE_MONSTER:
					mon = dynamic_cast<Monster*>(dealer);
					if(mon->getOwner() == nullptr)
						break;
					//TODO: SUMMON DEALS DMG
				break;
				case Entity::TYPE_PLAYER:
					player = dynamic_cast<Player*>(dealer);
					player->addExperience( static_cast<DWORD>(expPercent * totalExp) );
				break;
				default:
					throw TraceableExceptionARGS("EntityType %i dealt damage!", dealer->getEntityType());
			}
		} catch( std::exception& ex) {
			std::cout << ex.what() << "\n";
		}
	}
	//Now we get to the drop
	Entity* dropOwner = mainServer->getEntity(this->getMapId(), highestDealer);
	BYTE levelDiff = 0x00;
	if(this->getLevel() < dropOwner->getLevel())
		levelDiff = 0x00;

	else if(this->getLevel() - dropOwner->getLevel() > 16)
		levelDiff = 0x10; 

	else
		levelDiff = this->getLevel() - dropOwner->getLevel();

	//Drop Money
	Item toDrop;
	if(QuickInfo::random(100) < this->data->getMoneyPercentage()) {
		toDrop.type = ItemType::MONEY;
		toDrop.amount = static_cast<WORD>((this->getLevel() + 17) * (this->getLevel() + 18) * 0.175f);
		toDrop.amount += toDrop.amount * QuickInfo::random(10) / 100;
	} else {
		//10 attempts to drop something (for now)
		STBEntry& entry = mainServer->getDropTable(this->data->getDropTableId());
		WORD itemId = 0x00;
		for(unsigned int i=0;i<10;i++) {
			itemId = entry.getColumn<WORD>(static_cast<WORD>(levelDiff * 1.5 + QuickInfo::random(levelDiff) + QuickInfo::random(10)));
			
			toDrop.type = static_cast<BYTE>(itemId / 1000);
			toDrop.id = itemId % 1000;
			if(itemId > 0) {
				if(itemId > 10000) {
					toDrop.amount = QuickInfo::random(5) + 1;
				} else {
					toDrop.amount = 1;
				}
				break;
			}
		}
	}
	if(toDrop.type == ItemType::MONEY) { 
		new Drop(dropOwner, this->getPositionCurrent(), toDrop.amount, dropOwner != nullptr);
	} else {
		new Drop(dropOwner, this->getPositionCurrent(), toDrop, dropOwner != nullptr);
	}
	if (dropOwner->isPlayer()) {
		dynamic_cast<Player*>(dropOwner)->sendQuestTriggerViaMonster(this->getTypeId());
	}
	/*
	const DWORD resultHash = QuestService::runQuest(dropOwner, this->data->getQuestHash());
	if (dropOwner->isPlayer() && resultHash > 0) {
		dynamic_cast<Player*>(dropOwner)->sendQuestTriggerViaMonster(this->getTypeId());
	}
	*/
	AIService::run(this, AIP::ON_SELF_DEATH, this->getTarget(), totalAmountOfDamage);
	this->setTarget(nullptr);
}