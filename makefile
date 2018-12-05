.PHONY: clean	
.PHONY: execute

progname = URL-Distributor.prog
source_files = src/commThreads.cpp src/commThreads.h src/db.cpp src/db.h src/log.cpp src/log.h src/Source.cpp src/types.cpp src/types.h
g_flags = -iquote zlib/../ zlib/*.o -lpthread

all: $(progname)

$(progname): $(source_files)
	cd zlib && $(MAKE)
	g++ $(source_files) -o $(progname) $(g_flags)

clean:
	rm *.prog src/*.o src/*.gch -rdf

execute:
	./$(progname)
