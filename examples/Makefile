all: resources printall

resources: resources.c
	$(CC) -g -O3 -Wall resources.c ../src/otic.c -I ../src/ -l zstd -o resources

printall: printall.c
	$(CC) -g -O3 -Wall printall.c ../src/otic.c -I ../src/ -l zstd -o printall

clean:
	rm resources printall -f
