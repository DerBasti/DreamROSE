#pragma once

#ifndef __ROSE_LOGIN_CLIENT__
#define __ROSE_LOGIN_CLIENT__

#include "LoginServer.h"

class LoginClient : public ClientSocket {
	private:
		struct _accountInfo {
			DWORD userId;
			DWORD accessLevel;
			std::string userName;
			std::string password;
		} accountInfo;

		bool pakEncryptionRequest();
		bool pakRequestChannelList();
		bool pakUserLogin();
		bool pakRequestChannelIP();

	public:
		LoginClient(SOCKET sock, ServerSocket* pDelegate) {
			this->active = false;
			this->socket = sock;
			this->serverDelegate = pDelegate;

			this->accountInfo.userId = 0x00; this->accountInfo.accessLevel = 1;
			this->accountInfo.userName = this->accountInfo.password = "";
		}
		~LoginClient() {
			//...nothing to do
		}

		bool handlePacket();
		void setBaseInfo(const WORD userId, const DWORD accessLevel, std::string& userNam, std::string& pw);
};

#endif //__ROSE_LOGIN_CLIENT__