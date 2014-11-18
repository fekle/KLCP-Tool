#!/bin/bash

cd build
cp klcp_tool vsys-client
./vsys-client client 178.62.251.157 6543 ../../ClientFiles
