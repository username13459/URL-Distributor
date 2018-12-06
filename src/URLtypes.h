#pragma once
#include <string>
#include <vector>

using std::string;
using std::vector;

#include "types.h"

namespace types
{
	struct t_paramater
	{
		enum class paramaterErrors
		{
			noEquals,	//If there is no '=' in the string provided to convert to a paramater
			noKey,		//If the string starts with an '=' (aka it has no key before it)
			noValue,	//If the strign ends with '=' (aka it has no value after it)
		};

		//>>Creates an empty instance of t_paramater
		//@Ensures: 'key' and 'value' are both set to ""
		t_paramater() {}
		//>>Creates an instance of t_paramater using data extracted from 'src'
		//@Requires: 'src' is a string in '<key>=<value>' form, and that '<key>' and '<value>' both exist
		//@Ensures:  'key' is the value of the key in 'src'
		//@Ensures:  'value' is the valeu of the value in 'src'
		//@Exceptions: 'paramaterErrors::{noEquals, noKey, noValue'
		t_paramater(string src);
		//>>Creates an instance of t_paramater using 'key' and 'value' 
		//@Requires: 'key' and 'value' are not "" (that they are something)
		t_paramater(string key, string value);

	protected:
		string key;
		string value;

	public:
		string getKey() { return key; }
		string getValue() { return value; }
	};

	struct URLException : Exception
	{
		URLException() {}
		URLException(string details) { this->details = details; }
	};

	struct noDomainException : URLException
	{
		noDomainException() {}
		noDomainException(string details) { this->details = details; }
	};

	//A class to store A URL, both in its original form and separated into its components
	struct t_URL
	{
	public:
		t_URL() {};
		t_URL(string URL);

	protected:
		//The original URL, not modified in any way.
		string URL = "";

		//The protocol, without the "://"  (i.e. HTTP, HTTPS, etc.)
		//Realistically only HTTP(S), but here for the sake of completeness
		string protocol = "";
		
		//The domains, in directory format (reverse order)
		//"sub.example.com" would be {"com", "example", "sub"}
		vector<string> domains;

		//The port specified to connect on, if it exists.  -1 if it does not
		int port = -1;

		//The path portion of the URL, in order (if present)
		//for "example.com/path/filename.HTML" the path is { "path" }
		vector<string> path;

		//The filename (Again, if present)
		//for "example.com/path/filename.HTML" the filename is "filename.HTML"
		string filename = "";

		//The paramaters in the URL (everything after the '?')
		vector<t_paramater> paramaters;

		//Anything after the #
		string anchor = "";

	public:
		
		//Returns the raw URL that the components were extracted from
		inline string get() { return URL; }
		//Returns the URL, reconstructed from the components
		string getReconstructed();
		
		//Returns the protocol of the URL
		inline string getProtocol() { return protocol; }

		//Returns all the domains present in the URL (in reverse order)
		inline vector<string> getDomains() { return domains; }
		//Returns a specific domain in the comain path (indexed in reverse order)
		inline string getDomain(int index) { return domains[index]; }

		//Returns true if the port is specified in the URL
		inline bool hasPort() { return getPort() != -1; }
		//Returns the port of the host (if specified, -1 if otherwise)
		inline int getPort() { return port; }

		//Returns the full path of the URL
		inline vector<string> getPath() { return path; }
		//Returns a segment in the full path of the URL
		inline string getPathSegment(int index) { return path[index]; }

		//Returns the filename in the URL
		inline string getFilename() { return filename; }

		//Returns all the paramaters in the URL
		inline vector<t_paramater> getParamaters() { return paramaters; }
		//Returns a paramater
		inline t_paramater getParamater(int index) { return paramaters[index]; }

		//Returns true if an anchor exists
		inline bool hasAnchor() { return anchor != ""; }
		//Returns the anchor
		inline string getAnchor() { return anchor; }
	};

	struct t_tag
	{
		string tag;
		enum {
			creator

		} type;


	};
}