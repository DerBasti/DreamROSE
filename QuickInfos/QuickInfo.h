#pragma once

#include "QuickInfoFuncDefs.h"
#include "Trackable.hpp"
#include "..\ConfigReader\Config.h"
#include "..\CMyFile\MyFile.h"
#include "..\Exceptions\TrackableException.h"
#include <functional>
#include <xutility>

namespace QuickInfo {

	struct QUICKPROCESSINFOS {
		std::wstring path;
		std::wstring handleName;
		DWORD threadId;
		std::vector<std::wstring> dlls;
	};

	enum EOperatingSystems {
		OS_Windows_31 = 0,
		OS_Windows_95,
		OS_Windows_98,
		OS_Windows_ME,
		OS_Windows_NT,
		OS_Windows_2000,
		OS_Windows_XP,
		OS_Windows_Vista,
		OS_Windows_7
	};

	enum DirectXFunctionOffsets {
		D3D9_QueryInterface,
		D3D9_AddRef,
		D3D9_Release,

		/*** IDirect3DDevice9 methods ***/
		D3D9_TestCooperativeLevel,
		D3D9_GetAvailableTextureMem,
		D3D9_EvictManagedResources,
		D3D9_GetDirect3D,
		D3D9_GetDeviceCaps,
		D3D9_GetDisplayMode,
		D3D9_GetCreationParameters,
		D3D9_SetCursorProperties,
		D3D9_SetCursorPosition,
		D3D9_ShowCursor,
		D3D9_CreateAdditionalSwapChain,
		D3D9_GetSwapChain,
		D3D9_GetNumberOfSwapChains,
		D3D9_Reset,
		D3D9_Present,
		D3D9_GetBackBuffer,
		D3D9_GetRasterStatus,
		D3D9_SetDialogBoxMode,
		D3D9_SetGammaRamp,
		D3D9_GetGammaRamp,
		D3D9_CreateTexture,
		D3D9_CreateVolumeTexture,
		D3D9_CreateCubeTexture,
		D3D9_CreateVertexBuffer,
		D3D9_CreateIndexBuffer,
		D3D9_CreateRenderTarget,
		D3D9_CreateDepthStencilSurface,
		D3D9_UpdateSurface,
		D3D9_UpdateTexture,
		D3D9_GetRenderTargetData,
		D3D9_GetFrontBufferData,
		D3D9_StretchRect,
		D3D9_ColorFill,
		D3D9_CreateOffscreenPlainSurface,
		D3D9_SetRenderTarget,
		D3D9_GetRenderTarget,
		D3D9_SetDepthStencilSurface,
		D3D9_GetDepthStencilSurface,
		D3D9_BeginScene,
		D3D9_EndScene,
		D3D9_Clear,
		D3D9_SetTransform,
		D3D9_GetTransform,
		D3D9_MultiplyTransform,
		D3D9_SetViewport,
		D3D9_GetViewport,
		D3D9_SetMaterial,
		D3D9_GetMaterial,
		D3D9_SetLight,
		D3D9_GetLight,
		D3D9_LightEnable,
		D3D9_GetLightEnable,
		D3D9_SetClipPlane,
		D3D9_GetClipPlane,
		D3D9_SetRenderState,
		D3D9_GetRenderState,
		D3D9_CreateStateBlock,
		D3D9_BeginStateBlock,
		D3D9_EndStateBlock,
		D3D9_SetClipStatus,
		D3D9_GetClipStatus,
		D3D9_GetTexture,
		D3D9_SetTexture,
		D3D9_GetTextureStageState,
		D3D9_SetTextureStageState,
		D3D9_GetSamplerState,
		D3D9_SetSamplerState,
		D3D9_ValidateDevice,
		D3D9_SetPaletteEntries,
		D3D9_GetPaletteEntries,
		D3D9_SetCurrentTexturePalette,
		D3D9_GetCurrentTexturePalette,
		D3D9_SetScissorRect,
		D3D9_GetScissorRect,
		D3D9_SetSoftwareVertexProcessing,
		D3D9_GetSoftwareVertexProcessing,
		D3D9_SetNPatchMode,
		D3D9_GetNPatchMode,
		D3D9_DrawPrimitive,
		D3D9_DrawIndexedPrimitive,
		D3D9_DrawPrimitiveUP,
		D3D9_DrawIndexedPrimitiveUP,
		D3D9_ProcessVertices,
		D3D9_CreateVertexDeclaration,
		D3D9_SetVertexDeclaration,
		D3D9_GetVertexDeclaration,
		D3D9_SetFVF,
		D3D9_GetFVF,
		D3D9_CreateVertexShader,
		D3D9_SetVertexShader,
		D3D9_GetVertexShader,
		D3D9_SetVertexShaderConstantF,
		D3D9_GetVertexShaderConstantF,
		D3D9_SetVertexShaderConstantI,
		D3D9_GetVertexShaderConstantI,
		D3D9_SetVertexShaderConstantB,
		D3D9_GetVertexShaderConstantB,
		D3D9_SetStreamSource,
		D3D9_GetStreamSource,
		D3D9_SetStreamSourceFreq,
		D3D9_GetStreamSourceFreq,
		D3D9_SetIndices,
		D3D9_Indices,
		D3D9_CreatePixelShader,
		D3D9_SetPixelShader,
		D3D9_GetPixelShader,
		D3D9_SetPixelShaderConstantF,
		D3D9_GetPixelShaderConstantF,
		D3D9_SetPixelShaderConstantI,
		D3D9_GetPixelShaderConstantI,
		D3D9_SetPixelShaderConstantB,
		D3D9_GetPixelShaderConstantB,
		D3D9_DrawRectPatch,
		D3D9_DrawTriPatch,
		D3D9_DeletePatch,
		D3D9_CreateQuery,
		D3D9_ENUMEND
	};

	typedef int (__stdcall *procEnumProcessModulesExPtr)(
		HANDLE hProcess,
		HMODULE *lphModule,
		DWORD cb,
		LPDWORD lpcbNeeded,
		DWORD dwFilterFlag
    );
	extern procEnumProcessModulesExPtr procEnumProcessModulesEx;

	void convertImageNameToRealName(std::wstring*);
	BYTE getOperatingSystem();
	bool is64BitSystem();
	bool is64OperatingSystem();
	bool is64BitModule(HANDLE module);
	bool is64BitModule(HMODULE module);
	bool is64BitModule(DWORD threadId);
	bool is64BitModule(const wchar_t *name);

	bool isTarget64Bit(const wchar_t *windowName);
	bool isTarget64Bit(const wchar_t *windowName, const wchar_t *exeName);

	void isWindowExistent(DWORD threadId, HWND *result);
	void isWindowExistent(DWORD threadId, const wchar_t *exeName, HWND *result);
	void isWindowExistent(const wchar_t *windowName, HWND *exHwnd);
	void isWindowExistent(const wchar_t *windowName, const wchar_t *exeName, HWND *exHwnd);
	void isWindowExistent(HANDLE process, HWND *result);
	void isWindowExistent(HMODULE dll, HWND *result);

	int stringCompare(const std::wstring& wString, const std::string& aString);
	void convertStringWA(std::wstring& storage, const std::string& data, bool forceWide=false);
	void convertStringAW(std::string& storage, const std::wstring& data, bool forceShrink = true);
	void replaceTermInString(std::string& textBlock, const char* termToSearchFor, const char* replacementForSearchTerm);
	void removeTermFromString(std::string& textBlock, const char* term);
	void removeTermFromString(std::string& textBlock, const char* termStart, const char* termEnd);

	void intAsWString(const DWORD data, std::wstring& result);
	void intAsWString(const unsigned int data, std::wstring& result);
	void wstringAsFloat(const std::wstring& data, float& result);
	void wstringAsInt(const std::wstring& data, DWORD& result, bool ignoreComma);
	void wstringAsCurrency(const std::wstring& data, DWORD& result);
	void floatAsInt(const float& toConvert, DWORD& result);
	void intAsFloat(const DWORD& toConvert, float& result);
	float fRand(const float max = 100.0f, bool allowNegative=false);

