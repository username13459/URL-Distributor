#pragma once

#include<string>

using std::string;

namespace types
{
	enum class archiveState
	{
		pendingReview,
		needsArchive,
		archived,
	};

	class blog
	{
	public:
		blog() {}
		blog(string URL);

	private:
		
		//The blog's URL
		string URL;

		//The status of the blog (pending review, needing archive, etc.)
		archiveState state;

		//The number of people who've signed on to archive this blog
		int numCopies = 0;

	public:
		//Returns 'URL'
		string getURL() { return URL; }

		//Returns 'state'
		archiveState getState() { return state;}

		void setState(archiveState state) { this->state = state; }

		int getNumCopies() { return numCopies;  }
	}


}