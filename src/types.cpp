

#include "types.h"
#include "db.h"

namespace types
{
	//Provides human-readable versions of archiveState.   NEEDS TO BE KEPT IN SYNC!
	static const string archiveStateStrings[] = { "pendingReview", "needsArchive", "archived", "reject" };

	//Converts the archiveState enum to a human-readable string
	string archiveStateToString(types::archiveState state)
	{
		return types::archiveStateStrings[(int)state];
	}

	blog::blog(string URL)
	{
		this->URL = URL;
		if (URL == "") throw blogExceptionBadURL("No URL given");
		//TODO: Preform validation to make sure the link is formatted correctly in the future
	}

	void blog::addDownloader(archive _archive)
	{
		archivers.push_back(_archive);
		numCopies++;
		state = archiveState::archived;
	}



}