	void getClipboardData(std::wstring& dataStorage);
	void setClipboardData(std::wstring& dataToAssign);
	
	void getPath(std::string *cArray, bool removeHandle = true);

	void getPath(std::wstring *wArray, bool removeHandle = true);
	void getPath(DWORD handle, std::wstring *wArray, bool removeHandle = true);
	void getPath(HWND handle, std::wstring *wArray, bool removeHandle = true);
	void getPath(HMODULE handle, std::wstring *wArray, bool removeHandle = true);
	void getPath(HANDLE handle, std::wstring *wArray, bool removeHandle = true);

	void getHandleName(std::wstring *wArray, bool removePath = true);
	void getHandleName(DWORD handle, std::wstring *wArray, bool removePath = true);
	void getHandleName(HWND handle, std::wstring *wArray, bool removePath = true);
	void getHandleName(HMODULE handle, std::wstring *wArray, bool removePath = true);
	void getHandleName(HANDLE handle, std::wstring *wArray, bool removePath = true);

	DWORD getThreadIdByName(const wchar_t *executableName);
	DWORD getThreadIdByHWND(HWND window);
	DWORD getParentThreadId();
	DWORD getParentThreadId(wchar_t *exeName);

	bool isDllLoaded(HMODULE dll);
	bool isDllLoaded(std::wstring pathToDll);
	bool isDllLoaded(const wchar_t *pathToDll);

	bool ejectDLL(DWORD threadId, std::wstring dllName);
	bool ejectDLL(HANDLE process, std::wstring dllName);

	DWORD getAllRunningProcesses(std::wstring *result);
	DWORD getAllRunningProcesses(QUICKPROCESSINFOS *pResult, DWORD maxEntries = 1);

	void getAllDllsOfProcess(DWORD threadId, std::wstring *result, bool removePath = true);
	void getAllDllsOfProcess(HANDLE process, std::wstring *result, bool removePath = true);

	void getDllExports(std::wstring pathToDll, std::wstring* result);
	void getDllExports(HMODULE hDll, std::wstring* result);

	const char* getErrorName(DWORD reason);
	const char* getErrorDescription(DWORD reason);

#undef max
	template<class _Ty> const _Ty& max(const _Ty& a, const _Ty& b);
#undef min
	template<class _Ty> const _Ty& min(const _Ty& a, const _Ty& b);
	template<class _Ty> __inline _Ty round(double x) {
		return static_cast<_Ty>(::floor(x + 0.5));
	}

	unsigned int* removeHandleFromPath(std::wstring, unsigned int *res);
	unsigned int* removePathFromHandle(std::wstring, unsigned int *res);

	DWORD numlen(DWORD number);
	DWORD fnumlen(double number);

	void getFilesFromDirectory(std::string& filePath, std::vector<std::string>& result, bool parseSubDirectories=true);
	void getFilesFromDirectory(std::string& filePath, std::string& extensionFilter, std::vector<std::string>& result, bool parseSubDirectories=true);

	//http://stackoverflow.com/questions/4064134/arraysize-c-macro-how-does-it-work
	template< class Type, ptrdiff_t n> static ptrdiff_t arraySize(Type(&)[n]) { return n; }

	//FOR DIRECTX
	unsigned long* createVTable(HMODULE d9dll, DWORD *vTable, std::wstring *log = nullptr, DWORD vTableSize = D3D9_ENUMEND, DWORD timeoutInMs = 20000, bool logOffsets = false);
	DWORD getDirectXOffset(unsigned long *pVTable, DWORD functionId);

