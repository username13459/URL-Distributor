#pragma once
#include<string>

using std::string;

namespace progLog
{
	//Prepares for logging, opening the log file and etc.
	void startLog();

	//Writes a message to the log, with the timestamp appended to the front
	void write(string message);

	//Closes the current log
	void closeLog();
}
