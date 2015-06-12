#pragma once

#ifndef __GLOBAL_LOGGER__
#define __GLOBAL_LOGGER__
#include <string>
#include <iostream>
#include <type_traits>

#include <ShlObj.h>
//SYM_*
#include <DbgHelp.h>

#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN

//VA_ARGS
#include <stdlib.h>
#include <stdarg.h>
//ToolSnapshots
#include <tlhelp32.h>

#include "D:\Programmieren\QuickInfos\VarsToString"
#include "D:\Programmieren\Exceptions\CallstackGetter.h"

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned long dword_t;

#pragma comment(lib, "DbgHelp.lib")

class GlobalLogger {
	public:
		class Level {
			private:
				Level() {};
				~Level() {};
			public:
				const static byte_t DEBUG = 0; //All messages
				const static byte_t INFO = 1;
				const static byte_t WARNING = 2;
				const static byte_t FAULT = 3;
				const static byte_t FATAL = 4;
		};
		class StaticStuff {
			private:
				bool showFullPath;
				byte_t logLevel;
			public:
				StaticStuff() {
					showFullPath = false;
					logLevel = GlobalLogger::Level::DEBUG;
				}
				__inline bool isFullPathShown() const { return this->showFullPath; }
				__inline void setIsFullPathShown(bool flag) { this->showFullPath = flag; }
				__inline byte_t getLogLevel() const { return this->logLevel; }
				__inline void setLogLevel(const byte_t level) { this->logLevel = level; }
		};
	private:
		static GlobalLogger* instance;
		static StaticStuff staticStuff;
		const static byte_t LINE_BREAK = 0x0A;

		GlobalLogger() {};
		~GlobalLogger() {};

		template<class _Ty> static void mainRoutine(const byte_t reqLevel, const char* pPreface, const _Ty* msg, va_list args) {
			if (reqLevel < GlobalLogger::getLogLevel())
				return;
			std::basic_string<_Ty> result = QuickInfo::convertVarsToString<_Ty>(msg, args);
			va_end(args);

			std::basic_ostream<_Ty>* pOutput = std::is_same<char, _Ty>::value == true ? reinterpret_cast<std::basic_ostream<_Ty>*>(&std::cout) : reinterpret_cast<std::basic_ostream<_Ty>*>(&std::wcout);

			std::string initPreface = std::string(pPreface);
			std::basic_string<_Ty> preface;

			SYSTEMTIME st; GetSystemTime(&st);
			char buf[0x60] = { 0x00 };
			sprintf_s(buf, "%02i:%02i:%02i,%03i ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			std::string currentTime = std::string(buf);

			preface = CallStack::getAsAscii();
			for (unsigned int i = 0; i < 3; i++) {
				preface = preface.substr(preface.find_first_of(0x0A) + 1);
			}
			preface = preface.substr(0, preface.find_first_of(0x0A));

			//CallStack + Preface (e.g. DEBUG)
			preface += std::basic_string<_Ty>(initPreface.begin(), initPreface.end());
			preface = std::basic_string<_Ty>(currentTime.begin(), currentTime.end()) + preface;
			(*pOutput) << (const _Ty*)preface.c_str() << result.c_str();
		}

		template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value >::type > __inline static void debug(const _Ty* msg, va_list args) {
			return mainRoutine(GlobalLogger::Level::DEBUG, "DEBUG: ", msg, args);
		}
		template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value >::type > __inline static void info(const _Ty* msg, va_list args) {
			return mainRoutine(GlobalLogger::Level::INFO, "INFO: ", msg, args);
		}
		template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value >::type > __inline static void warning(const _Ty* msg, va_list args) {
			return mainRoutine(GlobalLogger::Level::WARNING, "WARNING: ", msg, args);
		}
		template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value >::type > __inline static void fault(const _Ty* msg, va_list args) {
			return mainRoutine(GlobalLogger::Level::FAULT, "FAULT: ", msg, args);
		}
		template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value >::type > __inline static void fatal(const _Ty* msg, va_list args) {
			return mainRoutine(GlobalLogger::Level::FATAL, "FATAL: ", msg, args);
		}
	public:
		static GlobalLogger& getLogger() {
			static GlobalLogger instance;
			return instance;
		}
		static __inline void setLogLevel(const byte_t newLogLevel) { staticStuff.setLogLevel(newLogLevel); }
		static __inline byte_t getLogLevel() { return staticStuff.getLogLevel(); }
		static __inline bool isFullPathShown() { return staticStuff.isFullPathShown(); }
		static __inline void setIsFullPathShown(bool flag) { staticStuff.setIsFullPathShown(flag); }

		template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value >::type > static __inline void debug(const _Ty* msg, ...) { va_list args; va_start(args, msg); return GlobalLogger::debug<_Ty>(msg, args); }
		template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value >::type > static __inline void info(const _Ty* msg, ...) { va_list args; va_start(args, msg); return GlobalLogger::info<_Ty>(msg, args); }
		template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value >::type > static __inline void warning(const _Ty* msg, ...) { va_list args; va_start(args, msg); return GlobalLogger::warning<_Ty>(msg, args); }
		template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value >::type > static __inline void fault(const _Ty* msg, ...) { va_list args; va_start(args, msg); return GlobalLogger::fault<_Ty>(msg, args); }
		template<class _Ty, class = typename std::enable_if< is_character_type<_Ty>::value >::type > static __inline void fatal(const _Ty* msg, ...) { va_list args; va_start(args, msg); return GlobalLogger::fatal<_Ty>(msg, args); }
};

#endif //__GLOBAL_LOGGER__