#include "Monster.h"
#include "Player.h"
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
	if(randomChance < 40 || this->damageDealers.size() == 0) { //40% Chance of adding dmg
		if(this->damageDealers.containsKey(enemy->getClientId())) {
			WORD id = enemy->getClientId();
			this->damageDealers.getValueByKey(id) += amount;
		} else {
			this->damageDealers.add(enemy->getClientId(), amount);
		}
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
	DWORD totalExp = this->data->getExpPerLevel() * this->data->getLevel();

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
}