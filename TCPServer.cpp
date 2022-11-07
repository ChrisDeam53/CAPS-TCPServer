//#include "stdafx.h"

// Used to deal with Protocol Requests.

#include <iostream>
// For String verification with the Protocol Verifier.
#include <string>

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include "TCPServer.h"

// Need to link with Ws2_32.lib.
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib").

#define PORT_BUFFER_LEN 10
#define DEFAULT_BUFLEN 65536

/* Constructor to initialise server.
   Initialises port.
*/
TCPServer::TCPServer(unsigned short int port)
{
	WSADATA wsaData;
	char portString[PORT_BUFFER_LEN];
	int iResult;

	RequestHandler requestHandler;

	this->port = port;

	snprintf(portString, PORT_BUFFER_LEN - 1, "%d", port);
	this->portString = portString;

	// Initialize Winsock.
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		exit(1);
	}

	this->OpenListenSocket();
}

/*
Opens port listener.
*/
void TCPServer::OpenListenSocket()
{
	int iResult;

	ListenSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port.
	iResult = getaddrinfo(NULL, portString.c_str(), &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		exit(1);
	}

	// Create a SOCKET for connecting to server.
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	// Setup the TCP listening socket.
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
}

/*
Used to accept the TCPClient.
*/
ReceivedSocketData TCPServer::accept()
{
	SOCKET ClientSocket = INVALID_SOCKET;
	ReceivedSocketData ret;

	ret.ClientSocket = INVALID_SOCKET;

	// Accept a client socket.
	ClientSocket = ::accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET)
	{
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	ret.ClientSocket = ClientSocket;

	return ret;
}

/*
@brief Method to receive data on the server, from client.
The protocol verifier tests need to be made to ensure all pass. (18/18)
@param ret - Socket data object -> Reply & Request.
@param blocking - Pased in as false(0) by main.cpp
*/
void TCPServer::receiveData(ReceivedSocketData &ret, bool blocking)
{
	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN - 1;
	struct pollfd pollDescriptor;

	pollDescriptor.fd = ret.ClientSocket;
	pollDescriptor.events = POLLIN;
	pollDescriptor.revents = 0;

	int retval;

	ret.request = "";
	ret.reply = "";

	if (!blocking)
		// If blocking
		retval = WSAPoll(&pollDescriptor, 1, 100); // Timeout in ms.

	if (blocking || (!blocking && retval > 0))
	{
		// Not blocking & bytes received
		iResult = recv(ret.ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0)
		{
			// iResult contains the number of bytes received.
			recvbuf[iResult] = '\0';
			// The request is equal to the buffer -> A message successfully received.
			ret.request = std::string(recvbuf);
		}
		else if (iResult < 0)
		{
			// iResult is 0 if connection is closed.
			int SocketError = WSAGetLastError();

			if (SocketError == WSAESHUTDOWN || SocketError == WSAECONNRESET || SocketError == WSAECONNABORTED || SocketError == WSAENETRESET)
			{
				// Connection has been closed, terminated, aborted or reset.
				closesocket(ret.ClientSocket);
				ret.ClientSocket = INVALID_SOCKET;
			}
			else if (SocketError != WSAEWOULDBLOCK)
			{
				printf("recv failed with error: %d\n", WSAGetLastError());
				closesocket(ret.ClientSocket);
				ret.ClientSocket = INVALID_SOCKET;
				WSACleanup();
				exit(1);
			}
		}
	}

	/*
		At this point, RecievedSocktData object has succesfully obtained a request.
		The server must allow for checks to be made so that it may conform to the Protocol Verifier checks. (18/18)
		Checks are made in the order as outlined in the ``Message Board Protocol`` html web-page. (apart from EXIT)
	*/
	if (ret.request.find("EXIT" || "exit") != std::string::npos)
	{
		// Check if the request is not an exit first.
		// Terminates the server.
		requestHandler.terminateRequestHandler(ret);
	}
	else if (ret.request.rfind("POST") != std::string::npos)
	{
		// Post a message.
		requestHandler.postMessageHandler(ret);
	}
	else if (ret.request.rfind("LIST") != std::string::npos)
	{
		// List all topicIDs.
		requestHandler.listMessageHandler(ret);
	}
	else if (ret.request.rfind("COUNT") != std::string::npos)
	{
		// Counts the number of posts on a given topic, identified by topicID.
		requestHandler.countMessageHandler(ret);
	}
	else if (ret.request.rfind("READ") != std::string::npos)
	{
		// Looks up a topic (identified by topicID) and returns the request message (identified by postID).
		requestHandler.readMessageHandler(ret);
	}
}

/*
Used to send the result, using C-Style String.
*/
int TCPServer::sendReply(ReceivedSocketData reply)
{
	int iSendResult;

	// Echo the buffer back to the sender.
	iSendResult = send(reply.ClientSocket, reply.reply.c_str(), (int)reply.reply.size()+1, 0);

	if (iSendResult == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(reply.ClientSocket);
		WSACleanup();
		return 1;
	}

	return iSendResult;
}

/*
Closes the socket & ends server.
*/
int TCPServer::closeClientSocket(ReceivedSocketData &reply)
{
	int iResult;

	// Shutdown the connection since we're done
	iResult = shutdown(reply.ClientSocket, SD_SEND);

	if (iResult == SOCKET_ERROR)
	{
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(reply.ClientSocket);
		WSACleanup();
		return 1;
	}

	// Cleanup.
	closesocket(reply.ClientSocket);

	return iResult;
}

void TCPServer::CloseListenSocket()
{
	// No longer need server socket.
	closesocket(ListenSocket);
}

TCPServer::~TCPServer()
{
	CloseListenSocket();
	WSACleanup();
}


