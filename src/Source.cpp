#include <iostream>
#include <fstream>
#include <string>
#include <mutex>

#include "zlib/varConv.h"
#include "zlib/input.h"

using namespace zlib::conv;

#include "URLtypes.h"
#include "util.h"
#include "types.h"
#include "db.h"
#include "log.h"

using std::cin;
using std::cout;
using std::endl;

using std::string;

void printStatus(types::blog b)
{
	cout << "Blog: " << b.getURL() << endl;
	cout << "State: " << types::archiveStateToString(b.getState()) << endl;
	cout << "Num. backups: " << b.getNumCopies() << endl;
	for (unsigned i = 0; i < b.getArchivers().size(); i++)
	{
		cout << "Archiver " << i << ":" << b.getArchivers()[i].who.getName() << endl;
		cout << "Archiver " << i << ": (program)" << b.getArchivers()[i].program.program << endl;
	}
	cout << endl;
}

void printIndexPair(blogIndexPair pair)
{
	std::cout << pair.first << ", " << pair.second << std::endl;
}

void loadEverything()
{
	progLog::startLog();
	blogDB::loadDB();

	progLog::write("Initial load complete");
}

void writeAndCloseEverything()
{
	blogDB::saveDB();
	var::longTime now = var::longTime::now();
	blogDB::saveDB("URL DB BACKUP" + now.getYMD() + "-" + now.getHMS() + ".txt.db");
	progLog::closeLog();
}

void writeToRejects(string data)
{
	static std::mutex fileLock;
	fileLock.lock();
	std::ofstream out;
	out.open("rejects.txt", ios_base::app);
	if (!out.good())
	{
		cout << "<<CRITICAL>> unable to open rejects.txt!" << endl;
	}
	else
	{
		out << data << endl;
	}
	out.close();
	fileLock.unlock();
}

void ingestLine(string data, bool ignoreChecks = false)
{
	util::trimSpaces(data);

	types::t_URL url;

	try
	{
		url = types::t_URL(data);
	}
	catch (...)
	{
		cout << "Unable to construct a URL from \"" << data << "\".  Writing to rejects.txt" << endl;
		writeToRejects(data);
	}

	if (url.getDomains().size() < 2)
	{
		cout << "Rejecting URL: " << data << endl;
		writeToRejects(data);
		return;
	}
	//Check for X.tumblr.com
	else if (url.getDomains().size() >= 3 && url.getDomain(0) == "com" && url.getDomain(1) == "tumblr" && url.getDomain(2) != "")
	{
		string shortURL = url.getDomain(2) + "." + url.getDomain(1) + "." + url.getDomain(0);
		cout << "Standard tumblr blog found: " << shortURL << endl;
		blogDB::append(shortURL, ignoreChecks);
		return;
	}
	else
	{
		cout << "Unrecognized URL - \"" << data << "\"." << endl;
		if (ignoreChecks)
		{
			cout << "Overridden - accepting." << endl;
			blogDB::append(data, ignoreChecks);
		}
		else
		{
			cout << "Approve or deny? <Y/N>?";
			char resp;
			do
			{
				resp = cin.get();
				cin.get();	//Drop the \n
				if (resp != 'Y' && resp != 'N') cout << "Please enter Y or N" << endl << ">>";
			} while (resp != 'Y' && resp != 'N');
			if (resp == 'Y') blogDB::append(data, ignoreChecks);
			else return;
		}
	}
}

void ingestFile(string filename, bool ignoreChecks = false)
{
	std::ifstream in;
	in.open(filename);

	if (ignoreChecks) progLog::write("<WARN> Duplicate checking is disabled!");

	if (!in.good())
	{
		cout << "Unable to open \"" << filename << "\"!" << endl;
		return;
	}

	do
	{
		string data;
		std::getline(in, data);
		
		ingestLine(data, ignoreChecks);
		

	} while (!in.eof());
}

