#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stddef.h>

static int align = 4;

static int usage(char *a0) {
	printf(
		"usage: %s [-a ALIGN] FILE\n"
		"truncates the trailing 0xff padding of FILE at the"
		"first ALIGN (default: 4) byte-aligned offset\n", a0);
	return 1;
}

int main(int argc, char **argv) {
	if (argc != 2 && argc != 4) return usage(argv[0]);
	int a = 1;
	if(argc == 4) {
		if(argv[a][0] != '-' || argv[a][1] != 'a' || argv[a][2] != 0)
			return usage(argv[0]);
		++a;
		align = atoi(argv[a]);
		++a;
	}
	const char *e = "fopen";
	FILE *f = fopen(argv[a], "r+b");
	if(!f) {
	pe:
		perror(e);
		return 1;
	}
	e = "fseeko";
	if(fseeko(f, 0, SEEK_END)) goto pe;
	off_t fs = ftello(f);
	if(fseeko(f, 0, SEEK_SET)) goto pe;
	unsigned char *data = malloc(fs);
	assert(!(((uintptr_t)data) & (align-1)));
	if(!data) {
		fprintf(stderr, "out of memory\n");
		return 1;
	}
	e = "fread";
	if(fs != fread(data, 1, fs, f)) goto pe;
	unsigned char *p = data+fs-1;
	while(p > data && *p == 0xff) --p;
	if(p > data && p < data+fs-1) ++p;
	while( (((uintptr_t)p) & (align-1)) && p < data+fs) ++p;
	fs = (ptrdiff_t)p - (ptrdiff_t)data;
	e = "ftruncate";
	if(ftruncate(fileno(f), fs)) goto pe;
	fclose(f);
	return 0;
}