	class FileDialog {
		public:
			static bool Open(wchar_t *nameBuf, const wchar_t *pureExtensionWithoutDot) {
				OPENFILENAME ofn;
				memset(&ofn,0,sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = nullptr;
			
				ofn.lpstrDefExt = pureExtensionWithoutDot;

				std::wstring wResult = pureExtensionWithoutDot;
				wchar_t zero[1] = {'\0'};
				ofn.lpstrFilter = wResult.append(zero).append(L".").append(pureExtensionWithoutDot).append(zero).append(zero).c_str();
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFile = nameBuf;
				ofn.lpstrTitle = L"Choose a file";
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

				return GetOpenFileName(&ofn) > 0;
			}

			static bool Save(wchar_t *saveTo, const wchar_t *pureExtensionWithoutDot) {
				OPENFILENAME ofn;
				memset(&ofn,0,sizeof(OPENFILENAME));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = nullptr;
				ofn.lpstrDefExt = pureExtensionWithoutDot;

				std::wstring wResult = pureExtensionWithoutDot;
				wResult = wResult.append(0x00).append(L".").append(pureExtensionWithoutDot).append(0x00).append(0x00);
				ofn.lpstrFilter = wResult.c_str();
				ofn.nMaxFile = MAX_PATH;
				//wchar_t nameBuf[MAX_PATH] = { 0x00 };
				ofn.lpstrFile = saveTo;
				ofn.lpstrTitle = L"Choose a saving path";
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
		
				return GetSaveFileName(&ofn) > 0;
			}
	};

#ifndef AddWindowStyle
#define AddWindowStyle(hwnd, WS_NEWSTYLE) SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) | (WS_NEWSTYLE))
#endif

#ifndef RemoveWindowStyle
#define RemoveWindowStyle(hwnd, WS_STYLE) SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~(WS_STYLE))
#endif

#ifndef AddWindowExStyle
#define AddWindowExStyle(hwnd, WS_EX_NEWSTYLE) SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | (WS_EX_NEWSTYLE))
#endif

#ifndef RemoveWindowExStyle
#define RemoveWindowExStyle(hwnd, WS_EX_STYLE) SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~(WS_EX_STYLE))
#endif

}

#ifdef __DEBUG__
CMyFile logFile;
#endif

#if _SORTEDLIST_WITHOUT_ALLOCATOR_
template<class _keyarable, class _ValueType> class SortedList {
public:
	static const DWORD _NewBuyMinimum = 3;
	static const DWORD _NewMarkPercentage = 90;

	struct ValueKeyPair {
		_keyarable compareVal;
		_ValueType val;
		ValueKeyPair(const _keyarable& comp, const _ValueType& value) : val(value), compareVal(comp) {
		}
	};

	struct ContainerType {
		ValueKeyPair **_Values;
		size_t _UsedSize;
		size_t _MaxSize;

		ContainerType(size_t _valSize) {
			this->_MaxSize = _valSize;
			this->_UsedSize = 0x00;
			this->_Values = new SortedList<_keyarable, _ValueType>::ValueKeyPair*[_valSize];
		}
		ContainerType() {
			this->_MaxSize = this->_UsedSize = 0x00;
			this->_Values = nullptr;
		}
		void add(const class SortedList<_keyarable, _ValueType>::iterator& it) {
			if (this->_UsedSize < this->_MaxSize) {
				this->_Values[this->_UsedSize] = (*it);
				this->_UsedSize++;
			}
		}
	};

	class iterator {
		private:
			ContainerType _containerPtr;
			size_t _containerIdx;
			friend class SortedList<_keyarable, _ValueType>;
			iterator(ContainerType &_newContainerPtr, size_t containerIdx) : _containerPtr(_newContainerPtr) {
				if (this->_containerPtr._Values == nullptr) {
					this->_containerIdx = 0x00;
				}
				else {
					this->_containerIdx = (containerIdx >= _newContainerPtr._UsedSize ? _newContainerPtr._UsedSize: containerIdx);
				}
			}
		public:
			iterator() {
				this->_containerIdx = 0x00;
			}
			iterator(const iterator& rhs) {
				this->_containerPtr = rhs._containerPtr;
				this->_containerIdx = rhs._containerIdx;
			}

			iterator& operator=(const iterator& rhs) {
				this->_containerPtr = rhs._containerPtr;
				return (*this);
			}

			iterator& operator++() {
				this->_containerIdx++;
				return (*this);
			}
			iterator operator++(int) {
				iterator tmp(*this);
				this->operator++();
				return tmp;
			}

			iterator& operator--() {
				this->_containerIdx--;
				return (*this);
			}
			iterator operator--(int) {
				iterator tmp(*this);
				this->operator--();
				return tmp;
			}

			bool operator==(const iterator& rhs) {
				if (this->_containerIdx == rhs._containerIdx && this->_containerPtr._Values == rhs._containerPtr._Values)
					return true;
				return false;
			}
			bool operator!=(const iterator& rhs) {
				return !(this->operator==(rhs));
			}

			ValueKeyPair* operator*() {
				if (this->_containerIdx >= this->_containerPtr._UsedSize)
					return nullptr;
				return this->_containerPtr._Values[this->_containerIdx];
			}

			ValueKeyPair* operator->() {
				if (this->_containerIdx >= this->_containerPtr._UsedSize)
					return nullptr;
				return this->_containerPtr._Values[this->_containerIdx];
			}
	};	
	__inline iterator begin() {
		return iterator(this->_container, 0x00);
	}
	__inline iterator end() {
		return iterator(this->_container, MAXDWORD);
	}

private:

	ContainerType _container;

	struct _NewBuyInformationStruct {
		size_t _Mark;
		size_t _NewTotalSize;
	} _NewBuyInformation;


	//Used algorithm: Divide and conquer (more divide, less conquer)
	WORD divideList(const _keyarable &toInsert, WORD lowerLimit, WORD upperLimit) {
		WORD pivot = (upperLimit + lowerLimit) / 2;
		SortedList<_keyarable, _ValueType>::ValueKeyPair* smallestSample = this->_container._Values[lowerLimit];
		SortedList<_keyarable, _ValueType>::ValueKeyPair* pivotEntry = this->_container._Values[pivot];
		SortedList<_keyarable, _ValueType>::ValueKeyPair* biggestSample = this->_container._Values[upperLimit];

		if (toInsert <= smallestSample->compareVal) {
#ifdef __DEBUG__
			::logFile.putStringWithVar(L"Insert(%i) <= smallestSample(%i) -- inserting at %i of %i\n", toInsert->compareVal, smallestSample->compareVal, lowerLimit, this->list.size());
#endif
			return lowerLimit;
		}
		else if (toInsert >= biggestSample->compareVal) {
#ifdef __DEBUG__
			::logFile.putStringWithVar(L"Insert(%i) >= biggestSample(%i) -- inserting at %i of %i\n", toInsert->compareVal, biggestSample->compareVal, (upperLimit + 1), this->list.size());
#endif
			return (upperLimit + 1);
		}
		if (((upperLimit - lowerLimit) / 2) == 0) {
#ifdef __DEBUG__
			::logFile.putStringWithVar(L"avg(upperLimit,lowerLimit)(%i) <= 1 -- inserting at %i of %i\n", ((upperLimit - lowerLimit) / 2), lowerLimit, this->list.size() );
#endif
			return (toInsert == smallestSample->compareVal ? lowerLimit : lowerLimit + 1);
		}

		//If the sample is in the smaller bracket
		if (toInsert >= smallestSample->compareVal && toInsert <= pivotEntry->compareVal) {
#ifdef __DEBUG__
			::logFile.putStringWithVar(L"toInsert(%i) >= smallestSample(%i) && toInsert(%i) <= pivotEntry(%i)\n", toInsert->compareVal, smallestSample->compareVal, toInsert->compareVal, pivotEntry->compareVal);
#endif
			return divideList(toInsert, lowerLimit, pivot);
		}
		if (toInsert >= pivotEntry->compareVal && toInsert <= biggestSample->compareVal) {
#ifdef __DEBUG__
			::logFile.putStringWithVar(L"toInsert(%i) >= pivotEntry(%i) && toInsert(%i) <= biggestSample(%i)\n", toInsert->compareVal, pivotEntry->compareVal, toInsert->compareVal, biggestSample->compareVal);
#endif
			return divideList(toInsert, pivot, upperLimit);
		}
#ifdef __DEBUG__
		::logFile.putStringWithVar(L"No criteria fits value %i", toInsert->compareVal);
#endif

		//Neither of the criteria above is fitting (just in case?)
		return this->_container._UsedSize;
	}

	__inline size_t _calculateMark() {
		return static_cast<size_t>(floor(this->_NewBuyInformation._NewTotalSize * SortedList<_keyarable, _ValueType>::_NewMarkPercentage / 100.0f));
	}

	void _buyAdditionalSpace() {
		if (!_checkBuyMark())
			return;
		ValueKeyPair **_newContainer = new ValueKeyPair*[this->_NewBuyInformation._NewTotalSize];
		if (!_newContainer)
			throw TraceableException("Additional Space container was not allocatable!");

		SortedList< _keyarable, _ValueType>::iterator it;
		unsigned int i = 0;
		for (it = this->begin(); it != this->end(); it++, i++) {
			_newContainer[i] = *it;
		}

		ValueKeyPair **_tmpPtr = this->_container._Values;
		this->_container._Values = _newContainer;

		for (it = this->begin(); it != this->end(); it++) {
			_tmpPtr[i] = nullptr;
		}
		delete[] _tmpPtr;
		_tmpPtr = nullptr;

		this->_NewBuyInformation._NewTotalSize += min(SortedList::_NewBuyMinimum, static_cast<size_t>(ceil(this->_NewBuyInformation._NewTotalSize * 0.1f + this->_NewBuyInformation._NewTotalSize)) );
		this->_NewBuyInformation._Mark = _calculateMark();
		this->_container._MaxSize = this->_NewBuyInformation._NewTotalSize;
	}
	bool _checkBuyMark() {
		if ((this->_container._UsedSize + 1) <= this->_NewBuyInformation._Mark)
			return false;
		return true;
	}

	bool insert(size_t position, ValueKeyPair* val) {
		this->_buyAdditionalSpace();
		for (int i = this->_container._UsedSize; i > position && i>0; i--) {
			this->_container._Values[i] = this->_container._Values[i-1];
		}
		this->_container._Values[position] = val;
		this->_container._UsedSize++;
		return true;
	}
public:
	SortedList() {
		this->_NewBuyInformation._NewTotalSize = SortedList::_NewBuyMinimum;
		this->_NewBuyInformation._Mark = 0x00;
		this->_buyAdditionalSpace();
	}
	~SortedList() {
		this->clear();
	}

	__inline size_t size() const {
		return this->_container._UsedSize;
	}

	__inline bool isEmpty() const {
		return (this->size() == 0);
	}

	void clear() {
		SortedList<_keyarable, _ValueType>::iterator it;
		for (it = this->begin(); it != this->end(); it++) {
			delete (*it);
		}
		delete[] this->_container._Values;
		this->_container = nullptr;
	}

	bool operator==(const SortedList<_keyarable, _ValueType>& otherList) {
		if (this->size() != otherList.size())
			return false;
		SortedList<_keyarable, _ValueType>::iterator itThis;
		SortedList<_keyarable, _ValueType>::iterator itOther;
		for (itThis = this->begin(), itOther = otherList.begin(); itThis != this->end() && itOther != otherList.end(); itThis++, itOther++) {
			if (*itThis == *itOther)
				return true;
		}
		return true;
	}

	SortedList<_keyarable, _ValueType>& operator=(const SortedList<_keyarable, _ValueType>& listToCpy) {
		if ((*this) == listToCpy)
			return (*this);
		this->clear();
		for (it = this->begin(); it != this->end(); it++) {
			this->add(it);
		}
		return (*this);
	}

	//Adds a given Value-Key-Pair into the storage.
	//The "CompVal" needs to have a working comparison operator (<, <=, ==, >, >=, !=)
	//The actual entry can have any pointer form; It should have a working assignment operator (=), though.
	//Inserting a given pair is done via the Divide-And-Conquer approach - total complexity: O(n * log(n))
	bool add(const _keyarable& comp, const _ValueType& entry) {
		SortedList<_keyarable, _ValueType>::ValueKeyPair *newPair = new SortedList<_keyarable, _ValueType>::ValueKeyPair(comp, const_cast<_ValueType&>(entry));
		if (this->list.empty()) {
			this->list.push_back(newPair);
			return true;
		}
		WORD position = divideList(newPair->compareVal, 0, this->list.size() - 1);
#ifdef __DEBUG__
		::logFile.putString("\n\n");
#endif
		if (position == this->list.size()) {
			this->list.push_back(newPair);
		}
		else {
			std::vector<SortedList<_keyarable, _ValueType>::ValueKeyPair*> tmpQueue;
			for (unsigned int i = 0; i < position; i++) {
				tmpQueue.push_back(this->list.at(i));
			}
			tmpQueue.push_back(newPair);
			for (unsigned int j = position; j < this->list.size(); j++) {
				tmpQueue.push_back(this->list.at(j));
			}
			this->list = tmpQueue;
			return true;
		}
		return false;
	}
	//Removes a given entry based on its previously added value (of the Value-Key-Pair)
	//Complexity: O(n)
	bool _CRT_DEPRECATE_TEXT("It only removes the first found _ValueType-value. For a safer route, take remove(_keyVal, _ValueType, bool);") remove(const _ValueType& entryToDelete, bool deleteAllDuplicates) {
		bool hasDeletedAnything = false;
		for (unsigned int i = 0; i < this->list.size(); i++) {
			if (entryToDelete == this->list.at(i)->val) {

				SortedList<_keyarable, _ValueType>::ValueKeyPair* pair = *(this->list.erase(this->list.begin() + i));
				delete pair; pair = nullptr;
				if (!deleteAllDuplicates)
					return true;

				hasDeletedAnything = true;
			}
		}
		return hasDeletedAnything;
	}

	bool remove(const _keyarable& comp, const _ValueType& val, bool deleteAllDuplicates) {
		bool hasDeletedAnything = false;
		for (unsigned int i = 0; i < this->list.size(); i++) {
			if (this->list.at(i)->val == val && this->list.at(i)->compareVal == comp) {
				SortedList<_keyarable, _ValueType>::ValueKeyPair *pair = *(this->list.erase(this->list.begin() + i));
				delete pair; pair = nullptr;
				if (!deleteAllDuplicates)
					return true;
				hasDeletedAnything = true;
			}
		}
		return hasDeletedAnything;
	}

	bool contains(const _ValueType& entryToFind) const {
		for (unsigned int i = 0; i < this->list.size(); i++) {
			if (entryToFind == this->list.at(i)->val) {
				return true;
			}
		}
		return false;
	}

	__inline bool contains(const _keyarable& compValue, const _ValueType& value) {
		return (this->findPair(compValue, value) != MAXWORD);
	}


#ifndef ppFindPosition
#define ppFindPosition(totalBooleanExpression) \
	if (totalBooleanExpression) {\
		return position; \
	} \
	else if (static_cast<size_t>(position + 1) < this->list.size()) {\
		pair = this->list.at(position + 1);\
		if (totalBooleanExpression)\
			return (position + 1);\
	} else if (static_cast<size_t>(position - 1) >= 0x00) { \
		pair = this->list.at(position - 1); \
		if (totalBooleanExpression)\
			return (position - 1); \
	}
#endif

		size_t findKey(const _keyarable& compValue) {
			if (this->list.empty())
				return MAXWORD;
			WORD position = divideList(compValue, 0, this->list.size() - 1);
			ValueKeyPair* pair = this->list.at(position);
			ppFindPosition(pair->compareVal == compValue);
			return MAXWORD;
		}

		size_t findValue(const _ValueType& value) {
			for (unsigned int i = 0; i < this->size(); i++) {
				if (value == this->list.at(i)->val)
					return i;
			}
			return MAXWORD;
		}

		size_t findPair(const _keyarable& compValue, const _ValueType& value) {
			if (this->list.empty())
				return MAXWORD;
			ValueKeyPair tmpPair(compValue, value);
			WORD position = divideList(compValue, 0, this->list.size() - 1);
			ValueKeyPair* pair = this->list.at( (position >= this->list.size() ? position - 1 : position) );
			ppFindPosition(pair->compareVal == compValue && pair->val == value);
			return MAXWORD;
		}

		__inline size_t size() const {
			return this->list.size();
		}

		__inline bool isEmpty() const {
			return this->list.empty();
		}

		void clear() {
			while (!this->list.empty()) {
				ValueKeyPair *pair = *(this->list.erase(this->list.begin()));
				delete pair; pair = nullptr;
			}
			this->list.clear();
		}

		//Throws CTraceableException in case the wanted position is out of range
		_keyarable& getKeyByPosition(size_t& position) const {
			if (position >= this->list.size())
				throw TraceableExceptionARGS("Given position(%i) is bigger than the SortedListSize(%i)", position, this->list.size());
			return this->list.at(position)->compareVal;
		}

		//Throws CTraceableException in case the wanted value could not be found
		_ValueType& getValueByKey(const _keyarable& comp) const {
			WORD position = this->findKey(comp);
			if (position < this->list.size()) {
				return this->list.at(position);
			}
			throw TraceableExceptionARGS("Given position(%i) is bigger than the SortedListSize(%i)", position, this->list.size());
		}

		//Throws CTraceableException in case the wanted position is out of range
		_ValueType& getValueByPosition(const size_t& position) const {
			if (position >= this->list.size())
				throw TraceableExceptionARGS("Given position(%i) is bigger than the SortedListSize(%i)", position, this->list.size());
			return this->list.at(position)->val;
		}

		//Throws CTraceableException in case the wanted position is out of range
		ValueKeyPair*& at(const size_t& position) {
			if (position >= this->list.size())
				throw TraceableExceptionARGS("Given position(%i) is bigger than the SortedListSize(%i)", position, this->list.size());
			return this->list.at(position);
		}
	*/
};

template<class _keyarable, class _ValueType> bool operator==(SortedList<_keyarable, _ValueType>& lhs, SortedList<_keyarable, _ValueType>& rhs) {
	if (reinterpret_cast<void*>(&lhs) == reinterpret_cast<void*>(&rhs))
		return true;
	if (lhs.size() != rhs.size())
		return false;
	for (unsigned int i = 0; i < lhs.size(); i++) {
		std::auto_ptr<SortedList<_keyarable, _ValueType>::ValueKeyPair> lhsPair(new ValueKeyPair(lhs.getKeyByPosition(i), lhs.getValueByPosition(i)));
		std::auto_ptr<SortedList<_keyarable, _ValueType>::ValueKeyPair> rhsPair(new ValueKeyPair(rhs.getKeyByPosition(i), rhs.getValueByPosition(i)));

		if (lhsPair->compareVal != rhsPair->compareVal ||
			lhsPair->val != rhsPair->val) {
			return false;
		}
	}
	return true;
}
#else //SortedList with Allocator

/*
template<class _T, class _U>
struct allocator_switch : _U { };

template<class _T>
struct allocator_switch<_T, _T> : _T { };
*/

template < class _KeyType, class _ValType, typename _KeyAlloc = std::allocator<_KeyType>, typename _ValAlloc = std::allocator<_ValType> > 
class SortedList : private _KeyAlloc, _ValAlloc {
	private:
		_KeyAlloc _keyAllocator;
		_ValAlloc _valAllocator;

