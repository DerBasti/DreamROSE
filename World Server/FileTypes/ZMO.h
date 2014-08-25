#pragma once

#ifndef __ROSE_ZMO__
#define __ROSE_ZMO__

#include "..\..\Common\Definitions.h"
#include "..\..\QuickInfos\QuickInfo.h"
#include <string>

#ifdef __ROSE_USE_VFS__
#include "VFS.h"
#endif

#ifdef __ZMO_OUTPUT__
#define __ROSE_ZMO_OUTPUT__
#endif

class ZMO {
	public:
		const static WORD MOTION_MELEE_ATTACK = 21;
		const static WORD MOTION_RANGED_ATTACK = 22;
	private:
		std::string filePath;
		DWORD framesPerSecond;
		DWORD totalFrameCount;
		WORD totalAnimationTime;
		std::vector<WORD> attackTimers;

		template<class FileType> void init(FileType& file);
	public:
#ifdef __ROSE_USE_VFS__
		ZMO() {}
		ZMO(VFS* pVFS, const char *filePath);
#else
		ZMO(const char* filePath);
#endif
		~ZMO();

		__inline DWORD getFPS() const { return this->framesPerSecond; }
		__inline DWORD getFrameCount() const { return this->totalFrameCount; }
		__inline WORD getTotalAnimationTime() const { return this->totalAnimationTime; }
		__inline BYTE getAttackTimerCount() const { return this->attackTimers.size(); }
		__inline WORD getAttackTimer(const BYTE timerId) { return this->attackTimers.at(timerId); }
};

#endif //__ROSE_ZMO__