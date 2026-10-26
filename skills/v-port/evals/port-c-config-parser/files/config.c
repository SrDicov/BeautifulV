#include "config.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

static char *trim(char *s) {
	char *end;
	while (*s && isspace((unsigned char)*s)) {
		s++;
	}
	if (*s == '\0') {
		return s;
	}
	end = s + strlen(s) - 1;
	while (end > s && isspace((unsigned char)*end)) {
		*end = '\0';
		end--;
	}
	return s;
}

static int copy_bounded(char *dst, size_t cap, const char *src) {
	size_t n = strlen(src);
	if (n >= cap) {
		return -1;
	}
	memcpy(dst, src, n + 1);
	return 0;
}

int config_load(const char *path, Config *out) {
	FILE *f;
	char line[512];

	if (out == NULL) {
		return CONFIG_ERR_SYNTAX;
	}
	out->count = 0;
	f = fopen(path, "r");
	if (f == NULL) {
		return CONFIG_ERR_OPEN;
	}
	while (fgets(line, sizeof line, f) != NULL) {
		char *eq;
		char *key;
		char *value;
		int i;
		int replaced = 0;

		line[strcspn(line, "\r\n")] = '\0';
		key = trim(line);
		if (*key == '\0' || *key == '#') {
			continue;
		}
		eq = strchr(key, '=');
		if (eq == NULL) {
			fclose(f);
			return CONFIG_ERR_SYNTAX;
		}
		*eq = '\0';
		value = trim(eq + 1);
		key = trim(key);
		if (*key == '\0') {
			fclose(f);
			return CONFIG_ERR_SYNTAX;
		}
		for (i = 0; i < out->count; i++) {
			if (strcmp(out->entries[i].key, key) == 0) {
				if (copy_bounded(out->entries[i].value, CONFIG_VALUE_MAX, value) != 0) {
					fclose(f);
					return CONFIG_ERR_FULL;
				}
				replaced = 1;
				break;
			}
		}
		if (replaced) {
			continue;
		}
		if (out->count >= CONFIG_MAX_ENTRIES) {
			fclose(f);
			return CONFIG_ERR_FULL;
		}
		if (copy_bounded(out->entries[out->count].key, CONFIG_KEY_MAX, key) != 0
			|| copy_bounded(out->entries[out->count].value, CONFIG_VALUE_MAX, value) != 0) {
			fclose(f);
			return CONFIG_ERR_FULL;
		}
		out->count++;
	}
	fclose(f);
	return CONFIG_OK;
}

const char *config_get(const Config *cfg, const char *key) {
	int i;
	if (cfg == NULL || key == NULL) {
		return NULL;
	}
	for (i = 0; i < cfg->count; i++) {
		if (strcmp(cfg->entries[i].key, key) == 0) {
			return cfg->entries[i].value;
		}
	}
	return NULL;
}
