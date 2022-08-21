all: main

main: main.cc
	clang++ -g -pthread $< -o $@

.PHONY: clean

clean:
	rm main
