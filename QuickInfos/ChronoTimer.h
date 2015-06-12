#pragma once

#ifndef __CHRONO_TIMER__
#define __CHRONO_TIMER__

#include <chrono>
#include <windows.h>


class ChronoTimer {
private:
	using ClockType = std::chrono::system_clock;
	bool started;
	std::chrono::time_point<ClockType> timestamp;
public:
	ChronoTimer() {
		this->timestamp = ClockType::now();
		started = true;
	};
	~ChronoTimer() {};

	static const long long getNow() {
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(ClockType::now().time_since_epoch());
		return ms.count();
	}

	void start() {
		started = true;
		this->update();
	}
	void start(const long long timeReduction) {
		this->timestamp = ClockType::now();
		this->timestamp -= std::chrono::duration<long long, std::milli>(timeReduction);
	}
	void stop() {
		started = false;
	}
	const long long getDuration() const { 
		if (!started) {
			return 0;
		}
		auto d = std::chrono::duration_cast<std::chrono::milliseconds>(ClockType::now() - this->timestamp);
		return d.count();
	}
	__inline void update() { 
		this->timestamp = ClockType::now(); 
	}
	__inline bool hasStarted() const { return this->started; }

};

#endif //__CHRONO_TIMER__