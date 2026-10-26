#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void write_file(const char *path, const char *text) {
	FILE *f = fopen(path, "w");
	assert(f != NULL);
	fputs(text, f);
	fclose(f);
}

static void test_loads_values(void) {
	char path[256];
	Config cfg;
	snprintf(path, sizeof path, "/tmp/config_test_%d.conf", (int)getpid());
	write_file(path,
		"# comment\n"
		"\n"
		" host = localhost \n"
		"port=8080\n"
		"host=example.com\n"
		"query=a=b\n");
	assert(config_load(path, &cfg) == CONFIG_OK);
	assert(cfg.count == 3);
	assert(strcmp(config_get(&cfg, "host"), "example.com") == 0);
	assert(strcmp(config_get(&cfg, "port"), "8080") == 0);
	assert(strcmp(config_get(&cfg, "query"), "a=b") == 0);
	remove(path);
}

static void test_missing_key(void) {
	Config cfg = {0};
	assert(config_get(&cfg, "nope") == NULL);
}

static void test_missing_file(void) {
	Config cfg;
	assert(config_load("/tmp/config_does_not_exist_12345.conf", &cfg) == CONFIG_ERR_OPEN);
}

static void test_syntax_error(void) {
	char path[256];
	Config cfg;
	snprintf(path, sizeof path, "/tmp/config_test_%d.conf", (int)getpid());
	write_file(path, "valid=1\nbroken line\n");
	assert(config_load(path, &cfg) == CONFIG_ERR_SYNTAX);
	remove(path);
}

static void test_full(void) {
	char path[256];
	char text[4096];
	Config cfg;
	int i;
	size_t used = 0;
	snprintf(path, sizeof path, "/tmp/config_test_%d.conf", (int)getpid());
	for (i = 0; i < CONFIG_MAX_ENTRIES; i++) {
		used += (size_t)snprintf(text + used, sizeof text - used, "k%d=%d\n", i, i);
	}
	write_file(path, text);
	assert(config_load(path, &cfg) == CONFIG_OK);
	assert(cfg.count == CONFIG_MAX_ENTRIES);
	write_file(path, "one=1\ntwo=2\nthree=3\n");
	used = 0;
	for (i = 0; i < CONFIG_MAX_ENTRIES + 1; i++) {
		used += (size_t)snprintf(text + used, sizeof text - used, "k%d=%d\n", i, i);
	}
	write_file(path, text);
	assert(config_load(path, &cfg) == CONFIG_ERR_FULL);
	remove(path);
}

int main(void) {
	test_loads_values();
	test_missing_key();
	test_missing_file();
	test_syntax_error();
	test_full();
	printf("all config tests passed\n");
	return 0;
}
