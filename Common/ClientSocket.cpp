#include "ClientSocket.h"

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

	unsigned char* data = reinterpret_cast<unsigned char*>(&this->packet);
	receivedBytesLocally = recv(this->getSocket(), (char*)&data[this->bytesReceived], bytesLeft, 0x00);
	
	//Didn't get anything - Socket Error.
	if (receivedBytesLocally <= 0)
		return false;

	this->bytesReceived += receivedBytesLocally;

	//Packet not fully received yet.
	if (receivedBytesLocally != bytesLeft)
		return true;

	//Just the header was sent.
	if (this->bytesReceived == Packet::DEFAULT_HEADER_OFFSET) {
		if (this->packet.getLength() < Packet::DEFAULT_HEADER_OFFSET)
			return false;

		WORD newLen = DecryptBufferHeader(&this->crypt.status, this->crypt.table, this->packet);
		this->bytesExpected = newLen;
		if (this->bytesExpected > Packet::DEFAULT_HEADER_OFFSET)
			return true;
	}
	if (!::DecryptBufferData(this->crypt.table, this->packet))
		return false;
	if (!this->handlePacket()) {
		return false;
	}
	this->bytesExpected = Packet::DEFAULT_HEADER_OFFSET;
	this->bytesReceived = 0x00;
	this->packet.newPacket(0x00, Packet::DEFAULT_HEADER_OFFSET);
	return true;
}

bool ClientSocket::sendData( Packet& pak ) {

	unsigned char* data = pak;
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

	DWORD dataSize = pak.getLength();
	//std::cout << "[OUT] Sending 0x" << std::hex << pak.getCommand() << std::dec << " with Length " << dataSize << "\n";


	EncryptBuffer(this->crypt.table, data);
	DWORD result = send(this->getSocket(), reinterpret_cast<char*>(data), dataSize, 0x00);
	return (result == dataSize);
}