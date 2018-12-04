
#include "types.h"

#include "db.h"

namespace blogDB
{
	//An array pointing to arrays, which themselves contain blogs.  The backbone of the DB
	blog ** blogArrays = NULL;
	//The length of blogArrays
	int blogArraysLen = 0;


	//Separates a raw index into a major and minor index (which each correspond to a level in the array tree)
	blogIndexPair getIndexPair(blogIndex index)
	{
		return blogIndexPair(index / maxArraySize, index % maxArraySize);
	}

	
	//Returns a reference to the blog at 'index'
	blog & getBlogRef(blogIndexPair index)
	{
		blogIndex rawLen = index.first * maxArraySize + index.second;

		if (rawLen >= blogArraysLen) throw dbOutOfBoundsException(rawLen);

		if (blogArrays == NULL) throw dbNULLPointerException("DB ROOT NULL!!!!");

		if (blogArrays[index.first] == NULL) throw dbNULLPointerException(index);

		return blogArrays[index.first][index.second];
	}

	//Returns a reference to the blog at 'index'
	blog & getBlogRef(blogIndex index)
	{
		return getBlogRef(getIndexPair(index));
	}

	


	void append(blog newBlog)
	{
		if (blogArraysLen == maxArraySize * maxArraySize) throw dbException("DB has reached max space!");

		getBlogRef(blogArraysLen) = newBlog;
		blogArraysLen++;
	}


	void loadDB()
	{
		blogArraysLen = 0;

		blogArrays = new blog*[maxArraySize];
	}




}