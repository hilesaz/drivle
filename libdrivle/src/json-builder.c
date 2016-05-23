#include <json-builder.h>

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

static int next_size_that_fits(int size, int count)
{
    while (size < count) {
        size *= BUILDER_SCALE_FACTOR;
    }
    return size;
}

static bool has_room_for(struct json_builder *json, int count)
{
    return (json->end - json->write) >= count;
}

static void ensure_room_for(struct json_builder *json, int count)
{
    int offset, size, new_size;
    if (has_room_for(json, count))
        return;

    offset = json->write-json->buffer;
    size = json->end-json->buffer;
    new_size = next_size_that_fits(size, count + offset);

    json->buffer = realloc(json->buffer, new_size);
    json->write = json->buffer + offset;
    json->end = json->buffer + new_size;
}

static bool should_escape_char(char ch)
{
    return ch == '"' || ch == '\\' || ch == '\n' || ch == '\r' || ch == '\f' || ch == '\b' || ch == '\t';
}



static char escape_char(char ch)
{
    if (ch == '\"')
        return '\"';
    if (ch == '\\')
        return '\\';
    if (ch == '\n')
        return 'n';
    if (ch == '\r')
        return 'r';
    if (ch == '\f')
        return 'f';
    if (ch == '\b')
        return 'b';
    if (ch == '\t')
        return 't';
    //wut
    return '\\';
}

static void write_string(struct json_builder *json, const char* str)
{
    int char_count = 0;
    const char* s = str;
    while (*s) {
        char_count++;
        if (should_escape_char(*s))
            char_count++;
        s++;
    }
    ensure_room_for(json, char_count+2);
    *json->write++ = '\"';
    while(*str) {
        if (should_escape_char(*str)) {
            *json->write++ = '\\';
            *json->write++ = escape_char(*str++);
        } else
            *json->write++ = *str++;
    }
    *json->write++ = '\"';
}

static void maybe_add_comma(struct json_builder *json)
{
    if (!json->stack)
        return;
    if (!json->stack->needs_comma) {
        json->stack->needs_comma = true;
        return;
    }

    ensure_room_for(json, 1);
    *json->write++ = ',';
}

void json_builder_init(struct json_builder *json)
{
    json->buffer = malloc(BUILDER_START_BYTES);
    json->write = json->buffer;
    json->end = json->buffer + BUILDER_START_BYTES;
    json->stack = 0;
}

char *json_builder_get(struct json_builder *json)
{
    ensure_room_for(json, 1);
    *(json->write+1) = '\0';
    return json->buffer;
}

void json_builder_add_object(struct json_builder *json)
{
    struct json_obj_stack *old;
    maybe_add_comma(json);
    ensure_room_for(json, 1);
    *json->write++ = '{';
    
    old = json->stack;
    json->stack = malloc(sizeof (*json->stack));
    json->stack->type = TYPE_OBJECT;
    json->stack->needs_comma = false;
    json->stack->prev = old;
}

void json_builder_end_object(struct json_builder *json)
{
    struct json_obj_stack *old_top;
    if (!json->stack || json->stack->type != TYPE_OBJECT)
        return;
    *json->write++ = '}';
    
    old_top = json->stack;
    json->stack = json->stack->prev;
    free(old_top);
}

void json_builder_add_key(struct json_builder *json, const char* key)
{
    if (!json->stack || json->stack->type != TYPE_OBJECT)
        return;

    maybe_add_comma(json);
    write_string(json, key);
    *json->write++ = ':';
    json->stack->needs_comma = false;
}

void json_builder_add_array(struct json_builder *json)
{
    struct json_obj_stack *old;
    maybe_add_comma(json);
    ensure_room_for(json, 1);
    *json->write++ = '[';

    old = json->stack;
    json->stack = malloc(sizeof (*json->stack));
    json->stack->type = TYPE_ARRAY;
    json->stack->needs_comma = false;
    json->stack->prev = old;
}

void json_builder_end_array(struct json_builder *json)
{
    struct json_obj_stack *old_top;
    if (!json->stack || json->stack->type != TYPE_ARRAY)
        return;
    *json->write++ = ']';
    
    old_top = json->stack;
    json->stack = json->stack->prev;
    free(old_top);
}

void json_builder_add_string(struct json_builder *json, const char* str)
{
    maybe_add_comma(json);
    write_string(json, str);
}

void json_builder_add_number(struct json_builder *json, double value)
{
    if (!isfinite(value)) {
        json_builder_add_null(json);
        return;
    }
    maybe_add_comma(json);
    ensure_room_for(json, MAX_NUMBER_SIZE);
    json->write += sprintf(json->write, "%g.18", value);
}

void copy(char* dst, const char* src)
{
    while (*src)
        *dst++ = *src++;
}

void json_builder_add_true(struct json_builder *json)
{
    maybe_add_comma(json);
    ensure_room_for(json, 4);
    copy(json->write, "true");
    json->write += 4;
}

void json_builder_add_false(struct json_builder *json)
{
    maybe_add_comma(json);
    ensure_room_for(json, 5);
    copy(json->write, "false");
    json->write += 5;
}

void json_builder_add_null(struct json_builder *json)
{
    maybe_add_comma(json);
    ensure_room_for(json, 4);
    copy(json->write, "null");
    json->write += 4;
}
