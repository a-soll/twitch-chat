#include "util.h"
#include <stdarg.h>
#include <stdio.h>

const char *get_key(struct json_object *from, const char *key) {
    struct json_object *val;

    json_object_object_get_ex(from, key, &val);

    return json_object_get_string(val);
}

int fmt_string(char *to, size_t size, const char *s, ...) {
    va_list ap;
    int ret;

    va_start(ap, s);
    ret = vsnprintf(to, size, s, ap);
    va_end(ap);

    return ret;
}
