#include "LoginClient.h"
#include "LoginServer.h"

int main() {
	MYSQL mysql;
	LoginServer server(29000, &mysql);
	server.loadEncryption();
	server.start();
	
}