#ifndef __TCPSERVER_H
#define __TCPSERVER_H

#include "ReceivedSocketData.h"
#include "RequestParser.h"

// Standard Templete Library.
// Note: Contains key-value pairs with unique keys.
#include <map>
// STL vector - Used to store Messageboard posts - size can change during compilation.
#include <vector>

// Work here.

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

	// Handlers for Server requests recieved from the client.
	void postMessageHandler(ReceivedSocketData& ret);
	void listMessageHandler(ReceivedSocketData& ret);
	void countMessageHandler(ReceivedSocketData& ret);
	void readMessageHandler(ReceivedSocketData& ret);
	void terminateRequestHandler(ReceivedSocketData& ret);

private:
	SOCKET ListenSocket;
	unsigned short int port;
	std::string portString;

	PostRequest postParser;
	ReadRequest readParser;
	CountRequest countParser;
	ListRequest listParser;
	ExitRequest exitParser;

	/*
	// NOTE: STL Map seems preferable over a Hash-Table for implementation.
	// NOTE: Map would run faster - Hash-table would be better for Multithreading safety.
	// NOTE: Map is faster, therefore the optimal choice.
	// Map will hold:
	// std::string - PostID name.
	// std::vector<std::string> - Vector of strings to contain the messages - Post Message.
	*/
	std::map <std::string, std::vector<std::string>> messageBoard;
};

#endif __TCPSERVER_H
