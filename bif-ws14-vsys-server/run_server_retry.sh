#!/bin/bash
while true
do	
	make
	cd build
	./klcp_tool server localhost 6543 ../../ServerFiles
	sleep 1
	cd ..
done