		template<class _Alloc, class _Ty1, class _Ty2> void constructValue(_Alloc& _alloc, _Ty1* _pDest, _Ty2&& _Src) {
			_alloc.construct(_pDest, std::forward<_Ty2>(_Src));
		}
	public:
#ifndef typeDefinitions
#define typeDefinitions(valName, allocName) \
	typedef allocName valName##_type;\
	typedef typename allocName##::reference valName##_reference;\
	typedef typename allocName##::const_reference valName##_const_reference;\
	\
	typedef typename allocName##::size_type valName##_size_type;\
	typedef typename allocName##::difference_type valName;\
	\
	typedef typename allocName##::pointer valName##_pointer;\
	typedef typename allocName##::const_pointer valName##_const_pointer;
#endif
		
	typeDefinitions(key, _KeyAlloc);
	typeDefinitions(value, _ValAlloc);
	
	size_t _usedSize;
	size_t _capacity;
	
	template<class _Type> class _iteratorInfo {
		public:
			_Type _firstItem;
			_Type _lastItem;
			_Type _endOfContainer;
			_iteratorInfo() {
				this->_firstItem = _Type();
				this->_lastItem = _Type();
				this->_endOfContainer = _Type();
			}
	};

	_iteratorInfo<key_pointer> compareInfo;
	_iteratorInfo<value_pointer> valueInfo;

#ifdef _CUSTOM_ITERATOR_
	typedef key_pointer key_iterator;
	typedef const value_pointer const_key_iterator;
		
