CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -g -fsanitize=address,undefined

test:
	mkdir -p build
	$(CXX) $(CXXFLAGS) -Istubs -I../firmware/version_ventilateur simulateur.cpp -o build/simulateur
	build/simulateur

.PHONY: test
