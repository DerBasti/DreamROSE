#include "Structures.h"
#undef max

Buffs::Buffs() {
	this->currentTime = clock();
}

Buffs::~Buffs() {
}


Buffs& Buffs::operator=(const Buffs& rhs) {
	for (unsigned int i = 0; i < Buffs::TOTAL_COUNT; i++) {
		this->buffs[i] = rhs.buffs[i];
	}
	this->currentTime = rhs.currentTime;
	return (*this);
}

void Buffs::internalClear(dword_t clearFlags) {
	if (this->_getActiveBuffCount() == 0)
		return;
	size_t idx = 0x00;
	byte_t curByte = 0x00;
	//As long as there are buffs, iterate through them and
	//get the visuality-byte. Shift 1 times visuality-bits
	//to get the dword_t value. bit-AND the value with the
	//wanted clear flag -> in case it's a wanted buff, remove it
	while(idx < Buffs::TOTAL_COUNT) {
		if( this->buffs[idx].amount > 0 && (1<<idx) & clearFlags ) {
			this->removeBuff(idx);
			continue;
		}
		idx++;
	}
}

const byte_t Buffs::_getActiveBuffCount() const {
	byte_t result = 0;
	for (unsigned int i = 0; i < Buffs::TOTAL_COUNT; i++) {
		result += static_cast<byte_t>(this->buffs[i].endTime != 0);
	}
	return result;
}

void Buffs::clearBuff(byte_t clearType) {
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
			this->internalClear(std::numeric_limits<dword_t>().max());
	}
}

bool Buffs::addBuff(const byte_t bitFromVisuality, const word_t amount, const dword_t durationInMilliseconds) {
	if(bitFromVisuality >= TOTAL_COUNT)
		return false;
	if (this->buffs[bitFromVisuality].endTime != 0) {
		if (this->buffs[bitFromVisuality].amount > amount) {
			return false;
		}
	}
	this->buffs[bitFromVisuality].endTime = clock() + durationInMilliseconds;
	this->buffs[bitFromVisuality].amount = amount;
	return true;
}

void Buffs::removeBuff(const size_t pos) {
	this->buffs[pos].amount = 0;
	this->buffs[pos].endTime = 0;
}

bool Buffs::checkBuffs() {
	this->currentTime = clock();
	dword_t idx = 0x00;
	bool result = false;
	//Iterate through all buffs to see whether they expired
	while(idx < Buffs::TOTAL_COUNT ) {
		clock_t buffTime = this->buffs[idx].endTime;
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

const word_t Buffs::getStatusAmount(byte_t visualityBit) {
	return this->buffs[visualityBit].amount;
}

const dword_t Buffs::getVisuality(byte_t type) {
	dword_t result = 0x00;
	//Iterate through all buff-bits and bit-shift it to get a 
	//DWORD
	for(unsigned int i=0;i<Buffs::TOTAL_COUNT;i++) {
		result |= this->buffs[i].amount > 0 ? 1<<i : 0;
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