int main(int argc, char * argv[])
{
	loadEverything();

	/*for (int i = 0; i < 5; i++)
	{
		blogDB::append(blog("blog" + conv::toString(i) + ".tumblr.com"));
	}*/

	//blogDB::append(blog("blog1.tumblr.com"));

	//blogDB::getBlogsToArchive(3, types::archive(types::user("test1"), types::archiveProgram("aArchiveProgram1")));
	//blogDB::getBlogsToArchive(4, types::archive(types::user("test1"), types::archiveProgram("ArchiveProgram2")));

	bool run = true;

	try
	{

		while (run)
		{
			cout << ">>";
			string input;

			auto beginsWith = [&input](string check)
			{
				if (input.length() < check.length()) return false;
				return(input.substr(0, check.length()) == check);
			};

			input = input::getLine();
			vector<string> commands = util::splitBySpace(input);
			
			if (commands.size() == 0)
			{
				//No command, do nothing
			}
			else if (commands[0] == "print")
			{
				//Print the entire chonkin' db
				if (commands.size() == 1)
				{
					cout << "There are " << blogDB::getSize() << " unique blogs in the DB.  Please select one." << endl;
				}
				else if (commands.size() == 2)
				{
					try
					{
						int loc = conv::toNum(commands[1]);
						printStatus(blogDB::getBlog(loc));
					}
					catch (...)
					{
						cout << "Unable to parse \"" << commands[1] << "\" as a number.  Abort." << endl; 
						continue;
					}
				}
				else
				{
					try
					{
						int first = conv::toNum(commands[1]);
						int second = conv::toNum(commands[2]);
						printStatus(blogDB::getBlog(blogIndexPair(first, second)));
					}
					catch (...)
					{

						cout << "Unable to parse \"" << commands[1] << " " << commands[2] << "\" as index pairs.  Abort." << endl;
					}
				}
			}
			else if (commands[0] == "get")
			{
				string prog;
				string user; 
				int count;

				if (commands.size() < 4)
				{
					cout << "Please enter the archive program: ";
					prog = input::getLine();
				}
				else prog = commands[3];

				if (commands.size() < 3)
				{
					cout << "Please enter archivers' discord tag: "; 
					user = input::getLine();
				}
				else user = commands[2];

				if (commands.size() < 2)
				{
					cout << "Please enter the number of blogs to retreieve: ";
					try
					{
						count = conv::toNum(input::getLine());
					}
					catch (...)
					{
						cout << "Unable to convert to number.  Abort." << endl;
						continue;
					}
				}
				else
				{
					try
					{
						count = conv::toNum(commands[1]);
					}
					catch (...)
					{
						cout << "Unable to convert to number.  Abort." << endl;
						continue;
					}
				}

				cout << "Num. URLs: " << count << endl;
				cout << "User name: " << user << endl;
				cout << "program  : " << prog << endl;
				cout << endl;
				cout << "Is this correct? <Y/N>?" << endl;
				char resp;
				do
				{
					resp = cin.get();
				} while (resp != 'Y' && resp != 'N');
				if (resp == 'N')
				{
					cout << "Aborted." << endl; 
					continue;
				}
				else
				{
					vector<blog> blogs = blogDB::getBlogsToArchive(count, types::archive(types::user(user), types::archiveProgram(prog)));
					
					static int outCounter = 0;

					outCounter++;

					string fOut = "URLsOut";
					if (outCounter < 10) fOut += "00";
					else if (outCounter < 100) fOut += "0";
					fOut += conv::toString(outCounter);
					fOut += ".txt";

					std::ofstream out;
					out.open(fOut);

					if (!out.good())
					{
						progLog::write("Unable to open file to write export URLs to!");
						out.close();
						continue;
					}
					else
					{
						out << blogs.size() << endl;
						for (int i = 0; i < blogs.size(); i++)
						{
							out << blogs[i].getURL() << endl;
						}
					}

					out.close();

					cout << blogs.size() << " URLs written to " << fOut << endl;
				}

			}
			else if (commands[0] == "find")
			{
				if (commands.size() < 2) cout << "Please enter a blog URL to search for." << endl;
				else
				{
					blogIndexPair loc = blogDB::getBlogByURL(commands[1]);

					if (loc == blogDB::npos) cout << "Unable to locate \"" << commands[1] << "\" within the DB." << endl;
					else
					{
						cout << "Found \"" << commands[1] << " at " << loc.first << ", " << loc.second << "\" in the DB." << endl;
					}
				}
			}
			else if (commands[0] == "review")
			{
				//Make sure all arguments are present
				if (commands.size() < 3)
				{
					cout << "Syntax: \"review <OK|BAD> <blog>" << endl;
					continue;
				}
				
				types::archiveState set;	//The state being set for the archives
				if (commands[1] == "BAD") set = types::archiveState::reject;
				else if(commands[1] == "OK" || commands[1] == "GOOD") set = types::archiveState::needsArchive;
				else
				{
					cout << "Please enter either \"OK\" or \"BAD\", not \"" << commands[1] << "\"." << endl;
					continue;
				}

				if (commands[2] == "--file")
				{
					if (commands.size() < 4)
					{
						cout << "Please specify a file to pull from" << endl;
						continue;
					}
					else
					{
						std::ifstream in;
						in.open(commands[3]);

						if (!in.good())
						{
							cout << "Unable to open file \"" << commands[3] << "\"." << endl;
							continue;
						}

						do
						{
							string data;
							std::getline(in, data);

							blogIndexPair loc = blogDB::getBlogByURL(data);

							if (loc == blogDB::npos)
							{
								cout << "Could not find blog \"" << data << "\"!" << endl;
								continue;
							}

							if (set == types::archiveState::needsArchive)
							{
								blogDB::approveBlog(loc);
								cout << "Blog \"" << data << "\" flagged as good!" << endl;
							}
							else if (set == types::archiveState::reject)
							{
								blogDB::rejectBlog(loc);
								cout << "Blog \"" << data << "\"  flagged as rejected." << endl;
							}

						} while (!in.eof());

					}

				}
				else
				{
					blogIndexPair loc = blogDB::getBlogByURL(commands[2]);

					if (loc == blogDB::npos)
					{
						cout << "Could not find blog \"" << commands[2] << "\"!" << endl;
						continue;
					}

					if (set == types::archiveState::needsArchive)
					{
						blogDB::approveBlog(loc);
						cout << "Blog flagged as good!" << endl;
					}
					else if (set == types::archiveState::reject)
					{
						blogDB::rejectBlog(loc);
						cout << "Blog flagged as rejected." << endl;
					}
				}

			}
			else if (commands[0] == "add")
			{
				if (commands.size() < 2)
				{
					cout << "Usage: add <blog>" << endl;
					continue;
				}
				else
				{
					ingestLine(commands[1]);
				}
			}
			else if (commands[0] == ("ingest"))
			{
				if (commands.size() < 2)
				{
					cout << "Please include a filename." << endl;
				}
				else
				{
					bool noCheck = false;
					if (commands.size() >= 3) if (commands[2] == "noCheck") noCheck = true;

					ingestFile(commands[1], noCheck);

					cout << "Ingest complete." << endl;
				}
			}
			else if (input == "Save")
			{
				blogDB::saveDB();
				var::longTime now = var::longTime::now();
				blogDB::saveDB("URL DB BACKUP" + now.getYMD() + "-" + now.getHMS() + ".txt.db");
			}
			else if (input == "Quit")
			{
				run = false;
			}
			else
			{
				cout << "Command unrecognized!" << endl;
			}




		}

	}
	catch (...)
	{
		progLog::write("<<CRITICIAL>> Unknown exception in program!  Exiting and saving DB...");
	}
	writeAndCloseEverything();

#ifdef _WIN32
	cout << "Press any key to close." << endl;
	string tmp_garbage;
	std::getline(cin, tmp_garbage);
#endif
	return 0;
}