	typedef value_pointer value_iterator;
	typedef const value_pointer const_value_iterator;

#pragma region CompareValue Functions
		key_iterator beginKey() {
			return this->compareInfo._firstItem;
		}
		key_iterator endKey() {
			return (this->compareInfo._lastItem + 1);
		}
#pragma endregion

#pragma region ValueType Region
		value_iterator beginValue() {
			return this->valueInfo._firstItem;
		}
		value_iterator endValue() {
			return (this->valueInfo._lastItem + 1);
		}
#else //ifdef _CUSTOM_ITERATOR_
#ifdef _CUSTOM_SINGLE_ITERATOR
	template<class _ItTy, class _Ty> class iterator : public std::iterator<std::bidirectional_iterator_tag, _Ty> {
		protected:
			friend class SortedList;
			_ItTy* listPtr;
			DWORD offset;
			iterator(_ItTy* listToAssign, DWORD _offset = 0x00) {
				this->listPtr = listToAssign;
				this->offset = _offset;
			}
			iterator(const iterator* const rhs) {
				this->listPtr = rhs->listPtr;
				this->offset = rhs->offset;
			}
		public:
			iterator() {
				this->listPtr = nullptr;
				this->offset = 0x00;
			}
			iterator(const iterator& rhs) {
				this->listPtr = rhs.listPtr;
				this->offset = rhs.offset;
			}
			~iterator() {
				this->listPtr = nullptr;
				this->offset = 0x00;
			}
			
			iterator& operator++() {
				this->offset++;
				return (*this);
			}
			const iterator operator++(int) {
				iterator tmp(this);
				this->offset++;
				return tmp;
			}

			iterator& operator--() {
				this->offset--;
				return (*this);
			}
			const iterator operator--(int) {
				iterator tmp(this);
				this->offset--;
				return tmp;
			}

			_Ty& operator->() {
				try {
					_Ty& test = (**this);
					return test;
				} catch( CTraceableException& ex ) {
					throw TraceableException(ex);
				}
			}

			_Ty& operator*() {
				if(this->offset >= this->listPtr->size())
					throw TraceableExceptionARGS("Iterator is out of range(%i of %i)", this->offset, this->listPtr->size());
				_Ty& value = this->listPtr->_firstItem[offset];
				return value;
			}

			bool operator==(const iterator& rhs) {
				if(this == (&rhs))
					return true;
				if(this->listPtr == rhs.listPtr) {
					if((this->offset >= this->listPtr->size() &&
						rhs.offset >= rhs.listPtr->size()) || this->offset == rhs.offset)
						return true;
				}
				return false;
			}

			bool operator!=(const iterator& rhs) {
				return !(this->operator==(rhs));
			}
	};

	typedef iterator<_iteratorInfo<key_pointer, key_size_type>, _KeyType> key_iterator;
	typedef iterator<_iteratorInfo<value_pointer, value_size_type>, _ValType> value_iterator;

	value_iterator beginValue() {
		return value_iterator(&this->valueInfo);
	}
	value_iterator endValue() {
		return value_iterator(&this->valueInfo, this->size());
	}

	key_iterator beginKey() {
		return key_iterator(&this->compareInfo);
	}
	key_iterator endKey() {
		return key_iterator(&this->compareInfo, this->size());
	}
	bool containsKey(const _KeyType compVal) {
		key_iterator it = this->cbegin();
		for(;it != this->cend();it++) {
			if(it == compVal)
				return true;
		}
		return false;
	}

	bool containsValue(const _ValType dataVal) {
		value_iterator it = this->vbegin();
		for(;it != this->vend();it++) {
			if(it == dataVal)
				return true;
		}
		return false;
	}
	
		bool removeByKey(const _KeyType compVal) {
			throw TraceableException("Method not implemented yet!");
		}
		bool removeByValue(const _ValType dataVal) {
			throw TraceableException("Method not implemented yet!");
		}
		
		bool remove(const _KeyType compVal, const _ValType dataVal) {
			throw TraceableException("Method not implemented yet!");
		}

		key_size_type findKeyPosition(const _KeyType key) {
			key_iterator it = this->cbegin();
			for(key_size_type i = 0;it != this->cend();it++, i++) {
				if(*it == key)
					return i;
			}
			return -1;
		}
		value_size_type findValuePosition(const _ValType data) {
			value_iterator it = this->vbegin();
			for(value_size_type i = 0;it != this->vend();it++, i++) {
				if(*it == data)
					return i;
			}
			return -1;
		}
		value_size_type find(const _KeyType comp, const _ValType data) {
			value_iterator v_it = this->vbegin();
			key_iterator c_it = this->cbegin();
			for(value_size_type i = 0;c_it != this->cend() && v_it != this->vend(); ;c_it++, v_it++, i++) {
				if(*v_it == data && *c_it == comp)
					return i;
			}
			return -1;
		}

#else //_CUSTOM_SINGLE_ITERATOR_
	template<class _KeyTy, class _ValTy> class _iterator : public std::iterator<std::bidirectional_iterator_tag, std::pair<_KeyTy, _ValTy> > {
		private:
			friend SortedList<_KeyTy, _ValTy>;
			SortedList<_KeyTy, _ValTy> *pList;
			size_t offset;
			_iterator(SortedList* const newList, size_t newOffset = 0x00) {
				this->pList = newList;
				this->offset = newOffset;
			}
		public:
			_iterator() {
				this->pList = nullptr; this->offset = 0x00;
			}
			_iterator(const _iterator& rhs) {
				this->pList = rhs.pList;
				this->offset = rhs.offset;
			}
			~_iterator() {
				this->pList = nullptr; this->offset = 0x00;
			}
			_iterator& operator=(const _iterator& rhs) {
				this->pList = rhs.pList;
				this->offset = rhs.offset;
				return (*this);
			}
			_iterator& operator++() {
				this->offset++;
				return (*this);
			}
			const _iterator operator++(int) {
				_iterator tmp(*this);
				this->offset++;
				return tmp;
			}
			_iterator& operator--() {
				this->offset--;
				return (*this);
			}
			const _iterator operator--(int) {
				iterator tmp(*this);
				this->offset--;
				return tmp;
			}
			bool operator==(const _iterator& rhs) {
				if(this == (&rhs))
					return true;
				if(this->pList == rhs.pList) {
					if(this->offset >= this->pList->size() &&
						rhs.offset >= this->pList->size())
						return true;
				}
				return false;
			}

			bool operator!=(const _iterator& rhs) {
				return !(this->operator==(rhs));
			}

			std::pair<_KeyTy, _ValTy> operator->() {
				return (**this);
			}

			std::pair<_KeyTy, _ValTy> operator*() {
				if(this->pList != nullptr && this->offset < this->pList->size()) {
					return std::pair<_KeyTy, _ValTy>(this->pList->getKey(this->offset), this->pList->getValue(this->offset));
				}
				throw TraceableException("Couldn't create an empty std::pair!");
			}
	};

	typedef _iterator<_KeyType, _ValType> iterator;

	iterator begin() {
		return iterator(this);
	}
	iterator end() {
		return iterator(this, this->size());
	}

	bool containsKey(const _KeyType& key) {
		iterator it = this->begin();
		for(;it != this->end(); it++) {
			if((*it).first == key)
				return true;
		}
		return false;
	}
	bool containsValue(const _ValType& value) {
		iterator it = this->begin();
		for(;it != this->end(); it++) {
			if((*it).second == value)
				return true;
		}
		return false;
	}
	size_t find(const _KeyType& key, const _ValType& value) {
		iterator it = this->begin();
		for(size_t pos = 0x00; it != this->end(); it++, pos++) {
			if((*it).first == key && (*it).second == value)
				return pos;
		}
		return static_cast<size_t>(-1);
	}
	key_size_type findKeyPosition(const _KeyType& key) {
		key_size_type position = 0x00;
		for(; position != this->size(); position++) {
			if(*(this->compareInfo._firstItem + position) == key)
				return position;
		}
		return static_cast<key_size_type>(-1);
	}

