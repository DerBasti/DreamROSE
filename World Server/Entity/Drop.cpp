#include "Drop.h"
#include "..\WorldServer.h"

void Drop::construct(Entity* giver, const position_t& pos, bool isPublicDomain) {
	if(!isPublicDomain)
		this->owner = giver;
	else
		this->owner = nullptr;
	
	this->entityInfo.pkFlagId = giver->getPlayerKillFlag();
	this->setMapId(giver->getMapId());
	this->position.current = pos;
	this->position.destination = pos;

	mainServer->getMap(this->getMapId())->assignLocalId(this);

	this->entityInfo.type = Entity::TYPE_DROP;

	Map::Sector *sector = giver->getSector();
	this->setSector(sector);
	this->checkVisuality();
}

Drop::Drop(Entity* dropGiver, dword_t zulyAmount, bool isPublicDomain) {
	this->item.id = 0xCCCC;
	this->item.type = ItemType::MONEY;
	this->item.amount = zulyAmount;
	
	this->construct(dropGiver, dropGiver->getPositionCurrent().calcNewPositionWithinRadius(500), isPublicDomain);
}

Drop::Drop(Entity* dropGiver, const Item& item, bool isPublicDomain) {
	this->item = item;
	this->construct(dropGiver, dropGiver->getPositionCurrent().calcNewPositionWithinRadius(500), isPublicDomain);
}

Drop::Drop(Entity* dropGiver, const position_t& pos, dword_t zulyAmount, bool isPublicDomain) {
	this->item.id = 0xCCCC;
	this->item.type = ItemType::MONEY;
	this->item.amount = zulyAmount;

	this->construct(dropGiver, pos, isPublicDomain);
}

Drop::Drop(Entity* dropGiver, const position_t& pos, const Item& itemToDrop, bool isPublicDomain) {
	this->item = itemToDrop;
	this->construct(dropGiver, pos, isPublicDomain);
}

Drop::~Drop() {

	for(unsigned int i=0;i<this->visibleSectors.size();i++) {
		this->removeSectorVisually(this->visibleSectors.getValueAtPosition(i));
	}
}