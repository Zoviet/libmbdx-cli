CFLAGS=-c -Wall -I$(INC_DIR)
WARNINGS="-Wall -Wno-trigraphs -Wmissing-field-initializers -Wreturn-type -Wmissing-braces -Wparentheses -Wno-switch -Wunused-function -Wunused-label -Wunused-parameter -Wunused-variable -Wunused-value -Wuninitialized -Wunknown-pragmas -Wshadow -Wsign-compare" \
MDBX_COVERAGE="$(MDBX_COVERAGE)" \
LIBS="/usr/include/libmdbx/libmdbx.a -pthread" \
CC="$(CC)" \
CFLAGS="$(CFLAGS)" \

all: mdbx.so
mdbx.so: mdbx.c 
	gcc -I /usr/include/libmdbx -l$(LIBS) -o mdbx.so mdbx.c -I$(INC_DIR) -Wall -shared -fPIC
