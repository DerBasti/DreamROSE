#include "Drop.h"
#include "..\WorldServer.h"

void Drop::construct(const WORD mapId, const Position& pos) {
	this->setMapId(mapId);
	this->position.current = pos;
	this->position.destination = pos;

	this->entityInfo.type = Entity::TYPE_DROP;
}

Drop::Drop(const WORD mapId, const Position& pos, DWORD zulyAmount) {
	this->isDropZulies = true;
	this->zuly = zulyAmount;

	this->construct(mapId, pos);
}

Drop::Drop(const WORD mapId, const Position& pos, const Item& item) {
	this->isDropZulies = false;
	this->item = item;
	
	this->construct(mapId, pos);
}