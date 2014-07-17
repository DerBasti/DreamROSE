#pragma once

#ifndef __ROSE_STL__
#define __ROSE_STL__

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#pragma warning(disable:4996)

#include <vector>
#include <string>
#include "D:\Programmieren\QuickInfos\Trackable.hpp"

class STLEntryMULTI {
	private:
		friend class STLFileMULTI;
		friend class STLFile;
		Trackable<char> idName;
		DWORD idNum;
		std::vector<Trackable<char>> languageEntry;
		std::vector<Trackable<char>> languageDescription;
	public:
		STLEntryMULTI(std::string newIdName, const DWORD newIdNum) {
			this->idName.init(newIdName.c_str(), newIdName.length());
			this->idNum = newIdNum;
		}
		__inline void addLanguage(std::string newLanguageEntry, std::string newDesc) {
			Trackable<char> newEntry(newLanguageEntry.c_str(), newLanguageEntry.length());
			this->languageEntry.push_back(newEntry);
			if(newDesc.length()>0) {
				Trackable<char> desc(newDesc.c_str(), newDesc.length());
				this->languageDescription.push_back(desc);
			}
		}
		const Trackable<char> getIDName() const { return this->idName; }
		const DWORD getID() const { return this->idNum; }
		const Trackable<char> getEntry(const size_t language) {
			try {
				return Trackable<char>(this->languageEntry.at(language));
			} catch(std::exception) {
			}
			return Trackable<char>();
		}
		const Trackable<char> getDescription(const size_t language) {
			try {
				return Trackable<char>(this->languageDescription.at(language));
			} catch(std::exception) {
			}
			return Trackable<char>();
		}
};

#ifndef __READ_STRING__
#define __READ_STRING__(len, buf) fread(&len, 1, 1, fh); fread(buf, 1, len, fh); buf[len] = 0x00;
#endif

class STLFileMULTI {
	private:
		friend class STLFile;
		bool descriptionAdded;
		std::vector<STLEntryMULTI> entries;

		void loadInfos(FILE* fh) {
			char buf[0x800] = {0x00};
			BYTE len = 0x00;
			__READ_STRING__(len, buf);
			if(_stricmp(buf, "ITST01")==0)
				this->descriptionAdded = true;

			DWORD dTmp = 0x00;
			fread(&dTmp, 4, 1, fh);
			this->entries.reserve(dTmp);

			DWORD entryCount = this->entries.capacity();
			for(unsigned int i=0;i<entryCount;i++) {
				__READ_STRING__(len, buf);
				fread(&dTmp, 4, 1, fh);
				
				this->entries.push_back(STLEntryMULTI(std::string(buf), dTmp));
			}
			DWORD numOfLanguages = 0x00;
			fread(&numOfLanguages, 4, 1, fh);
			
			std::vector<DWORD> languageOffsets; languageOffsets.reserve(numOfLanguages);
			for(unsigned int i=0;i<numOfLanguages;i++) {
				fread(&dTmp, 4, 1, fh);
				languageOffsets.push_back(dTmp);
			}
			std::vector<DWORD> substrOffsets;
			BYTE bLen=0x00; WORD wLen = 0x00;
			for(unsigned int i=0;i<numOfLanguages;i++) {
				fseek(fh, languageOffsets.at(i), SEEK_SET);

				substrOffsets.clear(); substrOffsets.reserve(entryCount);
				for(unsigned int j=0;j<entryCount;j++) {
					fread(&dTmp, 4, 1, fh);
					substrOffsets.push_back(dTmp);
				}
				for(unsigned int j=0;j<entryCount;j++) {
					fseek(fh, substrOffsets.at(j), SEEK_SET);
					wLen = 0x00;
					fread(&bLen, 1, 1, fh);
					if(bLen>0x7F) {
						fread(&wLen, 1, 1, fh);
						wLen = (wLen<<8) | bLen;
					} else {
						wLen = bLen;
					}
					std::string res = ""; res.reserve(wLen);
					fread(buf, 1, wLen, fh); buf[wLen] = 0x00;
					res = std::string(buf);

					STLEntryMULTI& entry = this->entries.at(j);
					std::string desc = std::string("");
					if(this->descriptionAdded) {
						wLen = 0x00;
						fread(&bLen, 1, 1, fh);
						if(bLen>0x7F) {
							fread(&wLen, 1, 1, fh);
							wLen = (wLen<<8) | bLen;
						} else {
							wLen = bLen;
						}
						fread(buf, 1, wLen, fh); buf[wLen] = 0x00;
						desc = std::string(buf);
					}
					entry.addLanguage(res, desc);
				}
			}
		}
		STLFileMULTI() { }
	public:
		const static BYTE JAPANESE = 0x00;
		const static BYTE ENGLISH = 0x01;
		const static BYTE TAIWANESE = 0x02;
		const static BYTE LANGUAGE_MAX = 0x05;
		STLFileMULTI(const char* pathToFile) {
			this->descriptionAdded = false;
			FILE *fh = fopen(pathToFile, "rb");
			if(fh) {
				this->loadInfos(fh);
				fclose(fh);
			}
		}
		virtual ~STLFileMULTI() {
			this->entries.clear();
		}
		__inline Trackable<char> getEntryName(const DWORD rowId, BYTE language = STLFileMULTI::ENGLISH) {
			language %= STLFileMULTI::LANGUAGE_MAX;
			try {
				return this->entries.at(rowId).getEntry(language);
			} catch(std::exception) {
			}
			return Trackable<char>();
		}
		__inline Trackable<char> getEntryDescription(const DWORD rowId, BYTE language = STLFileMULTI::ENGLISH) {
			language %= STLFileMULTI::LANGUAGE_MAX;
			try {
				return Trackable<char>(this->entries.at(rowId).getDescription(language));
			} catch(std::exception) {
			}
			return Trackable<char>();
		}
		const size_t size() const { return this->entries.size(); }
};


