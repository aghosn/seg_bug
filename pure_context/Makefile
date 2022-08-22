all: main

main: main.cc
	clang++ -g -pthread $< -o $@ -static

.PHONY: clean

clean:
	rm main
