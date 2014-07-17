#pragma once

#include <cstdarg>
#include <iostream>

typedef unsigned long DWORD;

class _MessageBox {
	private:
		_MessageBox() { };
		~_MessageBox() { };
	public:
		static void ShowW(const wchar_t *fmt, ...);
		static void ShowA(const char *fmt, ...);
		
		static void WarningW(const wchar_t *fmt, ...);
		static void WarningA(const char *fmt, ...);

		static void ErrorW(const wchar_t *fmt, ...);
		static void ErrorA(const char *fmt, ...);
		
		static bool PromptW(const wchar_t *fmt, ...);
		static bool PromptA(const char *fmt, ...);
};

#define Show ShowW
#define Prompt PromptW
#define Warning WarningW
#define Error ErrorW