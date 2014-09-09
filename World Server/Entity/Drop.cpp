#include "Drop.h"
#include "..\WorldServer.h"

void Drop::construct(Entity* giver, const Position& pos, bool isPublicDomain) {
	if(!isPublicDomain)
		this->owner = giver;
	else
		this->owner = nullptr;
	
	this->setMapId(giver->getMapId());
	this->position.current = pos;
	this->position.destination = pos;

	mainServer->getMap(this->getMapId())->assignClientID(this);

	this->entityInfo.type = Entity::TYPE_DROP;

	MapSector *sector = giver->getSector();
	this->setSector(sector);
	this->checkVisuality();
}

Drop::Drop(Entity* dropGiver, DWORD zulyAmount, bool isPublicDomain) {
	this->item.id = 0xCCCC;
	this->item.type = ItemType::MONEY;
	this->item.amount = zulyAmount;
	
	this->construct(dropGiver, dropGiver->getPositionCurrent().calcNewPositionWithinRadius(500), isPublicDomain);
}

Drop::Drop(Entity* dropGiver, const Item& item, bool isPublicDomain) {
	this->item = item;
	this->construct(dropGiver, dropGiver->getPositionCurrent().calcNewPositionWithinRadius(500), isPublicDomain);
}

Drop::Drop(Entity* dropGiver, const Position& pos, DWORD zulyAmount, bool isPublicDomain) {
	this->item.id = 0xCCCC;
	this->item.type = ItemType::MONEY;
	this->item.amount = zulyAmount;

	this->construct(dropGiver, pos, isPublicDomain);
}

Drop::Drop(Entity* dropGiver, const Position& pos, const Item& itemToDrop, bool isPublicDomain) {
	this->item = itemToDrop;
	this->construct(dropGiver, pos, isPublicDomain);
}

Drop::~Drop() {
	for(unsigned int i=0;i<this->visibleSectors.size();i++) {
		this->removeSectorVisually(this->visibleSectors.getValue(i));
	}
}