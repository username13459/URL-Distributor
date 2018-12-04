#include<mutex>
#include<fstream>
#include<string>

using std::string;


#include "types.h"
#include "log.h"
#include "db.h"

namespace blogDB
{
	//Mutex for write perms to the DB
	std::mutex dbWrite;

	string dbFile = "URL DB.txt.db";

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
		if(blogArrays == NULL) throw dbNULLPointerException("DB ROOT NULL!");


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

	blog getBlog(blogIndexPair index)
	{
		return getBlogRef(index);
	}

	blog getBlog(blogIndex index)
	{
		return getBlogRef(index);
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
				if(i < totalNumBlogs) ret.push_back(getIndexPair(i));
				else break;
			}

			//Compare the DB to our shortlist
			{
				//Start comparing the db after the blogs we just seeded our list with
				for(blogIndex index = numBlogs; index < totalNumBlogs; index++)
				{
					//Check the current item in the DB against our list
					for(int i = 0; i < ret.size(); i++)
					{
						if(getBlogRef(ret[i]).getNumCopies() > getBlogRef(index).getNumCopies())
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

	std::vector<blog> getBlogsToArchive(int numBlogs, types::user archiver)
	{
		std::vector<blogIndexPair> pairs = getBlogIndexesToArchive(numBlogs);

		for(int i = 0; i < pairs.size(); i++)
		{
			getBlogRef(pairs[i]).addDownloader(archiver);
		}

		std::vector<blog> ret;

		ret.reserve(pairs.size());

		for(unsigned i = 0; i < pairs.size(); i++)
		{
			ret.push_back(getBlog(pairs[i]));
		}

		return ret;
	}

	void append(blog newBlog)
	{
		dbWrite.lock();
		if ((long long int)totalNumBlogs >= (long long int) maxArraySize * (long long int) maxArraySize) throw dbException("DB has reached max space!");

		//setBlogAt(totalNumBlogs, newBlog);
		getBlogRef(totalNumBlogs, true) = newBlog;
		totalNumBlogs++;
		dbWrite.unlock();
	}


	void loadDB()
	{
		dbWrite.lock();
		totalNumBlogs = 0;

		blogArrays = new blog*[maxArraySize];

		for(int i = 0; i < maxArraySize; i++)
		{
			blogArrays[i] = NULL;
		}

		dbWrite.unlock();
	}

	void saveDB(string location)
	{
		if(location == "") location = dbFile;

		progLog::write("Preparing to write database to file at " + location);
		
		dbWrite.lock();

		progLog::write("Database locked.");

		ofstream out;
		try
		{
			out.open(location);
		}
		catch(...)
		{
			location += "_fallback";
			progLog::write("<<CRITICAL>> Unable to open preferred output location, using " + location);
			try
			{
				out.open(location);
			}
			catch(...)
			{
				progLog::write("<<CRITICAL>> Unable to open fallback location!");
				progLog::write("<<CRITICAL>> DB SAVE FAILED!");
				dbWrite.unlock();
				return;
			}
		}

		out << "totalNumBlogs" << endl;
		out << totalNumBlogs << endl;
		out << endl;

		for(blogIndex i = 0; i < totalNumBlogs; i++)
		{
			try
			{
				string outputBlock = "";

				auto append = [&outputBlock](string data)
				{
					outputBlock += data + '\n';
				};

				blog curr = getBlog(i);

				append("<blog>");
				append("<URL>");
				append(curr.getURL());
				append("<State>");
				append(conv::toString((int)curr.getState()));
				append("<numCopies>");
				append(conv::toString(curr.getNumCopies()));
				append("<Archivers>");
				for(int i = 0; i < curr.getNumCopies(); i++)
				{
					append(curr.getArchivers()[i].getName());
				}
				append("</blog>\n");
				
				out << outputBlock;
				out.flush();

			}
			catch(dbOutOfBoundsException e)
			{
				progLog::write("Caught dbOutOfBounds during save: " + e.details);
			}
			catch(dbNULLPointerException e)
			{
				progLog::write("Caught dbNullPointer during save: " + e.details);
			}
			catch(dbException e)
			{
				progLog::write("Caught generic DB exception: " + e.details);
			}
			catch(...)
			{
				progLog::write("Caught unknown exception!");
			}
		}

		progLog::write("Database save complete.");

		out.close();

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