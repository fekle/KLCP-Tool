#
#	VSYS Makefile
#	(c) 2014 Aleksandar Lepojic & Felix Klein
#

EXEC            =   klcp_tool
NAME            =   VSYS
VERSION		    =   ` date +"%d-%m-%Y_%H-%M-%S"`
ARCHIVE		    =   ${NAME}-Lepojic_Klein-${VERSION}

DEBUGFLAGS      =   -march=native -std=gnu++11 -O0 -g -Wall -Wno-deprecated-declarations
EXTRADEBUGFLAGS =   -march=native -std=c++11 -O0 -g -Weverything -Wno-deprecated-declarations
PRODFLAGS       =   -march=native -std=gnu++11 -Ofast -Wno-deprecated-declarations
LDFLAGS         =   -lpthread -DLDAP_DEPRECATED -lldap -llber

all: debug

debug: clean
	cd build; \
	clang++ ../main.cpp $(DEBUGFLAGS) $(LDFLAGS) -o $(EXEC)

prod: clean
	cd build; \
	clang++ ../main.cpp $(PRODFLAGS) $(LDFLAGS) -o $(EXEC)

extra_debug: clean
	cd build; \
	clang++ ../main.cpp $(EXTRADEBUGFLAGS) $(LDFLAGS) -o $(EXEC)

intel: clean
	cd build; \
	icc ../main.cpp $(DEBUGFLAGS) $(LDFLAGS) -o $(EXEC)

intel_prod: clean
	cd build; \
	icc ../main.cpp $(PRODFLAGS) $(LDFLAGS) -o $(EXEC)

dist: prod
	tar cfvz dist/${ARCHIVE}.tgz *.cpp *.hpp Makefile *.sh build doc

clean:
	rm -rf *.o

	if [ ! -d build ]; \
	then \
		mkdir build; \
	else \
		rm -rf build; \
		mkdir build; \
	fi

	if [ ! -d dist ]; \
	then \
		mkdir dist; \
	else \
		rm -rf dist; \
		mkdir dist; \
	fi
