#pragma once

#ifndef __COMMON_DATATYPES__
#define __COMMON_DATATYPES__

#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <xutility>
#include <xmemory>
#include <vector>
#include <mysql.h>

#include "Logger.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "libmysql")

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef unsigned long long QWORD;

#define __ROSE_DEBUG__

class Packet {
public:
	const static DWORD DEFAULT_DATA_MAX = 0x400;
	const static DWORD DEFAULT_HEADER_OFFSET = 0x06;
private:
	WORD length;
	WORD command;
	WORD notUsed;
	char data[Packet::DEFAULT_DATA_MAX];
public:
	Packet() {
		memset(this->data, 0x00, Packet::DEFAULT_DATA_MAX);
		this->length = DEFAULT_HEADER_OFFSET;
	}

	Packet(WORD command, WORD newSize = Packet::DEFAULT_HEADER_OFFSET) {
		this->newPacket(command, newSize);
	}

	operator unsigned char*() { return reinterpret_cast<unsigned char*>(this); }

	void newPacket(WORD command, WORD newSize = Packet::DEFAULT_HEADER_OFFSET) {
		this->command = command;
		this->length = newSize;
		this->notUsed = 0x00;

		memset(this->data, 0x00, Packet::DEFAULT_DATA_MAX);
	}
	void addByte(const BYTE toAdd) {
		this->data[this->length - Packet::DEFAULT_HEADER_OFFSET] = toAdd;
		this->length++;
	}
	BYTE getByte(const WORD position) {
		if (position < this->getLength())
			return this->data[position];
		return static_cast<BYTE>(-1);
	}

	void addWord(const WORD toAdd) {
		*reinterpret_cast<WORD*>(&this->data[this->length - Packet::DEFAULT_HEADER_OFFSET]) = toAdd;
		this->length += sizeof(WORD);
	}
	WORD getWord(const WORD position) {
		if (position < this->getLength())
			return *(reinterpret_cast<WORD*>(&this->data[position]));
		return static_cast<WORD>(-1);
	}

	void addDWord(const DWORD toAdd) {
		*reinterpret_cast<DWORD*>(&this->data[this->length - Packet::DEFAULT_HEADER_OFFSET]) = toAdd;
		this->length += sizeof(DWORD);
	}
	DWORD getDWord(const WORD position) {
		if (position < this->getLength())
			return *(reinterpret_cast<DWORD*>(&this->data[position]));
		return static_cast<DWORD>(-1);
	}

	void addQWord(const QWORD toAdd) {
		*reinterpret_cast<QWORD*>(&this->data[this->length - Packet::DEFAULT_HEADER_OFFSET]) = toAdd;
		this->length += sizeof(QWORD);
	}
	QWORD getQWord(const WORD position) {
		if (position < this->getLength())
			return *(reinterpret_cast<QWORD*>(&this->data[position]));
		return static_cast<QWORD>(-1);
	}

	void addFloat(const float toAdd) {
		*reinterpret_cast<float*>(&this->data[this->length - Packet::DEFAULT_HEADER_OFFSET]) = toAdd;
		this->length += sizeof(float);
	}
	float getFloat(const WORD position) {
		if (position < this->getLength())
			return *(reinterpret_cast<float*>(&this->data[position]));
		return static_cast<float>(-1);
	}

	void addString(const std::string& toAdd) {
		return this->addString(toAdd.c_str());
	}
	void addString(const char* toAdd) {
		if (!toAdd)
			return;
		DWORD len = strlen(toAdd);
		for (unsigned int i = 0; i < len; i++)
			this->addByte(toAdd[i]);
	}

	char* getString(WORD position) { return &this->data[position]; }

	WORD getLength() const { return this->length; }
	WORD getCommand() const { return this->command; }

	const char* getData() const { return this->data; }
};


#ifndef __DEFAULT_DATABASE__
#define __DEFAULT_DATABASE__

#define __DATABASE_HOST__ "localhost"
#define __DATABASE_PORT__ 3306
#define __DATABASE_USERNAME__ "root"
#define __DATABASE_PASSWORD__ "bla123"
#define __DATABASE_DBNAME__ "roseonline"

#endif

class DataBase {
private:
	const char* server;
	const char* userName;
	const char* pw;
	const char* database;
	DWORD port;
	MYSQL* sql;
	MYSQL_RES *result;

	bool isConnected;
	const char* errorDesc;

	void showErrorMsg() {
		this->errorDesc = this->lastError();
		std::cout << "[MYSQL ERROR]: " << this->errorDesc << "\n";
	}
public:
	DataBase() {
		this->server = this->userName = this->pw = this->database = nullptr;
		this->port = 0x00; 
		
		this->sql = nullptr;
		this->result = nullptr;
	}
	DataBase(const char* serverAddr, const char* user, const char* password, const char* db, DWORD _port, MYSQL* _mysql) {
		this->init(serverAddr, user, password, db, _port, _mysql);
	}
	~DataBase() {
		if (this->isConnected) {
			this->disconnect();
		}
	}
	void init(const char* serverAddr, const char* user, const char* password, const char* db, DWORD _port, MYSQL* _mysql) {
		this->server = serverAddr;
		this->userName = user;
		this->sql = _mysql;
		this->result = nullptr;
		this->pw = password;
		this->database = db;
		this->port = _port;

		this->isConnected = false;

		mysql_init(this->sql);

		this->connect(true);
	}

	bool checkIsConnected() {
		return (this->isConnected = (mysql_ping(this->sql)==0));
	}

	bool reconnect() {
		if (this->checkIsConnected())
			return false;
		if (!mysql_real_connect(this->sql, this->server, this->userName, this->pw, this->database, this->port, nullptr, 0)) {
			return false;
		}
		return true;
	}

	bool connect(bool isInit = false) {
		if (!isInit && this->checkIsConnected())
			return false;
		if (!mysql_real_connect(this->sql, this->server, this->userName, this->pw, this->database, this->port, nullptr, 0)) {
			this->showErrorMsg();
			return false;
		}
		this->isConnected = true;
		return true;
	}

	bool disconnect() {
		if (!this->checkIsConnected())
			return false;
		mysql_close(this->sql);
		this->isConnected = false;
		return true;
	}

	MYSQL_RES* get(const char* fmt, ...) {
		ArgConverterA(result, fmt);
		if (mysql_query(this->sql, result.c_str()) != 0) {
			this->showErrorMsg();
			if (this->reconnect())
				return get(result.c_str());
			else
				return false;
		}
		if (this->result) {
			this->freeResult();
		}
		this->result = mysql_store_result(this->sql);
		return this->result;
	}

	bool put(const char* fmt, ...) {
		ArgConverterA(result, fmt);
		if (mysql_query(this->sql, result.c_str()) != 0) {
			this->showErrorMsg();
			if (this->reconnect())
				return put(result.c_str());
			else
				return false;
		}
		return true;
	}

	MYSQL_ROW getNextEntry() {
		if (!this->result)
			return nullptr;
		return mysql_fetch_row(this->result);
	}
	DWORD getRowCount() {
		if (!this->result)
			return 0x00;
		return static_cast<DWORD>(mysql_num_rows(this->result));
	}

	void finishQuery() {
		this->freeResult();
	}

	__inline const char* lastError() { return mysql_error(this->sql); }

	void freeResult() {
		if (!this->result)
			return;
		mysql_free_result(this->result);
		this->result = nullptr;
	}

};

#endif //__COMMON_DATATYPES__