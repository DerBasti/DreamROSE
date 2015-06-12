#pragma once
/*
#ifndef _MY_DEBUG
#define _MY_DEBUG
#endif
*/


#include <xtr1common>
#include <string>

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned long dword_t; 

template<class _Ty> struct is_character_type : std::false_type { };
template<> struct is_character_type<char> : std::true_type{};
template<> struct is_character_type<wchar_t>  : std::true_type{};

template<class _Ty> struct is_string_type : std::false_type { };
template<> struct is_string_type<std::string> : std::true_type{};
template<> struct is_string_type<std::wstring> : std::true_type{};

//#define _OVERLAY_WINDOW_
#define _INJECTION_
//#define _DIRECT_DETOURING_
