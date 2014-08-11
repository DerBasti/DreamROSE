#pragma once

#ifndef __ROSE_ZMO__
#define __ROSE_ZMO__

#include "..\..\Common\Definitions.h"
#include "..\..\QuickInfos\QuickInfo.h"
#include <string>

class ZMO {
	public:
		struct Channel {
			DWORD type;
			DWORD index;
			DWORD frameCount;
		};
	private:
		std::string filePath;
		WORD framesPerSecond;
		WORD totalFrameCount;

		template<class FileType> void init(FileType& file);
	public:
#ifdef __ROSE_USE_VFS__
		ZMO(char* fileBuf, const DWORD fileLen);
#else
		ZMO(const char* filePath);
#endif
		~ZMO();

		__inline WORD getFPS() const { return this->framesPerSecond; }
		__inline WORD getFrameCount() const { return this->totalFrameCount; }
		//__inline WORD getActionPointCount() const { return this->actionPoints.size(); }
		//__inline WORD getActionPoint(const WORD point) const { return this->actionPoints[point]; }
};

#endif //__ROSE_ZMO__