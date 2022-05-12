#ifndef UTIL_H
#define UTIL_H

#include <json-c/json.h>

const char *get_key(struct json_object *from, const char *key);
int fmt_string(char *to, size_t size, const char *s, ...);

#endif /* UTIL_H */
