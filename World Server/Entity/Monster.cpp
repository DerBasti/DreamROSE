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

void Monster::addDamage(Entity* enemy, const DWORD amount) {
	WORD randomChance = rand() % 100;
	if(this->damageDealers.containsKey(enemy->getClientId())) {
		WORD id = enemy->getClientId();
		this->damageDealers.getValueByKey(id) += amount;
	} else {
		this->damageDealers.add(enemy->getClientId(), amount);
	}
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
	QWORD basicExp = this->data->getExpPerLevel() * this->data->getLevel();
	DWORD totalExp = basicExp * (totalAmountOfDamage * 100 / (this->data->getMaxHP()+1));

	Monster* mon = nullptr; Player* player = nullptr;
	for(unsigned int i=0;i<this->damageDealers.size();i++) {
		double expPercent = this->damageDealers.getValue(i) / static_cast<double>(highestDmg);

		Entity* dealer = mainServer->getEntity(this->damageDealers.getKey(i));
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
	Entity* dropOwner = mainServer->getEntity(highestDealer);
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
		new Drop(dropOwner,toDrop.amount, false);
	} else {
		new Drop(dropOwner,toDrop, false);
	}
}