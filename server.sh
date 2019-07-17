#!/usr/bin/env bash

g++ Server.cpp -pthread -o server -std=c++11
./server
