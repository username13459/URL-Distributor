

#include "types.h"

namespace types
{
	//Provides human-readable versions of archiveState.   NEEDS TO BE KEPT IN SYNC!
	static const string archiveStateStrings[] = { "pendingReview", "needsArchive", "archived" };

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

	void blog::addDownloader(user _user)
	{
		archivers.push_back(_user);
		numCopies++;
		state = archiveState::archived;
	}



}