#include "Buffs.h"
#undef max

Buffs::Buffs() {
	this->buffs.clear();
	this->currentTime = clock();
}

Buffs::~Buffs() {
	this->buffs.clear();
}

Buffs& Buffs::operator=(const Buffs& rhs) {
	this->buffs.clear();
	this->buffs = rhs.buffs;
	this->currentTime = rhs.currentTime;
	return (*this);
}

void Buffs::internalClear(DWORD clearFlags) {
	if(this->buffs.size() == 0)
		return;
	size_t idx = 0x00;
	BYTE curByte = 0x00;
	//As long as there are buffs, iterate through them and
	//get the visuality-byte. Shift 1 times visuality-bits
	//to get the DWORD value. bit-AND the value with the
	//wanted clear flag -> in case it's a wanted buff, remove it
	while(idx < this->buffs.size()) {
		curByte = this->buffs.getValue(idx).visuality;
		if( (1<<curByte) & clearFlags ) {
			this->removeBuff(idx);
			continue;
		}
		idx++;
	}
}

void Buffs::clearBuff(BYTE clearType) {
	switch(clearType) {
		case Buffs::POSITIVE_BUFFS:
			this->internalClear(Buffs::POSITIVE_BITS);
		break;
		case Buffs::NEGATIVE_BUFFS:
			this->internalClear(Buffs::NEGATIVE_BITS);
		break;
		case Buffs::ALL_BUFFS_NORMAL:
			this->internalClear(Buffs::POSITIVE_BITS | Buffs::NEGATIVE_BITS);
		break;
		default: //ALL_BUFFS_ADMIN_INCLUDED
			this->internalClear(std::numeric_limits<DWORD>().max());
	}
}

bool Buffs::addBuff(const BYTE bitFromVisuality, const WORD amount, const DWORD durationInMilliseconds) {
	if(bitFromVisuality >= TOTAL_COUNT)
		return false;
	//Iterate through all buffs to see whether the wanted buff
	//already exists.
	for(unsigned int i=0;i<this->buffs.size();i++) {
		if(this->buffs.getKey(i) == bitFromVisuality) {
			//Existent buff is stronger
			if(this->buffs.getValue(i).amount > amount)
				return false;
			//In case the current buff is equal or stronger,
			//remove and then add it with the new values
			this->removeBuff(i);
			break;
		}
	}
	this->buffs.add(clock() + durationInMilliseconds, buffValuePair(amount,bitFromVisuality));
	return true;
}

void Buffs::removeBuff(const size_t pos) {
	this->buffs.removeAt(pos);
}

bool Buffs::checkBuffs() {
	this->currentTime = clock();
	DWORD idx = 0x00;
	bool result = false;
	//Iterate through all buffs to see whether they expired
	while(idx < this->buffs.size() ) {
		clock_t buffTime = this->buffs.getKey(idx);
		//In case a buff expired, remove it and set result to true
		//in order to tell out-of-scope functions that something
		//changed
		if(buffTime <= this->currentTime) {
			this->removeBuff(idx);
			result = true;
			continue;
		}
		idx++;
	}
	//Result = whether or not a buff was removed.
	//Kind of like a "lever" to do something afterwards.
	return result;
}

const DWORD Buffs::getVisuality(BYTE type) {
	DWORD result = 0x00;
	//Iterate through all buff-bits and bit-shift it to get a 
	//DWORD
	for(unsigned int i=0;i<this->buffs.size();i++) {
		result |= 1<<this->buffs.getValue(i).visuality;
	}
	//In case we want a certain buff type, we remove the opposite
	//via bit-"NOT_AND"
	switch(type) {
		case POSITIVE_BUFFS:
			result &= ~NEGATIVE_BITS;
		break;
		case NEGATIVE_BUFFS:
			result &= ~POSITIVE_BITS;
		break;
	}
	return result;
}