#pragma once

#include<string>
#include<vector>

using std::string;

namespace types
{
	typedef long int blogIndex;
	typedef std::pair<int, int> blogIndexPair;

	struct Exception
	{
		Exception() {}
		Exception(string details) { this->details = details; }

		string details;
	};
	

	//Stores information for a single user, 
	class user
	{
	public:
		user() {}
		user(string discordTag) { this->discordTag = discordTag; }

	private:
		string discordTag = "UNDEF";

	public:
		string getName() { return discordTag; }
	};

	struct archiveProgram
	{
		archiveProgram() { }

		archiveProgram(string program)
		{
			this->program = program;
		}

		string program = "UNDEF";
	};



	struct archive
	{
		archive() {}
		archive(user _user, archiveProgram program)
		{
			this->who = _user;
			this->program = program;
		};

		user who;
		archiveProgram program;
	};

	enum class archiveState
	{
		pendingReview,
		needsArchive,
		archived,
		reject,
	};

	
	//Converts the archiveState enum to a human-readable string
	string archiveStateToString(types::archiveState state);
	

	struct blogException : Exception
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

	protected:
		
		//The blog's URL
		string URL = "";

		//The status of the blog (pending review, needing archive, etc.)
		archiveState state = archiveState::pendingReview;

		//The number of people who've signed on to archive this blog
		int numCopies = 0;

		std::vector<archive> archivers;

	public:
		//Returns 'URL'
		string getURL() { return URL; }

		//Returns 'state'
		archiveState getState() { return state;}

		//Returns 'numCopies'
		int getNumCopies() { return numCopies;  }

		std::vector<archive> getArchivers() { return archivers; }

		//Sets the state flag to 'needsArchive' and 'reject', respectively
		void approve() { if(state != archiveState::archived) state = archiveState::needsArchive; }
		void reject() { state = archiveState::reject; }

		//>>Adds '_user' to the list of people who are archiving the blog
		//@ Modifies: 'archivers', 'numCopies'
		//@ Modifies: 'state' *IF* state != archived
		void addDownloader(archive _archive);

	};

	class modifiableBlog : public blog
	{
	public:
		string & URLRef() { return URL; }
		archiveState & stateRef() { return state; }
		int & numCopiesRef() { return numCopies; }
		std::vector<archive> & usersRef() { return archivers; }
	};
}