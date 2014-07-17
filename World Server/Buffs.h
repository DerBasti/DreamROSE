#pragma once

#ifndef __ROSE_BUFFS__
#define __ROSE_BUFFS__

#include "D:\Programmieren\QuickInfos\QuickInfo.h"
#include <time.h>

//TODO: ADD GETBUFF WITH STATIC VALUES
class Buffs {
	private:
		const static BYTE TOTAL_COUNT = 0x20;
		struct buffValuePair {
			WORD amount;
			BYTE visuality;
			buffValuePair(WORD newAmount, BYTE visualityByte) {
				amount = newAmount;
				visuality = visualityByte;
			}
		};
		SortedList<clock_t, buffValuePair> buffs;
		clock_t currentTime;

		void internalClear(DWORD clearFlags);
	public:
		class Visuality {
			private:
				Visuality() { }
				~Visuality() { }
			public:
				const static BYTE HP_REST = 0x00;
				const static BYTE RESURRECTION = 0x01;
				const static BYTE POISON = 0x02;
				const static BYTE UNKNOWN = 0x03;
				const static BYTE HP_UP = 0x04;
				const static BYTE MP_UP = 0x05;
				const static BYTE MOVEMENT_UP = 0x06;
				const static BYTE MOVEMENT_DOWN = 0x07;

				const static BYTE ATTACKSPEED_UP = 0x08;
				const static BYTE ATTACKSPEED_DOWN = 0x09;
				const static BYTE ATTACKPOWER_UP = 0x0A;
				const static BYTE ATTACKPOWER_DOWN = 0x0B;
				const static BYTE DEFENSE_UP = 0x0C;
				const static BYTE DEFENSE_DOWN = 0x0D;
				const static BYTE MAGIC_DEFENSE_UP = 0x0E;
				const static BYTE MAGIC_DEFENSE_DOWN = 0x0F;

				const static BYTE HITRATE_UP = 0x10;
				const static BYTE HITRATE_DOWN = 0x11;
				const static BYTE CRITRATE_UP = 0x12;
				const static BYTE CRITRATE_DOWN = 0x13;
				const static BYTE DODGERATE_UP = 0x14;
				const static BYTE DODGERATE_DOWN = 0x15;
				const static BYTE MUTED = 0x16;
				const static BYTE SLEEPING = 0x17;
		
				const static BYTE STUNNED = 0x18;
				const static BYTE INVISIBLE_GM = 0x19;
				const static BYTE INVISIBLE_NORMAL = 0x1A;
				const static BYTE SHIELD_REFLECT = 0x1B;
				const static BYTE DAMAGE_UP = 0x1C;
				const static BYTE SUMMON = 0x1D;
				const static BYTE INVINCIBLE = 0x1E;
				const static BYTE FLAMING = 0x1F;
		};
		const static BYTE POSITIVE_BUFFS = 0x01;
		const static BYTE NEGATIVE_BUFFS = 0x02;
		const static BYTE ALL_BUFFS_NORMAL = 0x03;
		const static BYTE ALL_BUFFS_ADMIN_INCLUDED = 0x04;

		const static DWORD POSITIVE_BITS = 
			(1<<Visuality::ATTACKPOWER_UP | 1<<Visuality::ATTACKSPEED_UP |
			1<<Visuality::CRITRATE_UP | 1<<Visuality::DAMAGE_UP | 1<<Visuality::DEFENSE_UP |
			1<<Visuality::DODGERATE_UP | 1<<Visuality::HITRATE_UP | 1<<Visuality::HP_UP |
			1<<Visuality::MAGIC_DEFENSE_UP | 1<<Visuality::MOVEMENT_UP | 1<<Visuality::MP_UP);
		const static DWORD NEGATIVE_BITS = 
			(1<<Visuality::ATTACKPOWER_DOWN | 1<<Visuality::ATTACKSPEED_DOWN | 
			1<<Visuality::CRITRATE_DOWN | 1<<Visuality::DEFENSE_DOWN | 
			1<<Visuality::DODGERATE_DOWN | 1<<Visuality::FLAMING | 1<<Visuality::HITRATE_DOWN |
			1<<Visuality::MAGIC_DEFENSE_DOWN | 1<<Visuality::MOVEMENT_DOWN | 1<<Visuality::MUTED |
			1<<Visuality::POISON | Visuality::SLEEPING | 1<<Visuality::STUNNED);

		Buffs();
		~Buffs();

		Buffs& operator=(const Buffs& rhs);
		void clearBuff(BYTE clearType = 0xFF);
		bool addBuff(BYTE bitFromVisuality, WORD amount, DWORD durationInMilliseconds);
		
		__inline void removeBuff(const size_t pos);
		
		//Returns true if a buff was deleted
		bool checkBuffs();
		
		const DWORD getVisuality(BYTE );
};

#endif //__ROSE_BUFFS__