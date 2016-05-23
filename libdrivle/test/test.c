#include <json-builder.h>

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define ARR_LEN(x) (sizeof(x)/sizeof(*x))

typedef bool (*test_type)(void);

bool test_drive_json(void);
bool test_nested_objects(void);

test_type tests[] = {
    test_drive_json,
    test_nested_objects
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

    success = !strcmp(json.buffer, "{\"name\":\"file.txt\",\"parents\":[\"drive\",\"pictures\"]}");
    if (!success)
        printf("json buffer is: %s\n", json.buffer);
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
    return success;
}
