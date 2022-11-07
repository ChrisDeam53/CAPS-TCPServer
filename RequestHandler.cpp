#include "RequestHandler.h"

RequestHandler::RequestHandler()
{
}

RequestHandler::~RequestHandler()
{

}

/*
@brief The handle to deal with POST requests.
@param &ret - Socket data object -> Request data sent from the TCPclient.
*/
void RequestHandler::postMessageHandler(ReceivedSocketData& ret)
{
	PostRequest postParser = postParser.parse(ret.request);

	if (postParser.valid)
	{
		// POST request is valid.
		// Only one thread can write the value at a time.
		std::unique_lock<std::shared_mutex> mutexLock(sharedMutex);

		if (messageBoard.find(postParser.getTopicId()) != messageBoard.end())
		{
			// TopicID (Key in the Hashmap) has been found - Not returned the end of the data structure.
			// Note: After researching push_back vs emplace_back -> emplace_back was found to be faster by ~7.62%.
			messageBoard.find(postParser.getTopicId())->second.emplace_back(postParser.getMessage());

			// Reply with a postID (non-negatie number).
			// Return the size of the vector-1 to match 0-based indexing.
			ret.reply = std::to_string(messageBoard.find(postParser.getTopicId())->second.size() - 1);
		}
		else
		{
			// TopicID does not exist. Create the topic. Associated message has postID 0.
			// Initialise a vector of string to insert the message.
			std::vector<std::string> postMessageVector;
			postMessageVector.emplace_back(postParser.getMessage());

			// Insert the topicID & instantiated string vector.
			messageBoard.insert({ postParser.getTopicId(), postMessageVector });

			// Topic does not exist, it is created and the message becomes its first post.
			// 0 is returned, as outlined in the standard.
			ret.reply = "0";
		}
	}
}

/*
@brief The handle to deal with LIST requests.
@param &ret - Socket data object -> Request data sent from the TCPclient.
*/
void RequestHandler::listMessageHandler(ReceivedSocketData& ret)
{
	ListRequest listParser = listParser.parse(ret.request);

	if (listParser.valid)
	{
		// LIST request is valid.
		// Multiple threads/readers can read the value at the same time.
		std::shared_lock<std::shared_mutex> mutexLock(sharedMutex);

		std::string topicListings;
		const std::string hashDivider("#");

		for (std::map<std::string, std::vector<std::string>>::iterator i = messageBoard.begin(); i != messageBoard.end(); i++)
		{
			// Ensure all topicIDs are separated by the "#" character as outlined in the protocol.html.
			topicListings.append(i->first).append(hashDivider);
		}

		// Remove the trailing # character.
		if (!topicListings.empty())
		{
			topicListings.pop_back();
		}
		ret.reply = topicListings;
	}
}

/*
@brief The handle to deal with COUNT requests.
@param &ret - Socket data object -> Request data sent from the TCPclient.
*/
void RequestHandler::countMessageHandler(ReceivedSocketData& ret)
{
	CountRequest countParser = countParser.parse(ret.request);

	if (countParser.valid)
	{
		// COUNT request is valid.
		// Multiple threads/readers can read the value at the same time.
		std::shared_lock<std::shared_mutex> mutexLock(sharedMutex);

		if (messageBoard.find(countParser.getTopicId()) != messageBoard.end())
		{
			// TopicID (Key in the Hashmap) has been found - Not returned the end of the data structure.
			// Parse the int to string.
			ret.reply = std::to_string(messageBoard.find(countParser.getTopicId())->second.size());
		}
		else
		{
			// Topic does not exist - Return 0.
			ret.reply = "0";
		}
	}
}

/*
@brief The handle to deal with READ requests.
@param &ret - Socket data object -> Request data sent from the TCPclient.
*/
void RequestHandler::readMessageHandler(ReceivedSocketData& ret)
{
	ReadRequest readParser = readParser.parse(ret.request);

	if (readParser.valid)
	{
		// READ request is valid.
		// Multiple threads/readers can read the value at the same time.
		std::shared_lock<std::shared_mutex> mutexLock(sharedMutex);

		if (messageBoard.find(readParser.getTopicId()) != messageBoard.end() && readParser.getPostId() >= 0 && messageBoard.size() > 0 && messageBoard.find(readParser.getTopicId())->second.size() - 1 >= readParser.getPostId())
		{
			// TopicID (Key in the Hashmap) has been found - Not returned the end of the data structure.
			// MessageBoard is not empty (size must be greater than 0).
			// Message index exists -> The size-1 of the std::vector (matching a Vector that is 0-based index) is >= postID.
			ret.reply = messageBoard.find(readParser.getTopicId())->second.at(readParser.getPostId());
		}
		else
		{
			// Respond with a blank line.
			ret.reply = "";
		}
	}
}



/*
@brief The handle to deal with EXIT|exit requests.
@param &ret - Socket data object -> Request data sent from the TCPclient.
*/
void RequestHandler::terminateRequestHandler(ReceivedSocketData& ret)
{
	ExitRequest exitParser = exitParser.parse(ret.request);

	if (exitParser.valid)
	{
		// EXIT request is valid.

		ret.reply = "TERMINATING";
	}
}