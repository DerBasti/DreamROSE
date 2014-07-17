#pragma once

#ifndef __ROSE_DROP__
#define __ROSE_DROP__

#include "Entity.h"

class Drop : public Entity {
	private:
		Item item;
		DWORD zuly;

		bool isDropZulies;
		void construct(const WORD mapId, const Position& pos);
	public:
		Drop(const WORD mapId, const Position& pos, DWORD zulyAmount);
		Drop(const WORD mapId, const Position& pos, const Item& itemToDrop);

		__inline bool isZulyDrop() const { return this->isDropZulies; }
		__inline const Item& getItem() const { return this->item; }
};

#endif //__ROSE_DROP__