#include "MessageBox.h"
#include "D:\Programmieren\QuickInfos\VarsToString"
#include <Windows.h>

#pragma warning(disable:4996)
void _MessageBox::ShowW(const wchar_t *fmt, ...) {
	va_list args; va_start(args, fmt);
	std::wstring wResult = QuickInfo::convertVarsToString(fmt, args);
	va_end(args);
	MessageBoxW(NULL, wResult.c_str(), L"Info", 0);
	wResult.clear();
}

void _MessageBox::ShowA(const char *fmt, ...) {
	va_list args; va_start(args, fmt);
	std::string cResult = QuickInfo::convertVarsToString(fmt, args);
	va_end(args);
	MessageBoxA(NULL, cResult.c_str(), "Info", 0);
	cResult.clear();
}

void _MessageBox::ErrorW(const wchar_t *fmt, ...) {
	va_list args; va_start(args, fmt);
	std::wstring wResult = QuickInfo::convertVarsToString(fmt, args);
	va_end(args);
	MessageBoxW(NULL, wResult.c_str(), L"Error", MB_OK | MB_ICONERROR);
	wResult.clear();
}

void _MessageBox::ErrorA(const char *fmt, ...) {
	va_list args; va_start(args, fmt);
	std::string cResult = QuickInfo::convertVarsToString(fmt, args);
	va_end(args);
	MessageBoxA(NULL, cResult.c_str(), "Error", MB_OK | MB_ICONERROR);
	cResult.clear();
}

void _MessageBox::WarningW(const wchar_t *fmt, ...) {
	va_list args; va_start(args, fmt);
	std::wstring wResult = QuickInfo::convertVarsToString(fmt, args);
	va_end(args);
	MessageBoxW(NULL, wResult.c_str(), L"Warning!", MB_OK | MB_ICONWARNING);
	wResult.clear();
}

void _MessageBox::WarningA(const char *fmt, ...) {
	va_list args; va_start(args, fmt);
	std::string cResult = QuickInfo::convertVarsToString(fmt, args);
	va_end(args);
	MessageBoxA(NULL, cResult.c_str(), "Warning", MB_OK | MB_ICONWARNING);
	cResult.clear();
}

bool _MessageBox::PromptW(const wchar_t *fmt, ...) {
	va_list args; va_start(args, fmt);
	std::wstring wResult = QuickInfo::convertVarsToString(fmt, args);
	va_end(args);
	if (MessageBoxW(NULL, wResult.c_str(), L"IMPORTANT!", MB_OKCANCEL | MB_ICONEXCLAMATION) == MB_OK) {
		return true;
	}
	return false;
}

bool _MessageBox::PromptA(const char *fmt, ...) {
	va_list args; va_start(args, fmt);
	std::string cResult = QuickInfo::convertVarsToString(fmt, args);
	va_end(args);
	if (MessageBoxA(NULL, cResult.c_str(), "IMPORTANT!", MB_OKCANCEL | MB_ICONEXCLAMATION) == MB_OK) {
		return true;
	}
	return false;
}
#pragma warning(default:4996)