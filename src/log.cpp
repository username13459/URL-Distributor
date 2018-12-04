#include <iostream>
#include <fstream>
#include <mutex>
#include <string> 

using std::string;
using std::cout;
using std::endl;

#include "zlib/varTypes.h"

#include "log.h"



namespace progLog
{
	//The prefix and postfix for the log file name, to be placed around the timestamp the log was created
	std::string logFilePrefix = "log";
	std::string logFilePostfix = ".log";

	//The ofstream for the log
	std::ofstream logOut;

	bool fileOpen = false;

	std::mutex writePerms;

	void startLog()
	{
		var::longTime now = var::longTime::now();
		string logName = logFilePrefix + now.getYMD() + "," + now.getHMS() + logFilePostfix;

		try {
			logOut.open(logName);
			fileOpen = true;
		}
		catch(...)
		{
			cout << "UNABLE TO START LOG!" << endl;
			throw "Unable to start log!";
		}
		
		write("Log started successfully!");
	}

	void closeLog()
	{
		write("Log closed.");
		writePerms.lock();
		logOut.close();
		fileOpen = false;
		writePerms.unlock();
	}

	void write(string message)
	{
		var::longTime now = var::longTime::now();
		writePerms.lock();

		cout << now.getYMD() << ',' << now.getHMS() << " | " << message << endl;

		if(fileOpen)
		{

			try
			{

				logOut << now.getYMD() << ',' << now.getHMS() << " | " << message << endl;
				logOut.flush();
			}
			catch(...)
			{
				cout << "<<CRITICAL>> Unknown exception when writing to log!" << endl;
			}
		}
		else cout << "<<CRITICAL>> Log not opened!" << endl;

		writePerms.unlock();
	}

}