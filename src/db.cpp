#include<mutex>
#include<fstream>
#include<string>
#include <iostream>

using std::string;


#include "types.h"
#include "URLtypes.h"
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

	blogIndex getSize() { return totalNumBlogs; }

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

	blogIndexPair getBlogByURL(string URL)
	{
		string formattedURL;

		try
		{
			types::t_URL url(URL);

			if (url.getDomains().size() == 3 && url.getDomain(2) != "" && url.getDomain(1) == "tumblr" && url.getDomain(0) == "com")
				formattedURL = url.getDomain(2) + ".tumblr.com";
			else
				formattedURL = URL;
		}
		catch (...)
		{
			throw dbSearchException("Unable to form URL from \"" + URL + "\".");
		}

		for (blogIndex i = 0; i < blogDB::getSize(); i++)
		{
			if (getBlogRef(i).getURL() == formattedURL)
			{
				return getIndexPair(i);
			}
		}

		return blogDB::npos;

	}

	//Returns a vector contaning blog index-pairs, where the blogs contained have the least number of copies present in the DB
	std::vector<blogIndexPair> getBlogIndexesToArchive(int numBlogs)
	{
		std::vector<blogIndexPair> ret;

		ret.reserve(numBlogs);

		try
		{
			//"Seed" the list of blogs to return
			{
				blogIndex seedIndex = 0;
				//True when blogs with the 'needArchive' flag are present in the DB
				bool needsArchiveExist = true;
				while (ret.size() < numBlogs)
				{
					if (seedIndex < totalNumBlogs)
					{
						
						if (getBlogRef(seedIndex).getState() == (needsArchiveExist ? types::archiveState::needsArchive : types ::archiveState::archived))
						{
							ret.push_back(getIndexPair(seedIndex));
						}
					}
					else
					{
						seedIndex = 0;
						needsArchiveExist = false;
						continue;
					}
					seedIndex++;
				}
			}

			//Compare the DB to our shortlist
			{
				//Start comparing the db after the blogs we just seeded our list with
				for(blogIndex index = numBlogs; index < totalNumBlogs; index++)
				{
					//Check the current item in the DB against our list
					for(int i = 0; i < ret.size(); i++)
					{
						//Make sure it's been validated
						if(getBlogRef(index).getState() == types::archiveState::needsArchive || getBlogRef(index).getState() == types::archiveState::archived)
						{
							if (getBlogRef(ret[i]).getNumCopies() > getBlogRef(index).getNumCopies())
							{
								bool isUnique = true;
								//Makes sure the blog in question isn't already on the list
								for(int q = 0; q < ret.size(); q++)
								{
									if (ret[q] == getIndexPair(index))
									{
										isUnique = false;
										break;
									}
								}
								if(isUnique) ret[i] = getIndexPair(index);
								break;
							}
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

	std::vector<blog> getBlogsToArchive(int numBlogs, types::archive archiver)
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

	void approveBlog(blogIndexPair loc)
	{
		dbWrite.lock();
		getBlogRef(loc).approve();
		dbWrite.unlock();
	}

	void rejectBlog(blogIndexPair loc)
	{
		dbWrite.lock();
		getBlogRef(loc).reject();
		dbWrite.unlock();
	}

	void append(blog newBlog, bool ignoreDupeCheck)
	{
		if (!ignoreDupeCheck)
		{
			//Here so we can check all blogs we know of without locking the DB, and then look for any added while we were looking
			blogIndex lastKnownSize = totalNumBlogs;

			//Check to see if the blog already exists
			for (blogIndex i = 0; i < lastKnownSize; i++)
			{
				if (getBlogRef(i).getURL() == newBlog.getURL())
				{
					progLog::write("Blog \"" + newBlog.getURL() + "\" is arlready present in the DB.");
					return;
				}
			}

			dbWrite.lock();

			//Check for blogs added while we were doing the first round of checking?
			for (blogIndex i = lastKnownSize; i < totalNumBlogs; i++)
			{
				if (getBlogRef(i).getURL() == newBlog.getURL())
				{
					progLog::write("Blog \"" + newBlog.getURL() + "\" is arlready present in the DB.");
					return;
				}
			}
		}
		else dbWrite.lock();

		if ((long long int)totalNumBlogs >= (long long int) maxArraySize * (long long int) maxArraySize) throw dbException("DB has reached max size!");

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

		ifstream load;
	
		zlib::timer loadTime;

		try
		{
			load.open(dbFile);

			string data;
			getline(load, data);

			if(data != "totalNumBlogs") throw dbLoadFailed("DB file corrupted!");
			
			getline(load, data);
			{
				stringstream convert;
				convert << data;
				convert >> totalNumBlogs;
				if(convert.fail()) throw dbLoadFailed("Cannot parse totalNumBlogs: " + data);
			}

			//The size of one tenth of a percent of the DB, used to determine when to output progress messages
			int onePercDB = totalNumBlogs / 100;

			for(blogIndex i = 0; i < totalNumBlogs; i++)
			{
				

				if(i%onePercDB == 0)
				{
					std::cout << "Progress:";
					std::cout << (int)round(double(i) / double(totalNumBlogs) * double(100)) << '%' << endl;
				}

				auto advance = [&load, &data](string dataExpected = "")
				{
					getline(load, data);
					if(data != dataExpected)  progLog::write("Expected + \"" + dataExpected + "\", got \"" + data + "\"!");
				};

				auto get = [&load, &data]()
				{
					getline(load, data);
					return data;
				};

				types::modifiableBlog curr;

				advance("");
				advance("<blog>");
				advance("<URL>");
				curr.URLRef() = get();
				advance("<State>");
				curr.stateRef() = (types::archiveState)(int)conv::toNum(get());
				advance("<numCopies>");
				curr.numCopiesRef() = (int)conv::toNum(get());
				advance("<Archivers>");
				for(int i = 0; i < curr.numCopiesRef(); i++)
				{
					advance("<User>");
					types::archive cArch;

					cArch.who = types::user(get());
					cArch.program = types::archiveProgram(get());
					curr.usersRef().push_back(cArch);
				}
				advance("</blog>");

				getBlogRef(i, true) = blog(curr);
			}

			progLog::write("DB Load complete in " + conv::toString(loadTime.getTime()) + "s.");
		}
		catch(dbLoadFailed e)
		{
			blogArrays = NULL;
			totalNumBlogs = -1;
			progLog::write("<<CRITICAL>> Caught dbLoadFailed: " + e.details);
		}
		catch(...)
		{
			blogArrays = NULL;
			totalNumBlogs = -1;
			progLog::write("<<CRITICAL>> DB LOAD FAILED!");
		};

		dbWrite.unlock();
	}

	void saveDB(string location)
	{
		if(location == "") location = dbFile;

		zlib::timer saveTime;

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

		//The size of one tenth of a percent of the DB, used to determine when to output progress messages
		int onePercDB = totalNumBlogs / 100;

		for(blogIndex i = 0; i < totalNumBlogs; i++)
		{
			if (i%onePercDB == 0)
			{
				std::cout << "Progress:";
				std::cout << (int)round(double(i) / double(totalNumBlogs) * double(100)) << '%' << endl;
			}

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
					append("<User>");
					append(curr.getArchivers()[i].who.getName());
					append(curr.getArchivers()[i].program.program);
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

		progLog::write("Database save complete in " + conv::toString(saveTime.getTime()) + "s.");

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