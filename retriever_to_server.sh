#!/usr/bin/env bash

g++ Retriever.cpp -pthread -o retriever -std=c++11
./retriever uw1-320-07.uwb.edu /index.html 2681