#ifndef JSON_BUILDER_H
#define JSON_BUILDER_H

#include <stdbool.h>

#define BUILDER_START_BYTES ((1<<4))
#define BUILDER_SCALE_FACTOR 2

//2 signs, 1 leading digit, 1 e, 1 period, 3 exponents, 18 mantissa
#define MAX_NUMBER_SIZE 26

#define TYPE_NONE 0
#define TYPE_ARRAY 1
#define TYPE_OBJECT 2

struct json_obj_stack;

struct json_obj_stack {
    int type;
    bool needs_comma;
    struct json_obj_stack *prev;
};

struct json_builder {
    char *buffer;
    char *write;
    char *end;
    struct json_obj_stack *stack;
};


extern void json_builder_init(struct json_builder *json);
extern char *json_builder_get(struct json_builder *json);
extern void json_builder_add_object(struct json_builder *json);
extern void json_builder_end_object(struct json_builder *json);
extern void json_builder_add_key(struct json_builder *json, const char* key);
extern void json_builder_add_array(struct json_builder *json);
extern void json_builder_end_array(struct json_builder *json);
extern void json_builder_add_string(struct json_builder *json, const char* str);
extern void json_builder_add_number(struct json_builder *json, double value);
extern void json_builder_add_true(struct json_builder *json);
extern void json_builder_add_false(struct json_builder *json);
extern void json_builder_add_null(struct json_builder *json);

#endif /* JSON_BUILDER_H */
