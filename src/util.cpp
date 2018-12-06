#include <string>
#include <sstream>
#include <vector>

using std::string;

#include "util.h"



namespace util
{
	string trimSpaces(string & inp, bool changeArg)
	{
		string ret;
		string * ref;

		//If 'changeArg == true', set 'ref' to 'inp' so the input is modified.  If not, store 'inp' in 'ret' and point 'ref' to 'ret' so the input is not modified
		if (changeArg) ref = &inp;
		else
		{
			ret = inp;
			ref = &ret;
		}

		//Erase the first character until it is not a space
		while ((*ref).length() > 0 && (*ref)[0] == ' ')
		{
			(*ref).erase((*ref).begin());
		}

		//Erase the last character until it is not a space
		while ((*ref).length() > 0 && (*ref)[0] == ' ')
		{
			(*ref).erase((*ref).end() - 1);
		}

		return ret;
	}

	std::vector<string> splitBySpace(string inp)
	{
		std::vector<string> ret;
		string chunk = "";

		bool inQuotes = false;

		for (int i = 0; i < inp.length(); i++)
		{
			if (inp[i] == '"')
			{
				inQuotes = !inQuotes;
			}
			else if(inp[i] == ' ')
			{
				if (inQuotes) chunk += inp[i];
				else
				{
					if (chunk != "")
					{
						ret.push_back(chunk);
						chunk = "";
					}
				}
			}
			else chunk += inp[i];
		}

		if (chunk != "") ret.push_back(chunk);

		return ret;
	}
}