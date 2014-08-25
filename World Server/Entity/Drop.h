#pragma once

#ifndef __ROSE_DROP__
#define __ROSE_DROP__

#include "Entity.h"

class PickDropMessage {
	private:
		PickDropMessage() {}
		~PickDropMessage() {}
	public:
		static const BYTE OKAY = 0x00;
		static const BYTE NOT_OWNER = 0x02;
		static const BYTE INVENTORY_FULL = 0x03;
};

class Drop : public Entity {
	private:
		Entity* owner;
		Item item;
		void construct(Entity* dropGiver, const Position& pos, bool isPublicDomain);
			
	public:
		//e.g. drop from client
		Drop(Entity* dropGiver, DWORD zulyAmount, bool isPublicDomain);
		Drop(Entity* dropGiver, const Item& itemToDrop, bool isPublicDomain);
		Drop(Entity* dropGiver, const Position& pos, DWORD zulyAmount, bool isPublicDomain);
		Drop(Entity* dropGiver, const Position& pos, const Item& itemToDrop, bool isPublicDomain);
		~Drop();

		__inline bool isZulyDrop() const { return this->item.type == ItemType::MONEY; }
		__inline Entity* getOwner() const { return this->owner; }
		__inline const Item& getItem() const { return this->item; }
};

#endif //__ROSE_DROP__