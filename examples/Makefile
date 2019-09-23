all: resources printall

resources: resources.c
	$(CC) -g -Wall resources.c ../src/otic.c -I ../src/ -l zstd -o resources

printall: printall.c
	$(CC) -g -Wall printall.c ../src/otic.c -I ../src/ -l zstd -o printall

clean:
	rm resources printall -f
