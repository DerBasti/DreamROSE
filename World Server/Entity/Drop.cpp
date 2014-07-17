#include "Drop.h"
#include "..\WorldServer.h"

void Drop::construct(Entity* giver, bool isPublicDomain) {
	if(!isPublicDomain)
		this->owner = giver;
	
	this->setMapId(giver->getMapId());
	this->position.current = giver->getPositionCurrent();
	this->position.destination = this->position.current;

	mainServer->assignClientID(this);

	this->entityInfo.type = Entity::TYPE_DROP;

	MapSector *sector = giver->getSector();
	this->setSector(sector);
	this->checkVisuality();
}

Drop::Drop(Entity* dropGiver, DWORD zulyAmount, bool isPublicDomain) {
	this->item.type = ItemType::MONEY;
	this->item.id = 0x00;
	this->item.amount = zulyAmount;
	
	this->construct(dropGiver, isPublicDomain);
}

Drop::~Drop() {

}

Drop::Drop(Entity* dropGiver, const Item& item, bool isPublicDomain) {
	this->item = item;
	
	this->construct(dropGiver, isPublicDomain);
}