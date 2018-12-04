#include<iostream>

#include "../zlib/varConv.h"

using namespace zlib::conv;

#include "types.h"
#include "db.h"

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

int main(int argc, char * argv[])
{
	blogDB::loadDB();

	cout << "blog size:" << sizeof(types::blog) << endl;

	printIndexPair(blogDB::getIndexPair(0));
	printIndexPair(blogDB::getIndexPair(blogDB::maxArraySize));
	printIndexPair(blogDB::getIndexPair(blogDB::maxArraySize + 1));
	printIndexPair(blogDB::getIndexPair(blogDB::maxArraySize * 5 + 2));

	cin.get();
	return 0;
}