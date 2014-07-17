#include "MessageBox.h"
#include "D:\Programmieren\QuickInfos\QuickInfoFuncDefs.h"

#pragma warning(disable:4996)
void _MessageBox::ShowW(const wchar_t *fmt, ...) {
	ArgConverterW(wResult, fmt);
	MessageBoxW(nullptr, wResult.c_str(), L"Info", 0);
	wResult.clear();
}

void _MessageBox::ShowA(const char *fmt, ...) {
	ArgConverterA(cResult, fmt);
	MessageBoxA(nullptr, cResult.c_str(), "Info", 0);
	cResult.clear();
}

void _MessageBox::ErrorW(const wchar_t *fmt, ...) {
	ArgConverterW(wResult, fmt);
	MessageBoxW(nullptr, wResult.c_str(), L"Error", MB_OK | MB_ICONERROR);
	wResult.clear();
}

void _MessageBox::ErrorA(const char *fmt, ...) {
	ArgConverterA(cResult, fmt);
	MessageBoxA(nullptr, cResult.c_str(), "Error", MB_OK | MB_ICONERROR);
	cResult.clear();
}

void _MessageBox::WarningW(const wchar_t *fmt, ...) {
	ArgConverter(wResult, fmt);
	MessageBoxW(nullptr, wResult.c_str(), L"Warning!", MB_OK | MB_ICONWARNING);
	wResult.clear();
}

void _MessageBox::WarningA(const char *fmt, ...) {
	ArgConverterA(cResult, fmt);
	MessageBoxA(nullptr, cResult.c_str(), "Warning", MB_OK | MB_ICONWARNING);
	cResult.clear();
}

bool _MessageBox::PromptW(const wchar_t *fmt, ...) {
	ArgConverter(wResult, fmt);
	if (MessageBoxW(nullptr, wResult.c_str(), L"IMPORTANT!", MB_OKCANCEL | MB_ICONEXCLAMATION) == MB_OK) {
		return true;
	}
	return false;
}

bool _MessageBox::PromptA(const char *fmt, ...) {
	ArgConverterA(cResult, fmt);
	if (MessageBoxA(nullptr, cResult.c_str(), "IMPORTANT!", MB_OKCANCEL | MB_ICONEXCLAMATION) == MB_OK) {
		return true;
	}
	return false;
}
#pragma warning(default:4996)