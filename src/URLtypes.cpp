#include "zlib/varConv.h"

#include "URLtypes.h"
#include "util.h"

namespace types
{
	
	t_paramater::t_paramater(string src)
	{
		//Make sure there is an equals
		int equalPos = src.find('=');
		if (equalPos == string::npos) throw paramaterErrors::noEquals;

		//Extract the key and value from the string
		string key = src.substr(0, equalPos);
		string value = src.substr(equalPos + 1);

		//Make sure the key and value exist
		if (key == "") throw paramaterErrors::noKey;
		else if (value == "") throw paramaterErrors::noValue;

		//Assign the key and value values internally
		this->key = key;
		this->value = value;
	}

	t_paramater::t_paramater(string key, string value)
	{
		if (key == "") throw paramaterErrors::noKey;
		if (value == "") throw paramaterErrors::noValue;

		this->key;
		this->value = value;
	}


	t_URL::t_URL(string URL)
	{
		util::trimSpaces(URL);

		this->URL = URL;

		if (URL.find(' ') != string::npos) throw URLException("Spaces in URL body");

		if (URL.find("://") == string::npos)
		{
			protocol = "";
			URL = "://" + URL;	//Append the protocol-identifier bits so the rest will function properly
		}
		else
			protocol = URL.substr(0, URL.find("://"));
		
		//TODO: In the future, perhaps tweak this section so that the URL will be trimmed from the front once the data has been copied from it?

		string domainHunk = URL.substr(
			//Find the position just after the ";//"
			URL.find("://") + string("://").length(),

			//Find the first slash not part of the "://" following the protocol
			URL.find(
				"/",
				URL.find("://") + string("://").length()
			)
			//Subtract the length of the protocol to take account for unwanted characters at the beginning of the string
			- URL.find("://") - string("://").length()
		);

		string pathHunk;
		if (URL.find('/', URL.find("://")+ string("://").length()) == string::npos)
		{
			pathHunk = "";	//If there is no path
		}
		else
		{
			pathHunk = URL.substr(
				//Start just after the domain hunk ends
				protocol.length() + string("://").length() + domainHunk.length() + 1,

				//The end point we want to capture
				URL.rfind('/')
				//Subtract the beginning point to get the length
				- protocol.length() - string("://").length() - domainHunk.length() - 1
			);
		}

		//Isolate the port, if it exists
		if (domainHunk.find(":") != string::npos)
		{
			//Extract the port
			string stPort = domainHunk.substr(domainHunk.find(':')+1);

			//Trimthe port off the domainHunk
			domainHunk = domainHunk.substr(0, domainHunk.find(':'));

			//Convert the port to a number
			try { port = conv::toNum(stPort); }
			catch (conv::convertError e)
			{
				//If we can't convert the string-represention of the port, assume it was a "false-alarm"
				port = -1;
			}
		}

		//Isolate the domain list from the domainHunk
		{
			while (domainHunk.find('.') != string::npos)
			{
				domains.push_back(domainHunk.substr(0, domainHunk.find('.')));

				domainHunk = domainHunk.substr(domainHunk.find('.') + 1);
			}

			//Catch the root domain (which doesn't have a dot on the end)
			domains.push_back(domainHunk);

			util::reverseVector(domains);
		}

		//If there isn't at least 1 domain (excluding the com/org/etc) this URL isn't valid
		if (domains.size() < 2)	throw noDomainException();

		//Isolate the path elements from the pathHunk
		{
			while(pathHunk.find('/') != string::npos)
			{
				path.push_back(pathHunk.substr(0, pathHunk.find('/')));

				pathHunk = pathHunk.substr(pathHunk.find('/') + 1);
			}

			if(pathHunk != "") path.push_back(pathHunk);

		}

		URL = URL.substr(URL.rfind('/') + 1);

		//Trim the anchor off the end of the URL, if it exists.
		if (URL.find('#') != string::npos)
		{
			anchor = URL.substr(URL.find('#')+1);
			URL = URL.substr(0, URL.find('#'));
		}

		//Trim the paramaters off the end of the URL, if they exist
		if(URL.find('?') != string::npos)
		{
			string params = URL.substr(URL.find('?')+1);

			URL = URL.substr(0, URL.find('?'));

			while (params.find('&') == string::npos)
			{
				paramaters.push_back(t_paramater(
					params.substr(0, params.find('&'))
				));

				params = params.substr(params.find('&')+1);
			}

			paramaters.push_back(t_paramater(params));
		}

		filename = URL;

	}

	string t_URL::getReconstructed()
	{
		string rec = "";

		//Append the protocol
		rec += protocol + "://";

		//Append all domains, except for the root one
		for (int i = domains.size() - 1; i != 0; i--)
		{
			rec += domains[i] + '.';
		}
		
		//Append the root domain (Done separately so the period isn't included)
		rec += domains[0];

		if (hasPort())// rec += (":" + port);
		{
			rec += ':';
			rec += conv::toString(port);
		}

		rec += '/';

		//Append the path
		for (int i = 0; i < path.size(); i++)
		{
			rec += path[i] + '/';
		}

		//Append the filename;
		rec += filename;

		//Append the paramaters
		if (paramaters.size() > 0)
		{
			//The first paramater is handled separately along with the '?' so that trailing '&'s aren't produced
			rec += '?' + paramaters[0].getKey() + '=' + paramaters[0].getValue();
			
			//Append the rest of the paramaters, preceeded with '&'
			for (int i = 1; i < paramaters.size(); i++)
			{
				rec += '&' + paramaters[i].getKey() + '=' + paramaters[i].getValue();
			}
		}

		if (hasAnchor()) rec += '#' + anchor;

		return rec;
	}
}