#pragma once

#ifndef __ROSE_ZMO__
#define __ROSE_ZMO__

#include "..\..\Common\Definitions.h"
#include <string>
#include <vector>

#ifdef __ROSE_USE_VFS__
#include "VFS.h"
#endif

#ifdef __ZMO_OUTPUT__
#define __ROSE_ZMO_OUTPUT__
#endif

#undef min
#undef max

class ZMO {
	public:
		const static word_t MOTION_MELEE_ATTACK = 21;
		const static word_t MOTION_RANGED_ATTACK = 22;
		const static word_t MOTION_MAGIC_ATTACK = 23;
		const static word_t MOTION_SKILL_MELEE = 24;
		const static word_t MOTION_SKILL_RANGED = 26;

		class AnimationInfo {
			private:
				/* Stores the time in ms it takes to play the animation with default speed*/
				word_t defaultTime;

				/* Self-explaining*/ 
				word_t framesPerSecond;

				/* Stores the amount of frames the animation contains*/
				word_t framesInTotal;

				friend class ZMO;
			public:
				AnimationInfo() {	}
				~AnimationInfo() { }
				const word_t getDefaultTime() const { return this->defaultTime; }
				const word_t getAdjustedTime(const word_t speed) const { return this->defaultTime * 100 / speed; }
				const word_t getTimeAtFrame(const word_t frame, const word_t speed = 100) const {
					float percentage = frame / static_cast<float>(this->getFrameAmount());
					return static_cast<word_t>(percentage * this->getAdjustedTime(speed));
				}
				const word_t getFPS() const { return this->framesPerSecond; }
				const word_t getFrameAmount() const { return this->framesInTotal; }
		};

		class TimingInfo {
			private:
				struct Frame {
					word_t timeToReach;
					word_t motionType;
				};
				/* FOR DEBUG: Maps a given frame with a time span which is necessary to reach it*/
				std::map<word_t, struct Frame> attacksAtFrames;
				friend class ZMO;
			public:
				TimingInfo() { }
				~TimingInfo() { 
					this->attacksAtFrames.clear(); 
				}

				const word_t getAttackFrameAmount() const { 
					return this->attacksAtFrames.size(); 
				}
				const word_t getNextAttackFrame(const word_t currentFrame) const {
					for (auto i : this->attacksAtFrames) {
						if (i.first >= currentFrame)  {
							return i.first;
						}
					}
					return std::numeric_limits<word_t>::max();
				}
				/* DEBUG: Returns the read motion type (>= 20 && <= 40)*/
				const word_t getFrameType_DEBUG(const word_t frame) const {
					for (auto i : this->attacksAtFrames) {
						if (i.first == frame) {
							return i.second.motionType;
						}
					}
					return 0;
				}
				const word_t getAttackFrameById(const word_t id) const {
					word_t curId = 0;
					for (auto i : this->attacksAtFrames) {
						if (curId == id) {
							return i.first;
						}
						curId++;
					}
					return std::numeric_limits<word_t>::max();
				}
		};

	private:
		std::string filePath;

		AnimationInfo animationInfo;
		TimingInfo timingInfo;
		Trackable<word_t> content;

		template<class FileType> void init(FileType& file);
	public:
#ifdef __ROSE_USE_VFS__
		ZMO() {}
		ZMO(VFS* pVFS, const char *filePath);
#else
		ZMO(const char* filePath);
#endif
		~ZMO();

		/* Returns the read motion type (>= 20 && <= 40)*/
		const word_t getFrameType(const word_t frame) const {
			if (frame >= this->content.size())
				return 0;

			return this->content[frame];
		}
		__inline const TimingInfo& getTimingInfo() const { return this->timingInfo; }
		__inline const AnimationInfo& getInfo() const { return this->animationInfo; }
};

#endif //__ROSE_ZMO__