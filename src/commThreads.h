#pragma once

#include "zlib/network.h"

namespace commThreads
{
	void openNewConnection(network::childSocket socket);

	void joinIfPossible();



}