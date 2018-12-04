#pragma once

#include<string>

using std::string;

#include "zlib/varConv.h"

#include "types.h"

using types::blog;

typedef long int blogIndex;
typedef std::pair<int, int> blogIndexPair;

struct dbException : types::Exception
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

//Thrown when 'numBlogs' passed to getBlogsToArchive is a bad value
struct dbBadGetToArchiveException : dbException
{
	dbBadGetToArchiveException() {}
	dbBadGetToArchiveException(string details) { this->details = details; }
};

namespace blogDB
{	
	static const long int maxArraySize = 1'000'000;

	//Converts an index into an index pair
	blogIndexPair getIndexPair(blogIndex index);

	//Returns 'numBlogs' (or fewer) blogs to archive)
	std::vector<blog> getBlogsToArchive(int numBlogs);

	//Appends a blog to the dp
	void append(blog newBlog);

	//Loads the DB
	void loadDB();

	//Sorts the DB from least-archived to most-archived
	//THIS IS RESOURCE INTENSIVE, AND WILL LOCK THE DB FOR SOME TIME
	void sortDB();



};