	value_size_type findValuePosition(const _ValType& value) {
		value_size_type position = 0x00;
		for(; position != this->size(); position++) {
			if(*(this->valueInfo._firstItem + position) == value)
				return position;
		}
		return static_cast<value_size_type>(-1);
	}
#endif //_CUSTOM_SINGLE_ITERATOR_

#endif //_CUSTOM_ITERATOR_
#pragma endregion

		value_size_type capacity() const {
			return this->_capacity;
		}
		value_size_type size() const {
			return this->_usedSize;
		}

		bool empty() const {
			return (this->size() == 0);
		}

#ifndef TypedClear
#define TypedClear(structName, ptrName) this->structName##._firstItem = ptrName##();\
	this->structName##._lastItem = ptrName##();\
	this->structName##._endOfContainer = ptrName##();
#endif

		SortedList() {
			TypedClear(valueInfo, value_pointer);
			TypedClear(compareInfo, key_pointer);

			this->_usedSize = 0x00;
			this->_capacity = 0x00;
		}

		SortedList(const SortedList& rhs) {
			this->clear();
			
			key_pointer key_newContainer = this->_keyAllocator.allocate(rhs.capacity());
			value_pointer value_newContainer = this->_valAllocator.allocate(rhs.capacity());

#define easierAlloc(structName, containerName) this->structName##._firstItem = containerName;\
			this->structName##._lastItem = this->structName##._firstItem + rhs.size();\
			this->structName##._endOfContainer = this->structName##._firstItem + rhs.capacity();
			
			easierAlloc(compareInfo, key_newContainer);
			easierAlloc(valueInfo, value_newContainer);

			for (unsigned int i = 0; i < rhs.size(); i++) {
				this->_keyAllocator.construct(&this->compareInfo._firstItem[i], *(rhs.compareInfo._firstItem + i));
				this->_valAllocator.construct(&this->valueInfo._firstItem[i], *(rhs.valueInfo._firstItem + i));
			}
		}

		SortedList& operator=(const SortedList<_KeyType, _ValType>& rhs) {
			this->clear();
			
			key_pointer key_newContainer = this->_keyAllocator.allocate(rhs.capacity());
			value_pointer value_newContainer = this->_valAllocator.allocate(rhs.capacity());

			easierAlloc(compareInfo, key_newContainer);
			easierAlloc(valueInfo, value_newContainer);

			for (unsigned int i = 0; i < rhs.size(); i++) {
				this->_keyAllocator.construct(&this->compareInfo._firstItem[i], *(rhs.compareInfo._firstItem + i));
				this->_valAllocator.construct(&this->valueInfo._firstItem[i], *(rhs.valueInfo._firstItem + i));
			}
			this->_usedSize = rhs.size();
			this->_capacity = rhs.capacity();
			return (*this);
		}

		~SortedList() {
			this->clear();
		}

		void reserve(const size_t newSize) {
			if (this->valueInfo._firstItem == value_pointer() && 
				this->compareInfo._firstItem == key_pointer()) {
					this->compareInfo._firstItem = this->_keyAllocator.allocate(newSize);
					this->valueInfo._firstItem = this->_valAllocator.allocate(newSize);
					
					this->compareInfo._lastItem = this->compareInfo._firstItem;
					this->valueInfo._lastItem = this->valueInfo._firstItem;

					this->compareInfo._endOfContainer = this->compareInfo._firstItem + newSize;
					this->valueInfo._endOfContainer = this->valueInfo._firstItem + newSize;

					this->_usedSize = 0x00;
					this->_capacity = newSize;
			}
		}

		void clear() {
			if (this->valueInfo._firstItem != value_pointer() && 
				this->compareInfo._firstItem != key_pointer() && this->capacity()>0) {
				
				std::_Destroy_range(this->valueInfo._firstItem, this->valueInfo._lastItem, this->_valAllocator);
				this->_valAllocator.deallocate(this->valueInfo._firstItem, this->capacity());
				
				std::_Destroy_range(this->compareInfo._firstItem, this->compareInfo._lastItem, this->_keyAllocator);
				this->_keyAllocator.deallocate(this->compareInfo._firstItem, this->capacity());

				this->_usedSize = 0x00;
				this->_capacity = 0x00;

				//EMPTY "pointer"
				TypedClear(valueInfo, value_pointer);
				TypedClear(compareInfo, key_pointer);
			}
		}
		void add(const _KeyType& newCompareValue, const _ValType& data) {
			_KeyType lastKey(newCompareValue);
			_ValType lastVal(data);

			//First item == nullptr or capacity was reached.
			if (this->valueInfo._firstItem == value_pointer() ||
				this->compareInfo._firstItem == key_pointer() || 
				this->size() == this->capacity()) {
				
				value_size_type newSize = this->capacity() + 1;
				
 				key_pointer key_newContainer = this->_keyAllocator.allocate(newSize);
				value_pointer value_newContainer = this->_valAllocator.allocate(newSize);

				for (unsigned int i = 0; i < this->size(); i++) {
					_KeyType currentKey(*(this->compareInfo._firstItem + i));
					_ValType currentVal(*(this->valueInfo._firstItem + i));
					if (lastKey < currentKey) {
						_KeyType tmpKey(currentKey);
						currentKey = lastKey;
						lastKey = tmpKey;

						_ValType tmpVal(currentVal);
						currentVal = lastVal;
						lastVal = tmpVal;
					}
					this->constructValue(this->_keyAllocator, &key_newContainer[i], currentKey);
					this->constructValue(this->_valAllocator, &value_newContainer[i], currentVal);
				}
				this->constructValue(this->_keyAllocator, &key_newContainer[newSize - 1], lastKey);
				this->constructValue(this->_valAllocator, &value_newContainer[newSize - 1], lastVal);
				
				this->clear();
				
				this->valueInfo._firstItem = value_newContainer;
				this->valueInfo._lastItem = this->valueInfo._firstItem + (newSize-1);
				this->valueInfo._endOfContainer = this->valueInfo._firstItem + newSize;
				
				this->compareInfo._firstItem = key_newContainer;
				this->compareInfo._lastItem = this->compareInfo._firstItem + (newSize-1);
				this->compareInfo._endOfContainer = this->compareInfo._firstItem + newSize;

				this->_capacity = newSize;
				this->_usedSize = newSize;
			} else {
				bool realloc = false;
				if(newCompareValue < *this->compareInfo._lastItem) {
					for (unsigned int i = 0; i < this->size(); i++) {
						_KeyType currentKey(*(this->compareInfo._firstItem + i));
						_ValType currentVal(*(this->valueInfo._firstItem + i));
						if (lastKey < currentKey) {
							_KeyType tmpKey(currentKey);
							currentKey = lastKey;
							lastKey = tmpKey;

							_ValType tmpVal(currentVal);
							currentVal = lastVal;
							lastVal = tmpVal;
							realloc = true;
						}
						if(realloc) {
							this->constructValue(this->_keyAllocator, &this->compareInfo._firstItem[i], currentKey);
							this->constructValue(this->_valAllocator, &this->valueInfo._firstItem[i], currentVal);
						}
					}
				}
				this->constructValue(this->_keyAllocator, &this->compareInfo._firstItem[this->size()], lastKey);
				this->constructValue(this->_valAllocator, &this->valueInfo._firstItem[this->size()], lastVal);
				
				this->_usedSize++;
				this->compareInfo._lastItem++;
				this->valueInfo._lastItem++;
			}
		}
		bool contains(const _KeyType compVal, const _ValType dataVal) {
			if(containsKey(compVal) && containsValue(dataVal))
				return true;
			return false;
		}

