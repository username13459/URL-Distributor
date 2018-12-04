#include<iostream>

#include "../zlib/varConv.h"

using namespace zlib::conv;

#include "types.h"
#include "db.h"
#include "log.h"

using std::cin;
using std::cout;
using std::endl;


void printStatus(types::blog b)
{
	cout << "Blog: " << b.getURL() << endl;
	cout << "State: " << types::archiveStateToString(b.getState()) << endl;
	cout << "Num. backups: " << b.getNumCopies() << endl;
	for (unsigned i = 0; i < b.getArchivers().size(); i++)
	{
		cout << "Archiver " << i << ":" << b.getArchivers()[i].getName() << endl;
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
}

void writeAndCloseEverything()
{
	blogDB::saveDB();
	var::longTime now = var::longTime::now();
	blogDB::saveDB("URL DB BACKUP" + now.getYMD() + "-" + now.getHMS() + ".txt.db");
	progLog::closeLog();
}

int main(int argc, char * argv[])
{
	loadEverything();

	blogDB::append(blog("blog1.tumblr.com"));
	blogDB::append(blog("blog2.tumblr.com"));
	blogDB::append(blog("blog3.tumblr.com"));
	blogDB::append(blog("blog4.tumblr.com"));
	blogDB::append(blog("blog5.tumblr.com"));

	blogDB::getBlogsToArchive(2, types::user("TESTING#TESTING"));

	blogDB::sortDB();

	blogDB::getBlogsToArchive(1, types::user("TESTING2####"));
	blogDB::getBlogsToArchive(3, types::user("TESTING3####"));

	blogDB::sortDB();

	blogDB::getBlogsToArchive(7, types::user("TESTING4####"));

	for(int i = 0; i < 5; i++)
	{
		printStatus(blogDB::getBlog(i));
	}

	writeAndCloseEverything();

	cin.get();
	return 0;
}