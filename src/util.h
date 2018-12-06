#pragma once

#include <string>
#include <sstream>
#include <vector>

using std::string;



namespace util
{
	//>>Trims the spaces from inp, potentially modifies inp
	//@Modifies: inp (if 'changeArg == true')
	//@Ensures:  All leading & trailing spaces present in 'inp' are removed in the returned value.
	//@Ensures:	 The value returned will be stripped of all leading and trailing spaces (if they exist)
	//@Ensures:  If 'changeArg == true', 'inp' will be modified (spaces removed)
	//@Ensures:  If 'changeArg == false', 'inp' will not be modified
	string trimSpaces(string & inp, bool changeArg = false);

	template<typename T>
	void reverseVector(T & vec)
	{
		for (int i = 0; i < vec.size() / 2; i++)
		{
			std::swap(vec[i], vec[vec.size() - 1 - i]);
		}
	}

	std::vector<string> splitBySpace(string inp);
}