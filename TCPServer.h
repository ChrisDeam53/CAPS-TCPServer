#ifndef __TCPSERVER_H
#define __TCPSERVER_H

#include "ReceivedSocketData.h"
#include "RequestHandler.h"

class TCPServer
{
public:
	TCPServer(unsigned short int port);
	~TCPServer();
	ReceivedSocketData accept();
	void receiveData(ReceivedSocketData& ret, bool blocking);
	int sendReply(ReceivedSocketData reply);

	void OpenListenSocket();
	void CloseListenSocket();
	int closeClientSocket(ReceivedSocketData &reply);

private:
	SOCKET ListenSocket;
	unsigned short int port;
	std::string portString;

	RequestHandler requestHandler;

};

#endif __TCPSERVER_H
