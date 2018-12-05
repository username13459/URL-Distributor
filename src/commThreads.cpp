#include<thread>
#include<iostream>
#include<vector>
#include<functional>

using std::vector;

#include "zlib/network.h"

namespace commThreads
{
	vector<std::thread> threads;


	void connectionHandler(network::childSocket socket)
	{
		try
		{
			std::cout << "Connection established!" << std::endl;
			
			auto expect = [&socket](string message)
			{

			};



			socket.closeSocket();
			std::cout << "Connection finished." << endl;
		}
		catch(...)
		{
			std::cout << "Undefined thread error" << endl;
		}
	}


	void openNewConnection(network::childSocket socket)
	{
		//threads.push_back
		(std::thread{std::bind(connectionHandler, socket)}).detach();
	}

	//Checks if any threads are ready to join, and joins them if possible
	void joinIfPossible()
	{
		for(int i = 0; i < threads.size(); i++)
		{
			if(threads[i].joinable())
			{
				threads[i].join();
				threads.erase(threads.begin() + i);
				i--;
			}

		}
	}




}
