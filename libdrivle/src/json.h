#ifndef JSON_H
#define JSON_H

#define JSON_INVALID 0
#define JSON_OBJECT 1
#define JSON_ARRAY 2
#define JSON_STRING 3
#define JSON_NUMBER 4
#define JSON_TRUE 5
#define JSON_FALSE 6
#define JSON_NULL 7

struct json_field
{
    json_val *key;//must be a string
    json_val *value;
};

struct json_val
{
    int type;
    int length;//used for object or array
    union {
        struct json_field *object;
        struct json_val **array;
        double number;
        char *string;
    };
    
};


extern struct json_val *json_parse(char *start, char *end);

#endif /* JSON_H */
