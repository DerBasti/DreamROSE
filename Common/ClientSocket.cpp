#include "ClientSocket.h"
#include "D:\Programmieren\GlobalLogger\GlobalLogger.h"

ClientSocket::~ClientSocket() {
	//this->serverDelegate = nullptr;
	this->packet.newPacket(0x00, 0x00);
	this->closeSocket();
}

bool ClientSocket::receiveData() {

	int receivedBytesLocally = 0x00;
	short bytesLeft = this->bytesExpected - this->bytesReceived;

	//Packet full? wat.
	if (bytesLeft == 0)
		return false;

	byte_t* data = reinterpret_cast<byte_t*>(&this->packet);
	receivedBytesLocally = recv(this->getSocket(), (char*)&data[this->bytesReceived], bytesLeft, 0x00);
	
	//Didn't get anything - Socket Error.
	if (receivedBytesLocally <= 0) {
		GlobalLogger::warning("Couldn't receive data from client %s\n", this->ip.c_str());
		return false;
	}

	this->bytesReceived += receivedBytesLocally;

	//Packet not fully received yet.
	if (receivedBytesLocally != bytesLeft) 
		return true;

	//Just the header was sent.
	if (this->bytesReceived == Packet::DEFAULT_HEADER_OFFSET) {
		if (this->packet.getLength() < Packet::DEFAULT_HEADER_OFFSET) {
			GlobalLogger::fault("Client from %s sent an invalid packet\n", this->ip.c_str());
			return false;
		}

		word_t newLen = DecryptBufferHeader(&this->crypt.status, this->crypt.table, this->packet);
		this->bytesExpected = newLen;
		if (this->bytesExpected > Packet::DEFAULT_HEADER_OFFSET)
			return true;
	}
	if (!::DecryptBufferData(this->crypt.table, this->packet)) {
		GlobalLogger::fault("Couldn't decrypt packet from client %s\n", this->ip.c_str());
		return false;
	}
	if (!this->handlePacket()) {
		GlobalLogger::fault("Couldn't handle packet from client %s\n", this->ip.c_str());
		return false;
	}
	this->bytesExpected = Packet::DEFAULT_HEADER_OFFSET;
	this->bytesReceived = 0x00;
	this->packet.newPacket(0x00, Packet::DEFAULT_HEADER_OFFSET);
	return true;
}

bool ClientSocket::sendData( Packet& pak ) {

	byte_t* data = pak;
	/*
	FILE* fh = fopen("D:\\Games\\iROSE_OPENSOURCE\\packetlog_OWN.log", "a+");
	char buf[0x1000] = { 0x00 }; sprintf(buf, "PacketID: 0x%x, Size: %i\nContent:\n", pak.getCommand(), pak.getLength());
	for (unsigned int i = 0; i < pak.getLength() - Packet::DEFAULT_HEADER_OFFSET; i++) {
		sprintf(&buf[strlen(buf)], "0x%x ", data[i]);
		if (i % 10 == 0 && i>0)
			sprintf(&buf[strlen(buf)], "\n");
	}
	fputs(buf, fh);
	fclose(fh);
	*/

	dword_t dataSize = pak.getLength();
	//std::cout << "[OUT] Sending 0x" << std::hex << pak.getCommand() << std::dec << " with Length " << dataSize << "\n";


	EncryptBuffer(this->crypt.table, data);
	dword_t result = send(this->getSocket(), reinterpret_cast<char*>(data), dataSize, 0x00);
	return (result == dataSize);
}