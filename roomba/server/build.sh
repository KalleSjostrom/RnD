#!/bin/bash
source ../../common.sh

clang $debug $common -I../../ mock_server.cpp -o mock_server

