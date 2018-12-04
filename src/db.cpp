#include<mutex>


#include "types.h"
#include "log.h"
#include "db.h"

namespace blogDB
{
	//Mutex for write perms to the DB
	std::mutex dbWrite;

	//An array pointing to arrays, which themselves contain blogs.  The backbone of the DB
	blog ** blogArrays = NULL;
	//The length of blogArrays
	//int blogArraysLen = 0;
	//The number of blogs in the DB
	blogIndex totalNumBlogs = 0;



	//Separates a raw index into a major and minor index (which each correspond to a level in the array tree)
	blogIndexPair getIndexPair(blogIndex index)
	{
		return blogIndexPair(index / maxArraySize, index % maxArraySize);
	}

	
	//Returns a reference to the blog at 'index'
	//If createMode is true, the db will create missing arrays as it goes, and disregard accessing positions past the end of the current db
	blog & getBlogRef(blogIndexPair index, bool createMode = false)
	{
		if(!createMode)
		{
			blogIndex rawLen = index.first * maxArraySize + index.second;

			if(rawLen >= totalNumBlogs) throw dbOutOfBoundsException(rawLen);

		}

		//This shouldn't EVER happen, so createMode won't override this
		throw dbNULLPointerException("DB ROOT NULL!");


		if(blogArrays[index.first] == NULL)
		{
			if(!createMode) throw dbNULLPointerException(index);
			else blogArrays[index.first] = new blog[maxArraySize];
		}

		return blogArrays[index.first][index.second];
	}

	//Returns a reference to the blog at 'index'
	blog & getBlogRef(blogIndex index, bool createMode = false)
	{
		return getBlogRef(getIndexPair(index), createMode);
	}

	std::vector<blog> getBlogsToArchive(int numBlogs, types::user archiver)
	{
		std::vector<blogIndexPair> pairs = getBlogIndexesToArchive(numBlogs);

		for(int i = 0; i < pairs.size(); i++)
		{
			getBlogRef(pairs[i]).addDownloader(archiver);
		}
	}

	//Returns a vector contaning blog index-pairs, where the blogs contained have the least number of copies present in the DB
	std::vector<blogIndexPair> getBlogIndexesToArchive(int numBlogs)
	{
		std::vector<blogIndexPair> ret;

		ret.reserve(numBlogs);

		try
		{
			//"Seed" the list of blogs to return
			for(int i = 0; i < numBlogs; i++)
			{
				ret.push_back(getIndexPair(i));
			}

			//Compare the DB to our shortlist
			{
				//Start comparing the db after the blogs we just seeded our list with
				for(blogIndex index = numBlogs; index < totalNumBlogs; index++)
				{
					//Check the current item in the DB against our list
					for(int i = 0; i < ret.size(); i++)
					{
						if(getBlogRef(ret[i]).getNumCopies() < getBlogRef(index).getNumCopies())
						{
							ret[i] = getIndexPair(index);
							break;
						}
					}

					//Check if all the blogs in our batch haven't been archived (copies ==0).  If so, just return this list
					{
						bool allZero = true;
						for(int j = 0; j < ret.size(); j++)
						{

							if(getBlogRef(ret[j]).getNumCopies() != 0)
							{
								allZero = false;
								break;
							}
						}

						if(allZero) return ret;
					}


				}
			}




		}
		catch(dbOutOfBoundsException e)
		{
			//If we get here we assume we tried to get more blogs than exist in the DB
		};

		return ret;
	}


	void append(blog newBlog)
	{
		dbWrite.lock();
		if (totalNumBlogs == maxArraySize * maxArraySize) throw dbException("DB has reached max space!");

		getBlogRef(totalNumBlogs, true) = newBlog;
		totalNumBlogs++;
		dbWrite.unlock();
	}


	void loadDB()
	{
		dbWrite.lock();
		totalNumBlogs = 0;

		blogArrays = new blog*[maxArraySize];




		dbWrite.unlock();
	}

	void sortDB()
	{
		//This works off a crappy bubble-sort algorithm, and should DEF be replaced later
		progLog::write("Preparing to sort DB");
		dbWrite.lock();
		progLog::write("DB lock acquired.");

		bool sorted = false;
		long long unsigned numPasses = 0;
		while(!sorted)
		{
			sorted = true;
			for(blogIndex i = 0; i < totalNumBlogs - 1; i++)
			{
				if(getBlogRef(i).getNumCopies() > getBlogRef(i + 1).getNumCopies())
				{
					sorted = false;
					std::swap(getBlogRef(i), getBlogRef(i + 1));
				}
			}
			numPasses++;
			if(numPasses % 100 == 0) progLog::write("DB Sort passes made: " + conv::toString(numPasses));
		}

		progLog::write("Sorting finished.");
		dbWrite.unlock();
	}


}