		bool remove(const _KeyType& key, const _ValType& value) {
			iterator it = this->begin();
			for(size_t pos = 0x00; it != this->end(); it++, pos++) {
				if((*it).first == key && (*it).second == value) {
					this->_keyAllocator.destroy((this->compareInfo._firstItem + pos));
					this->_valAllocator.destroy((this->valueInfo._firstItem + pos));
					
					for(;pos < this->size() - 1;pos++) {
						this->_keyAllocator.construct((this->compareInfo._firstItem + pos), *(this->compareInfo._firstItem + pos + 1));
						this->_valAllocator.construct((this->valueInfo._firstItem + pos), *(this->valueInfo._firstItem + pos + 1));
					}
					this->_usedSize--;
					this->compareInfo._lastItem--;
					this->valueInfo._lastItem--;
					return true;
				}
			}
			return false;
		}
		
		bool removeByKey(const _KeyType& key) {
			iterator it = this->begin();
			for(size_t pos = 0x00;it != this->end(); it++, pos++) {
				if((*it).first == key) {
					this->_keyAllocator.destroy((this->compareInfo._firstItem + pos));
					this->_valAllocator.destroy((this->valueInfo._firstItem + pos));
					
					for(;pos < this->size() - 1;pos++) {
						this->_keyAllocator.construct((this->compareInfo._firstItem + pos), *(this->compareInfo._firstItem + pos + 1));
						this->_valAllocator.construct((this->valueInfo._firstItem + pos), *(this->valueInfo._firstItem + pos + 1));
					}
					this->_usedSize--;
					
					this->compareInfo._lastItem--;
					this->valueInfo._lastItem--;
					return true;
				}
			}
			return false;
		}
		bool removeByValue(const _ValType& value) {
			iterator it = this->begin();
			for(size_t pos = 0x00;it != this->end(); it++, pos++) {
				if((*it).second == value) {
					this->_keyAllocator.destroy((this->compareInfo._firstItem + pos));
					this->_valAllocator.destroy((this->valueInfo._firstItem + pos));
					
					for(;pos < this->size() - 1;pos++) {
						this->_keyAllocator.construct((this->compareInfo._firstItem + pos), *(this->compareInfo._firstItem + pos + 1));
						this->_valAllocator.construct((this->valueInfo._firstItem + pos), *(this->valueInfo._firstItem + pos + 1));
					}
					this->_usedSize--;
					
					this->compareInfo._lastItem--;
					this->valueInfo._lastItem--;
					return true;
				}
			}
			return false;
		}
		bool removeAt(size_t pos) {
			if(this->_usedSize <= pos)
				return false;
			this->_keyAllocator.destroy((this->compareInfo._firstItem + pos));
			this->_valAllocator.destroy((this->valueInfo._firstItem + pos));
			for(;pos < this->size() - 1;pos++) {
				this->_keyAllocator.construct((this->compareInfo._firstItem + pos), *(this->compareInfo._firstItem + pos + 1));
				this->_valAllocator.construct((this->valueInfo._firstItem + pos), *(this->valueInfo._firstItem + pos + 1));
			}
			this->_usedSize--;
					
			this->compareInfo._lastItem--;
			this->valueInfo._lastItem--;
			return true;
		}

		key_const_reference getKey(size_t position) {
			if(position >= this->_usedSize) {
				throw TraceableExceptionARGS("Position is out of range(%i of %i) ", position, this->_usedSize);
			}
			return (*(this->compareInfo._firstItem + position));
		}

		value_reference getValue(size_t position) {
			if(position >= this->_usedSize) {
				throw TraceableExceptionARGS("Position is out of range(%i of %i) ", position, this->_usedSize);
			}
			return (*(this->valueInfo._firstItem + position));
		}
		value_reference getValueByKey(key_reference key) {
			for(unsigned int i=0;i<this->compareInfo._usedSize;i++) {
				if(*(this->compareInfo._firstItem + i) == key)
					return this->getValue(i);
			}
			throw TraceableExceptionARGS("Wanted key could not be found!");
		}

#endif //_SORTEDLIST_WITHOUT_ALLOCATOR_
};

template < class _KeyType, class _ValType, class _KeyAlloc = std::allocator<_KeyType>, class _ValAlloc = std::allocator<_ValType> > 
class UniqueSortedList {
	private:
		SortedList<_KeyType, _ValType, _KeyAlloc, _ValAlloc> container;
	public:
		UniqueSortedList() {
			this->container.clear();
		}
		~UniqueSortedList() {
			this->container.clear();
		}
		size_t size() const { return this->container.size(); }
		size_t capacity() const { return this->container.capacity(); }

