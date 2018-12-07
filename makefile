.PHONY: clean	
.PHONY: execute

progname = URL-Distributor.prog
source_files = src/commThreads.cpp src/commThreads.h src/db.cpp src/db.h src/log.cpp src/log.h src/Source.cpp src/types.cpp src/types.h src/URLtypes.cpp src/URLtypes.h src/util.h src/util.cpp
g_flags = -iquote zlib/../ zlib/*.o -lpthread

debugFlag = debugIndicator
zlibDebug = zlib/debug_made.txt

all: $(progname)

$(progname): $(source_files)
	cd zlib && $(MAKE)
	g++ $(source_files) -o $(progname) $(g_flags)


clean:
	rm $(debugFlag) -f
	rm *.prog src/*.o src/*.gch -rdf
	cd zlib && $(MAKE) clean

execute:
	./$(progname)

debug: g_flags += -g
debug: $(zlibDebug)  $(debugFlag) all
	@echo compiled in debug mode > $(debugFlag)

$(zlibDebug): 
	cd zlib && $(MAKE) debug
	
$(debugFlag): all
