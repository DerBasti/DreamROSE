#include "CharServer.h"

int main() {
	MYSQL mysql;
	CharServer server(29100, &mysql);
	server.loadEncryption();
	server.start();
}