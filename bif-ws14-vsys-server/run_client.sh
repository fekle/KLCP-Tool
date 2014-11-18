#!/bin/bash

cd build
cp klcp_tool vsys-client
./vsys-client client localhost 6543 ../../ClientFiles
