all:
	cd build; \
	clang++ -lpthread ../main.cpp -std=c++11 -O0 -g -Wall -o klcp_tool

prod:
	cd build; \
    clang++ -lpthread ../main.cpp -std=c++11 -Ofast -o klcp_tool

intel:
	cd build; \
	icc -lpthread ../main.cpp -std=c++11 -O0 -g -Wall -o klcp_tool

intel_prod:
	cd build; \
    clang++ -lpthread ../main.cpp -std=c++11 -Ofast -o klcp_tool