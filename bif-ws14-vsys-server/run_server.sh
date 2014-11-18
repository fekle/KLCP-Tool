#!/bin/bash

cd build
cp klcp_tool vsys-server
./vsys-server server localhost 6543 ../../ServerFiles
