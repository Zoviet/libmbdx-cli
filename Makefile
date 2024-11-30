all: mdbx
mdbx: mdbx.c 
	gcc mdbx.c -I /usr/include/libmdbx -L /usr/include/libmdbx -Wl,-R/usr/include/libmdbx -lmdbx -lm -o mdbx
