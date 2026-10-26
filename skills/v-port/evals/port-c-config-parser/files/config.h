#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_MAX_ENTRIES 32
#define CONFIG_KEY_MAX 64
#define CONFIG_VALUE_MAX 128

typedef enum {
	CONFIG_OK = 0,
	CONFIG_ERR_OPEN = 1,
	CONFIG_ERR_SYNTAX = 2,
	CONFIG_ERR_FULL = 3
} ConfigError;

typedef struct {
	char key[CONFIG_KEY_MAX];
	char value[CONFIG_VALUE_MAX];
} ConfigEntry;

typedef struct {
	ConfigEntry entries[CONFIG_MAX_ENTRIES];
	int count;
} Config;

/* config_load reads key=value lines. Blank lines and lines starting with # are
 * ignored. Surrounding whitespace is trimmed. A duplicate key replaces the
 * previous value. Returns CONFIG_OK, CONFIG_ERR_OPEN, CONFIG_ERR_SYNTAX, or
 * CONFIG_ERR_FULL. */
int config_load(const char *path, Config *out);

/* config_get returns the value for key, or NULL when the key is absent. */
const char *config_get(const Config *cfg, const char *key);

#endif
