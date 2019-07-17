#!/usr/bin/env bash

g++ Retriever.cpp -pthread -o retriever -std=c++11
./retriever www.columbia.edu /~fdc/sample.html 80