		bool add(const _KeyType& newCompareValue, const _ValType& data) {
			if(this->container.findKeyPosition(newCompareValue) != static_cast<size_t>(-1))
				return false; //NOT A UNIQUE KEY VALUE
			this->container.add(newCompareValue, data);
			return true;
		}
		bool contains(const _KeyType key, const _ValType val) {
			return this->container.contains(key, val);
		}
		size_t find(const _KeyType key, const _ValType val) {
			return this->container.find(key, val);
		}
		size_t findKey(const _KeyType key) {
			return this->container.findKeyPosition(key);
		}
		size_t findValue(const _ValType value) {
			return this->container.findValuePosition(value);
		}
		bool removeByKey(const _KeyType& key) {
			return this->container.removeByKey(key);
		}
		bool removeByValue(const _ValType& data) {
			return this->container.removeByValue(data);
		}
		bool removeAt(size_t pos) {
			return this->container.removeAt(pos);
		}
		const _KeyType getKey(size_t pos) {
			return this->container.getKey(pos);
		}
		_ValType& getValue(size_t pos) {
			return this->container.getValue(pos);
		}
		_ValType& getValue(const _KeyType& key) {
			return this->container.getValue(key);
		}
		void clear() {
			return this->container.clear();
		}
		bool operator==(const UniqueSortedList<_KeyType, _ValType>& rhs) {
			if((&rhs) == this)
				return true;
			if(rhs.size() != this->size() || rhs.capacity() != this->capacity())
				return false;
			if(std::memcmp(this, &rhs, sizeof(UniqueSortedList<_KeyType, _ValType>)) == 0)
				return true;
			return false;
		}
		bool operator!=(const UniqueSortedList<_KeyType, _ValType>& rhs) {
			return !(this->operator==(rhs));
		}
};

template<class _Ty> class FixedArray {
	private:
		size_t maxSize;
		size_t usedSize;
		_Ty* container;
	public:
		FixedArray() {
			this->maxSize = 0x00;
			this->usedSize = 0x00;

			this->container = nullptr;
		}
		FixedArray(const size_t maxSize) {
			this->maxSize = this->usedSize = 0x00;
			this->reserve(maxSize);
		}
		~FixedArray() {
			delete [] this->container;
			this->container = nullptr;
			this->usedSize = this->maxSize = 0x00;
		}
		__inline _Ty& operator[](const size_t pos) {
			return this->getValue(pos);
		}
		__inline const _Ty& operator[](const size_t pos) const {
			return (this->getValueCONST(pos));
		}

		_Ty* begin() { return this->container; }
		_Ty* end() { return &this->container[this->capacity()]; }

		void reserve(const size_t newMax) {
			if(newMax > 0 && this->maxSize == 0x00) {
				this->maxSize = newMax;
				this->usedSize = 0x00;

				this->container = new _Ty[this->maxSize];
			}
		}
		void addValue(const _Ty& val) {
			if(this->usedSize < this->maxSize) {
				this->container[this->usedSize] = val;
				this->usedSize++;
			}
		}
		const _Ty& getValueCONST(const size_t pos) const {
			if(pos >= this->usedSize)
				throw std::exception();
			return this->container[pos];
		}
		_Ty& getValue(const size_t pos) const {
			if(pos >= this->usedSize)
				throw std::exception();
			return this->container[pos];
		}
		const size_t capacity() const { return this->maxSize; }
		const size_t size() const { return this->usedSize; }
};

template<class _Ty> class FixedArray2D {
	private:
		_Ty** container;
		DWORD cap[2];
	public:
		FixedArray2D() {
			this->cap[0] = this->cap[1] = 0x00;
			this->container = nullptr;
		}
		~FixedArray2D() {
			for(unsigned int i=0;i<this->cap[0];i++)
				delete [] this->container[i];
			delete [] this->container;
			
			this->container = nullptr;
			this->cap[0] = this->cap[1] = 0x00;
		}
		void reserve(const DWORD firstArraySize, const DWORD secondArraySize) {
			this->cap[0] = firstArraySize;
			this->cap[1] = secondArraySize;
			this->container = new _Ty*[this->capacityTopLevel()];
			for(unsigned int i=0;i<this->capacityTopLevel();i++) {
				this->container[i] = new _Ty[this->capacitySubLevel()];
			}
		}
		void addValue(const DWORD first, const DWORD second, const _Ty& value) {
			if(this->capacityTopLevel() < first || this->capacitySubLevel() < second)
				return;
			this->container[first][second] = _Ty(value);
		}

		_Ty& getValue(const DWORD first, const DWORD second) {
			if(this->capacityTopLevel() < first || this->capacitySubLevel() < second)
				throw std::exception();
			return this->container[first][second];
		}

		void removeValue(const DWORD first, const DWORD second) {
			if(this->capacityTopLevel() < first || this->capacitySubLevel() < second)
				return;
			this->container[first][second] = _Ty(0);
		}
		DWORD capacity() const { return this->capacityTopLevel() * this->capacitySubLevel(); }
		__inline DWORD capacityTopLevel() const { return this->cap[0]; }
		__inline DWORD capacitySubLevel() const { return this->cap[1]; }
};

template<class _Ty> class LinkedHeaderList {
	public:
		class Node {
			private:
				_Ty value;
			public:
				Node* prev;
				Node* next;

				Node() { 
					prev = next = nullptr;
					value = nullptr;
				}
				__inline const _Ty& getValue() const { return this->value; }
		};
	private:
		DWORD headCount;
		Node** nodeHeadList;
	public:
		explicit LinkedHeaderList(const DWORD newHeadCount) {
			this->headCount = newHeadCount;
			this->nodeHeadList = new Node*[this->headCount];
			for(unsigned int i=0;i<this->headCount;i++) {
				this->nodeHeadList[i] = new Node();
				this->nodeHeadList[i]->prev = nullptr;
				this->nodeHeadList[i]->next = nullptr;
			}
		}
		~LinkedHeaderList() {
			for(unsigned int i=0;i<this->headCount;i++) {
				Node* curNode = this->nodeHeadList[i];
				while(curNode) {
					//Get the next node before deleting it
					Node* nextNode = curNode->next;
					
					//Now we can delete the current node
					delete curNode;
					curNode = nullptr;

					//And mark it as next entry
					curNode = nextNode;
				}
			}
		}
		void addEntry(const DWORD headerId, const _Ty& value) {
			Node* curNode = this->nodeHeadList[headerId % this->headCount];
			while(curNode) {
				if(curNode->next == nullptr) {
					//Create a new linked node
					Node* newNode = new LinkedHeaderList::Node<_Ty>();
					newNode->prev = curNode;
					newNode->next = nullptr;

					//Assign the newest value to the last node
					curNode->next = newNode;
					curNode->value = value;
					return;
				}
				curNode = curNode->next;
			}
		}
		void removeEntry(const DWORD headerId, const _Ty& value) {
			DWORD curHeaderId = headerId % this->headCount;
			Node* curNode = this->nodeHeadList[curHeaderId];
			while(curNode) {
				if(curNode->getValue() == value) {
					//in case we have a previous node, 
					//re-wire it.
					if(curNode->prev != nullptr) {
						curNode->prev->next = curNode->next;
					}
					//Same as above, just with the next node
					if(curNode->next != nullptr) {
						curNode->next->prev = curNode->prev;
					}
					//In case we're deleting the head node
					if(curNode == this->getHeadEntry(curHeaderId)) {
						this->nodeHeadList[curHeaderId] = this->nodeHeadList[curHeaderId]->next;
					}
					delete curNode;
					curNode = nullptr;
				}
			}
		}
		__inline Node* getHeadEntry(const DWORD headerId) { 
			return this->nodeHeadList[headerId % this->getHeadCount()];
		}
		__inline Node* getNextEntry( Node* curNode ) {
			return curNode->next;
		}
		__inline const DWORD getHeaderCount() const { return this->headCount; }
};

//TODO:
template<class _Ty, class = typename std::enable_if<std::has_trivial_copy<_Ty>::value && std::has_trivial_assign<_Ty>::value>::type> class LinkedList {	
	public:
		typedef struct Node {
			friend class LinkedList;
			private:
				_Ty value;
				Node* prev;
				Node* next;
			public:
				__inline _Ty getValue() const { return this->value; }
				__inline const _Ty& getValueCONST() const { return this->value; }
				__inline Node* getNextNode() const { return this->next; }
				__inline Node* getPreviousNode() const { return this->prev; }
		};
	private:
		Node* head;
		Node* last;
		size_t count;
	public:
		LinkedList() {
			this->count = 0x00;
			this->head = this->last = nullptr;
		}
		~LinkedList() {

		}
		void add(const _Ty& data) {
			Node* newNode = new Node();
			if(this->count == 0x00) { //empty list
				newNode->prev = nullptr;
				newNode->next = nullptr;
				this->head = this->last = newNode;
			} else {
				newNode->prev = this->last;
				this->last->next = newNode;
				this->last = newNode;
			}
			this->last->value = _Ty(data);
			this->count++;
		}
		
		__inline Node* getHeadNode() const { return this->head; }
		__inline const size_t getNodeCount() const { return this->count; }

		Node* getNode(const size_t pos) {
			size_t offset = 0x00;
			Node* tmpNode = this->head;
			while(tmpNode && offset != pos) {
				tmpNode = tmpNode->next;
				offset++;
			}
			return tmpNode;
		}
		
		_Ty& getValue(const size_t pos) {
			Node* tmpNode = this->getNode(pos);
			if(!tmpNode)
				throw std::exception();
			return tmpNode->getValue();
		}

		void remove(const _Ty& data) {
			Node* tmpNode = this->head;
			size_t idx = 0x00;
			while(tmpNode) {
				if(tmpNode->value == data) {
					return this->removeAt(idx);
				}
				idx++;
				tmpNode = tmpNode->next;
			}
		}
		void removeAt(const size_t pos) {
			if(this->count == 0x00 || pos >= this->count)
				return;

			Node* tmpNode = this->head;
			if(pos == 0x00) {
				if(this->count == 0x01) { //only header
					this->head = this->last = nullptr;
				} else { //other items are available -> create 
					this->head->next->prev = nullptr; 
					this->head = this->head->next;
				}
			} else { //something in between or last
				size_t offset = 0x00;
				while(tmpNode && offset != pos) {
					tmpNode = tmpNode->next;
					offset++;
				}
				if(tmpNode == this->last) {
					this->last = this->last->prev;
					this->last->next = nullptr;
				} else {
					tmpNode->prev->next = tmpNode->next;
					tmpNode->next->prev = tmpNode->prev;
				}
			}
			if(tmpNode) {
				this->count--;
				delete tmpNode;
				tmpNode = nullptr;
			}
		}
		void clear() {
			size_t cnt = this->getNodeCount();
			for(unsigned int i=0;i<cnt;i++)
				this->removeAt(0x00);
		}
};
class Timer {
	private:
		clock_t currentTime;
		SortedList<clock_t, std::function<void()>> list;
		bool isRunning;
		
		DWORD loop() {
			while(this->isRunning) {
				this->currentTime = clock();
				for(unsigned int i=0;i<this->list.size();) {
					clock_t oTime = this->list.getKey(i);
					if(oTime <= this->currentTime)
						this->list.getValue(i)();
					else //other timers are "higher"
						break;
				}
				Sleep(1);
			}
		}

		static DWORD WINAPI threadStart(void *pParam) {
			Timer* thisPtr = reinterpret_cast<Timer*>(pParam);
			return thisPtr->loop();
		}
	public:
		Timer() {
			this->currentTime = clock();
		}
		~Timer() {
			this->isRunning = false;
		}

		void start() {
			this->isRunning = true;
			::CreateThread(nullptr, 0, threadStart, this, 0, nullptr);
		}
		__inline void add(clock_t timeToPassInMilliseconds, std::function<void()> functionToApply) {
			this->currentTime = clock();
			this->list.add( (timeToPassInMilliseconds + this->currentTime), functionToApply );
		}
};