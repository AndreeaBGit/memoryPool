#!/bin/bash

GTEST_INCLUDE= # google test include path
GTEST_LIB= # google test library path

g++ --std=c++11 -isystem $GTEST_INCLUDE test.cpp -pthread $GTEST_LIB/libgtest_main.a $GTEST_LIB/libgtest.a -o memPoolTest

