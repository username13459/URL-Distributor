#pragma once

#include<string>

using std::string;

#include "zlib/varConv.h"

#include "types.h"

using types::blog;

typedef long int blogIndex;
typedef std::pair<int, int> blogIndexPair;

struct dbException
{
	dbException() {}
	dbException(string details) { this->details = details; }

	string details = "";
};

struct dbOutOfBoundsException : dbException
{
	dbOutOfBoundsException() {}
	dbOutOfBoundsException(string details) { this->details = details; }
	dbOutOfBoundsException(blogIndex indexRequested) { this->details = "" + conv::toString(indexRequested) + " out of range!"; }
};

struct dbNULLPointerException : dbException
{
	dbNULLPointerException() {}
	dbNULLPointerException(string details) { this->details = details; }
	dbNULLPointerException(blogIndexPair indexRequested) { this->details = "" + conv::toString(indexRequested.first) + "," + conv::toString(indexRequested.second); }
};

namespace blogDB
{	
	static const long int maxArraySize = 1'000'000;

	blogIndexPair getIndexPair(blogIndex index);

	void append(blog newBlog);

	void loadDB();



};