#include <stdio.h>
#include <stdlib.h>

static int usage(char *a0) {
	fprintf(stderr, "usage: %s FILE.GBA\nprints gameid of FILE.GBA.\n", a0);
	return 1;
}

int main(int argc, char** argv) {
	if(argc != 2) return usage(argv[0]);
	const char *e = "fopen";
	FILE *f;
	if(!(f = fopen(argv[1], "r"))) {
	err:
		perror(e);
		return 1;
	}
	char buf[19] = {0};
	e = "fseek";
	if(fseeko(f, 0xA0+12, SEEK_SET)) goto err;
	e = "fread";
	if(18-12 != fread(buf, 1, 18-12, f)) goto err;
	fprintf(stdout, "%s\n", buf);
	fclose(f);
	return 0;
}
