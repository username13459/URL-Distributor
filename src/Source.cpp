#include<iostream>

#include "zlib/varConv.h"
#include "zlib/network.h"

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

	progLog::write("Initial load complete");
}

void writeAndCloseEverything()
{
	blogDB::saveDB();
	var::longTime now = var::longTime::now();
	blogDB::saveDB("URL DB BACKUP" + now.getYMD() + "-" + now.getHMS() + ".txt.db");
	progLog::closeLog();
}

void openListenSocket()
{

}

int main(int argc, char * argv[])
{
	loadEverything();

	for(blogIndex i = 0; i < blogDB::getSize(); i++) printStatus(blogDB::getBlog(i));



	writeAndCloseEverything();

#ifdef _WIN32
	cout << "Press any key to close." << endl;
	cin.get();
#endif
	return 0;
}