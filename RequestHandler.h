#pragma once

// Include all Regex classes from RequestParser.h
#include "RequestParser.h"
// Include Socket Data information.
#include "ReceivedSocketData.h"

// Standard Templete Library.
// Note: Contains key-value pairs with unique keys.
#include <map>
// STL vector - Used to store Messageboard posts - size can change during compilation.
#include <vector>
// Used for shared mutex - Threads. (shared_lock)
#include <shared_mutex>
// Used for locking - Threads. (unique_lock)
#include <mutex>

class RequestHandler
{
public:

	// Default Constructor.
	RequestHandler();

	// Destructor.
	~RequestHandler();

	// Handlers for Server requests recieved from the client.
	void postMessageHandler(ReceivedSocketData& ret);
	void listMessageHandler(ReceivedSocketData& ret);
	void countMessageHandler(ReceivedSocketData& ret);
	void readMessageHandler(ReceivedSocketData& ret);
	void terminateRequestHandler(ReceivedSocketData& ret);

private:

	/*
	// NOTE: STL Map seems preferable over a Hash-Table for implementation.
	// NOTE: Map would run faster - Hash-table would be better for Multithreading safety.
	// NOTE: Map is faster, therefore the optimal choice.
	// Map will hold:
	// std::string - PostID name.
	// std::vector<std::string> - Vector of strings to contain the messages - Post Message.
	*/
	std::map <std::string, std::vector<std::string>> messageBoard;

	// Shared Mutex (mutual exclusion).
	std::shared_mutex sharedMutex;
};

