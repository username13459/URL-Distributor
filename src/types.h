#pragma once

#include<string>
#include<vector>

using std::string;

namespace types
{
	//Stores information for a single user, 
	class user
	{
	public:
		user(string discordTag) { this->discordTag = discordTag; }

	private:
		string discordTag;

	public:
		string getName() { return discordTag; }
	};

	enum class archiveState
	{
		pendingReview,
		needsArchive,
		archived,
	};

	
	//Converts the archiveState enum to a human-readable string
	string archiveStateToString(types::archiveState state);
	

	struct blogException
	{
		blogException() {}
		blogException(string details) { this->details = details; }
		string details;
	};

	struct blogExceptionBadURL : blogException {
		blogExceptionBadURL() {}
		blogExceptionBadURL(string details) { this->details = details;  }
	};

	class blog
	{
	public:
		blog() {}
		blog(string URL);

	private:
		
		//The blog's URL
		string URL = "";

		//The status of the blog (pending review, needing archive, etc.)
		archiveState state = archiveState::pendingReview;

		//The number of people who've signed on to archive this blog
		int numCopies = 0;

		std::vector<user> archivers;

	public:
		//Returns 'URL'
		string getURL() { return URL; }

		//Returns 'state'
		archiveState getState() { return state;}

		//Returns 'numCopies'
		int getNumCopies() { return numCopies;  }

		std::vector<user> getArchivers() { return archivers; }


		//>>Adds '_user' to the list of people who are archiving the blog
		//@ Modifies: 'archivers', 'numCopies'
		//@ Modifies: 'state' *IF* state != archived
		void addDownloader(user _user);

	};


}