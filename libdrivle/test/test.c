#include <json-builder.h>
#include <json.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARR_LEN(x) (sizeof(x)/sizeof(*x))

typedef bool (*test_type)(void);

bool test_drive_json(void);
bool test_nested_objects(void);
bool test_parse_json(void);
bool test_parse_naked(void);

test_type tests[] = {
    test_drive_json,
    test_nested_objects,
    test_parse_json,
    test_parse_naked
};

int main(int argc, char **argv)
{
    int test_count, passed;
    int i;
    test_count = ARR_LEN(tests);
    passed = 0;
    for (i = 0; i < test_count; i++) {
        passed += tests[i]()?1:0;
    }
    printf("%d tests; %d passed\n", test_count, passed);
}

bool test_drive_json(void)
{
    struct json_builder json;
    bool success;
    json_builder_init(&json);
    json_builder_add_object(&json);
    json_builder_add_key(&json, "name");
    json_builder_add_string(&json, "file.txt");
    json_builder_add_key(&json, "parents");
    json_builder_add_array(&json);
    json_builder_add_string(&json, "drive");
    json_builder_add_string(&json, "pictures");
    json_builder_end_array(&json);
    json_builder_end_object(&json);

    success = !strcmp(json_builder_get(&json), "{\"name\":\"file.txt\",\"parents\":[\"drive\",\"pictures\"]}");
    if (!success)
        printf("json buffer is: %s\n", json.buffer);
    free(json.buffer);
    return success;
}

bool test_nested_objects(void)
{
    struct json_builder json;
    bool success;
    int i;
    json_builder_init(&json);
    for (i = 0; i < 20; i++) {
        json_builder_add_object(&json);
    }
    for (i = 0; i < 20; i++) {
        json_builder_end_object(&json);
    }
    success = !strcmp(json_builder_get(&json), "{{{{{{{{{{{{{{{{{{{{}}}}}}}}}}}}}}}}}}}}");
    if (!success)
        printf("json buffer is: %s\n", json.buffer);
    free(json.buffer);
    return success;
}

bool test_parse_json(void)
{
    struct json_val *root;
    int start, end;
    char *buf;
    bool success;

    FILE *file;
    file = fopen("drive.res", "r");
    if (!file) {
        printf("Couldn't find drive.res to do test\n");
        return false;
    }
    start = ftell(file);
    fseek(file, 0, SEEK_END);
    end = ftell(file);
    rewind(file);

    buf = malloc(end - start);
    fread(buf, 1, end-start, file);

    fclose(file);


    root = json_parse(buf, buf+end);
    free(buf);

    if (!root) {
        printf("No root value returned\n");
        goto fail;
    }
    if (root->type != JSON_OBJECT) {
        printf("Root object parsed with wrong type\n");
        goto fail;
    }
    if (root->length != 2) {
        printf("Root object has wrong number of members\n");
        goto fail;
    }
    if (!root->object[0].key) {
        printf("First object in root's key is NULL\n");
        goto fail;
    }
    if (root->object[0].key->type != JSON_STRING) {
        printf("First object in root's key is not a string\n");
        goto fail;
    }
    if (strcmp(root->object[0].key->string, "kind")) {
        printf("First object in root's key has wrong value: %s\n", root->object[0].key->string);
        goto fail;
    }
    if (!root->object[0].value) {
        printf("First object in root's value is NULL\n");
        goto fail;
    }
    if (root->object[0].value->type != JSON_STRING) {
        printf("First object in root's value is not a string\n");
        goto fail;
    }
    if (strcmp(root->object[0].value->string, "drive#fileList")) {
        printf("First object in root's value has wrong value: %s\n", root->object[0].key->string);
        goto fail;
    }

    json_free_value(root);
    return true;
fail:
    json_free_value(root);
    return false;
}

bool test_parse_naked(void)
{
    char *buf = "null";
    struct json_val *val = json_parse(buf, buf + sizeof("null")-1);
    if (!val || val->type != JSON_NULL) {
        printf("Failed to parse naked null\n");
        goto fail;
    }
    json_free_value(val);
    buf = "true";
    val = json_parse(buf, buf + sizeof("true")-1);
    if (!val || val->type != JSON_TRUE) {
        printf("Failed to parse naked true\n");
        goto fail;
    }
    json_free_value(val);
    buf = "false";
    val = json_parse(buf, buf + sizeof("false")-1);
    if (!val || val->type != JSON_FALSE) {
        printf("Failed to parse naked false\n");
        goto fail;
    }
    json_free_value(val);
    buf = "{}";
    val = json_parse(buf, buf + sizeof("{}")-1);
    if (!val || val->type != JSON_OBJECT) {
        printf("Failed to parse naked object\n");
        goto fail;
    }
    json_free_value(val);
    buf = "[]";
    val = json_parse(buf, buf + sizeof("[]")-1);
    if (!val || val->type != JSON_ARRAY) {
        printf("Failed to parse naked array\n");
        goto fail;
    }
    json_free_value(val);
    buf = "-123.45e10";
    val = json_parse(buf, buf + sizeof("-123.45e10")-1);
    if (!val || val->type != JSON_NUMBER) {
        printf("Failed to parse naked number ending in exponent\n");
        goto fail;
    }
    json_free_value(val);
    buf = "-123.45";
    val = json_parse(buf, buf + sizeof("-123.45")-1);
    if (!val || val->type != JSON_NUMBER) {
        printf("Failed to parse naked number ending in decimal\n");
        goto fail;
    }
    json_free_value(val);
    buf = "-123";
    val = json_parse(buf, buf + sizeof("-123")-1);
    if (!val || val->type != JSON_NUMBER) {
        printf("Failed to parse naked number ending in whole number\n");
        goto fail;
    }
    json_free_value(val);
    buf = "\"\"";
    val = json_parse(buf, buf + sizeof("\"\"")-1);
    if (!val || val->type != JSON_STRING) {
        printf("Failed to parse naked string\n");
        goto fail;
    }
    json_free_value(val);
    return true;

fail:
    return false;
}