class STLEntry {
	private:
		Trackable<char> idName;
		DWORD idNum;
#ifdef __ROSE_READ_DESCRIPTION__
		typedef std::pair<Trackable<char>,Trackable<char>> trackableEntry;
		std::vector<trackableEntry> entries;
#else //__ROSE_READ_DESCRIPTION__
		std::vector<Trackable<char>> entries;
#endif //__ROSE_READ_DESCRIPTION__
	public:
		STLEntry(const char* newIdName, const DWORD newId) {
			this->idName.init(newIdName, strlen(newIdName));
			this->idNum = newId;
		}
		__inline const char* getIDName() const { return this->idName.getData(); }
		__inline const DWORD getID() const { return this->idNum; }
		const char* getEntry(const size_t pos) {
			try {
#ifdef __ROSE_READ_DESCRIPTION__
				return this->entries.at(pos).first.getData();
#else //__ROSE_READ_DESCRIPTION__
				return this->entries.at(pos).getData();
#endif //__ROSE_READ_DESCRIPTION
			} catch(...) { }
			return "";
		}
#ifdef __ROSE_READ_DESCRIPTION__
		void addLanguage(const char* name, const char* desc) {
			trackableEntry entry(	Trackable<char>(name, strlen(name)),
							Trackable<char>(desc, (desc == nullptr ? 0 : strlen(desc)))
							);
			this->entries.push_back(entry);
		}
		__inline void addLanguage(const Trackable<char> name, const Trackable<char> desc) {
			this->entries.push_back(trackableEntry( name, desc ));
		}
		const char* getDescription(const size_t pos) {
			try {
				return this->entries.at(pos).second.getData();
			} catch(...) { }
			return "";
		}
#else //__ROSE_READ_DESCRIPTION__
	void addLanguage(const char* name) {
		if(!name)
			return;
		this->entries.push_back(Trackable<char>(name, strlen(name)));
	}
	__inline void addLanguage(const Trackable<char>& name) {
		this->entries.push_back(name);
	}
#endif //__ROSE_READ_DESCRIPTION__
};

class STLFile {
	private:
		std::vector<STLEntry> entries;
	public:
		STLFile(const char* filePath) {
			STLFileMULTI multiFile(filePath);
			this->entries.reserve(multiFile.size());
			for(unsigned int i=0;i<multiFile.size();i++) {
				STLEntryMULTI& curEntry = multiFile.entries.at(i);
				STLEntry cpy(curEntry.getIDName().getData(), curEntry.getID());
#ifdef __ROSE_READ_DESCRIPTION__
				cpy.addLanguage(curEntry.getEntry(STLFileMULTI::ENGLISH),
					curEntry.getDescription(STLFileMULTI::ENGLISH));
#else //__ROSE_READ_DESCRIPTION__
				cpy.addLanguage(curEntry.getEntry(STLFileMULTI::ENGLISH));
#endif
				this->entries.push_back(cpy);
			}
		}
		__inline DWORD getEntryId(const size_t pos) {
			return this->entries.at(pos).getID();
		}
		__inline std::string getEntryName(const size_t pos) {
			try {
				return this->entries.at(pos).getEntry(0x00);
			} catch(...) { }
			return "";
		}
		__inline const size_t size() const { return this->entries.size(); }
#ifdef __ROSE_READ_DESCRIPTION__
		__inline std::string getEntryDescription(const size_t pos) {
			try {
				return this->entries.at(pos).getDescription(0x00);
			} catch(...) { }
			return "";
		}
#endif //__ROSE_READ_DESCRIPTION__
};

#endif //__ROSE_STL__