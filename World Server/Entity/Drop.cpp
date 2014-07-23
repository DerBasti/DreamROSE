#include "Drop.h"
#include "..\WorldServer.h"

void Drop::construct(Entity* giver, bool isPublicDomain) {
	if(!isPublicDomain)
		this->owner = giver;
	else
		this->owner = nullptr;
	
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
	this->item.id = 0xCCCC;
	this->item.type = ItemType::MONEY;
	this->item.amount = zulyAmount;
	
	this->construct(dropGiver, isPublicDomain);
}

Drop::Drop(Entity* dropGiver, const Item& item, bool isPublicDomain) {
	this->item = item;
	
	this->construct(dropGiver, isPublicDomain);
}

Drop::~Drop() {
	for(unsigned int i=0;i<this->visibleSectors.size();i++) {
		this->removeSectorVisually(this->visibleSectors.getValue(i));
	}
}