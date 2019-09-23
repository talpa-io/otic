all: otic.so otic.o otic_all.h

otic.so: internal.h otic.h otic.c
	$(CC) -Wall otic.c --shared -fPIC -o otic.so
otic.o: internal.h otic.h otic.c
	$(CC) -Wall -c otic.c -l zstd
otic_all.h: internal.h otic.h otic.c
	python3 make_singleheader.py

clean:
	rm -f otic.so
	rm -f otic.o
	rm -f otic_all.h
gcov:
	rm otic/_internal*.so
	rm otic/_internal.gcno otic/_internal.gcda
	cd otic && LDFLAGS="-fprofile-arcs" CC="gcc -O0 -fprofile-arcs -ftest-coverage" python3 build_cffi.py && cd -
	python3 -m pytest
	cd otic && gcovr --html-details --output out.html && cd -
