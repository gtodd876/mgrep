#!/usr/bin/env bash
# Orthodox C++?
clang++ -std=c++20 -Wall -Wextra -Wpedantic -Wconversion -g \
  -fno-exceptions -fno-rtti \
  -fsanitize=address,undefined \
  -o mgrep mgrep.cpp

exec ./mgrep
