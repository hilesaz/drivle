#include <json.h>
#include <iostate.h>

#include <stdbool.h>




struct json_val *json_parse_true(struct read_state *state);
struct json_val *json_parse_false(struct read_state *state);
struct json_val *json_parse_null(struct read_state *state);
struct json_val *json_parse_number(struct read_state *state);
struct json_val *json_parse_string(struct read_state *state);
struct json_val *json_parse_object(struct read_state *state);
struct json_val *json_parse_array(struct read_state *state);
struct json_val *json_parse_value(struct read_state *state);

struct json_val *json_parse_true(struct read_state *state)
{
    struct json_val *value;
    value = malloc(sizeof(*value));
    if (state->end - state->read < 3) {
        printf("Unexpected end of file parsing true literal\n");
        goto fail;
    }
    if (state->read[0] == 'r' && state->read[1] == 'u' && state->read[2] == 'e') {
        state->read += 3;
        value.type = JSON_TRUE;
    } else {
        printf("Unexpected character in true literal\n");
        goto fail;
    }
    return value;

fail:
    free(value);
    return NULL;
}
struct json_val *json_parse_false(struct read_state *state)
{
    struct json_val *value;
    value = malloc(sizeof(*value));
    if (state->end - state->read < 4) {
        printf("Unexpected end of file parsing false literal\n");
        goto fail;
    }
    if (state->read[0] == 'a' && state->read[1] == 'l' && state->read[2] == 's' && state->read[3] == 'e') {
        state->read += 4;
        value.type = JSON_FALSE;
    } else {
        printf("Unexpected character in false literal\n");
        goto fail;
    }
    return value;

fail:
    free(value);
    return NULL;
}
struct json_val *json_parse_null(struct read_state *state)
{
    struct json_val *value;
    value = malloc(sizeof(*value));
    if (state->end - state->read < 3) {
        printf("Unexpected end of file parsing null literal\n");
        goto fail;
    }
    if (state->read[0] == 'u' && state->read[1] == 'l' && state->read[2] == 'l') {
        state->read += 3;
        value.type = JSON_NULL;
    } else {
        printf("Unexpected character in null literal\n");
        goto fail;
    }
    return value;

fail:
    free(value);
    return NULL;
}

static struct json_field json_parse_object_field(struct read_state *state)
{
    struct json_field ret = {NULL,NULL};
    
    if (!eat_whitespace(state)) {
        printf("Unexpected end of file parsing object field key\n");
        goto fail;
    }
    ret.key = json_parse_value(state);
    if (ret.key == NULL)
        goto fail;
    if (ret.key->type != JSON_STRING) {
        printf("Objected field key must be of type string\n");
        goto fail;
    }
    ;
    if (!eat_whitespace(state)) {
        printf("Unexpected end of file parsing object field\n");
        goto fail;
    }
    if (*state->read++ != ':') {
        printf("Didn't find expected ':' reading object\n");
        goto fail;
    }
    if (!eat_whitespace(state)) {
        printf("Unexpected end of file parsing object field value\n");
        goto fail;
    }
    ret.value = json_parse_value(state);
    if (ret.value == NULL)
        goto fail;
    return ret;
        
fail:
    free(ret.key);
    ret.key == NULL;
    return ret;
}

struct json_val *json_parse_object(struct read_state *state)
{
    struct json_val *value;
    char ch;
    int i;
    value = malloc(sizeof(*value));
    value->type = JSON_OBJECT;
    value->length = 0;
    value->object = NULL;


    if (!eat_whitespace(state)) {
        printf("Unexpected end of file parsing object\n");
        goto fail;
    }
    ch = *state->read++;
    if (ch == '}') {
        //pass
    } else if(ch == '\"'){
        do {
            value->length++;
            value->object = realloc(value->object, value->length * sizeof(*value->object));
            value->object[value->length-1] = json_parse_object_field(state);
            if (value->object[value->length-1].key == NULL || value->object[value->length-1].value == NULL)
                goto fail;

            if (!eat_whitespace(state)) {
                printf("Unexpected end of file parsing object\n");
                goto fail;
            }
            ch = *state->read++;
            if (ch != '}' && ch != ',') {
                printf("Unexpected %c reading object\n", ch);
                goto fail;
            }
        } while(ch == ',');
    }
    return value;

    fail:
    json_free_value(value);
    return NULL;
}
struct json_val *json_parse_array(struct read_state *state)
{
    struct json_val *value;
    char ch;
    int i;
    value = malloc(sizeof(*value));
    value->type = JSON_ARRAY;
    value->length = 0;
    value->array = NULL;

    if(!eat_whitespace(state)) {
        printf("Unexpected end of file parsing array\n");
        goto fail;
    }
    ch = *state->read++;
    if (ch == ']') {
        //pass
    } else {
        do {
            value->length++;
            value->array = realloc(value->object, value->length * sizeof(*value->object));
            value->array[value->length-1] = json_parse_value(state, ch);
            if (value->array[value->length-1] == NULL)
                goto fail;

            if (!eat_whitespace(state)) {
                printf("Unexpected end of file parsing array\n");
                goto fail;
            }
            ch = *state->read++;
        } while (ch == ',');
    }
    return value;

fail:
    json_free_value(value);
    return NULL;
}


static char unencode_hex(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'z') {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'Z') {
        return ch - 'A' + 10;
    }
    printf("Value is not hex: %c\n", ch);
    return 0;
}

static char unencode_hex_codepoint(char c0, char c1, char c2, char c3)
{
    if (!ishex(c0) || !ishex(c1) || !ishex(c2) || !ishex(c3))
        return 0;
    if (c0 != '0' || c1 != '0') {
        printf("We have a problem: using ascii, encoded is utf16: %c%c%c%c\n", c0, c1, c2, c3);
    }
    return unencode_hex(c2) * 16 + unencode_hex(c3);
}

static char json_parse_unescape_char(struct read_state *state)
{
    char ch, ret;
    if (read_state_left(state) >= 1) {
        printf("Unexpected end of file in escaped sequence\n");
        return 0;
    }
    ch = *state->read++;
    if (ch == 'u') {
        if (read_state_left(state) >= 4) {
            printf("Unexpected end of file in unicode escape sequence\n");
            return 0;
        }
        ret = unencode_hex_codepoint(state->read[0],state->read[1],state->read[2],state->read[3]);
        state->read += 4;
    } else {
        if (read_state_left(state) >= 1) {
            printf("Unexpected end of file in character escape sequence\n");
            return 0;
        }
        switch(ch) {
            case '\"':
                ret = '\"';
                break;
            case '\\':
                ret = '\\';
                break;
            case '/':
                ret = '/';
                break;
            case 'b':
                ret = '\b';
                break;
            case 'f':
                ret = '\f';
                break;
            case 'n':
                ret = '\n';
                break;
            case 'r':
                ret = '\r';
                break;
            case 't':
                ret = '\t';
                break;
            default:
                printf("invalid escape sequence found\n");
                return 0;
        }
    }
    return ret;
}

static bool is_invalid_string_char(char ch)
{
    return ch >= 0 && ch <= 0x1f;
}

static char *json_parse_chars(struct read_state *state)
{
    struct write_struct write;
    char ch, decoded_char;
    
    write_struct_init(&write);

    while (true) {
        if (read_state_left(state) >= 1) {
            printf("Unexpected end of file parsing string\n");
            goto fail;
        }
        ch = *state->read++;
        if (ch == '\\') {
            decoded_char = json_parse_unescape_char(state);
            //TODO: this has failure states
        } else if (ch == '\"') {
            break;
        } else if (is_invalid_string_char(ch)) {
            printf("Invalid unescaped character in string: %c\n", ch);
            goto fail;
        } else {
            decoded_char = ch;
        }
        if (write_struct_left(&write) < 1)
            write_struct_extend(&write);
        *write.write++ = decoded_char;
    }
    return write->buf;

fail:
    write_struct_free(&write);
    return NULL;
}

struct json_val *json_parse_string(struct read_state *state)
{
    struct json_val *value;

    value = malloc(sizeof(*value));
    value->type = JSON_STRING;

    value->string = json_parse_chars(state);
    if (value->string == NULL)
        goto fail;
    return value;

fail:
    free(value);
    return NULL;
}

static double get_value_of(int pre_decimal, int post_decimal, int exponent, bool negative, bool negative_exp)
{
    
}

struct json_val *json_parse_number(struct read_state *state, char ch)
{
    bool negative = false, negative_exp = false;
    int pre_decimal = 0, post_decimal = 0, exponent = 0;
    if (ch == '-') {
        negative = true;
    }

    if (!read_state_get(state, ch))
        goto fail;
    if (ch == '0') {
        //pass
    } else if (ch >= '1' && ch <= '9') {
        pre_decimal = ch - '0';

        while (true) {
            if (!read_state_get(state, ch))
                goto calc_value;
            if (ch < '0' || ch > '9') {
                read_state_put_back(state);
                break;
            }
            pre_decimal = 10*pre_decimal + ch - '0';
        }
    } else {
        printf("Invalid character found parsing number\n");
        goto fail;
    }
    
    if (read_state_get(state, ch)) {
        if (ch == '.') {
            if (!read_state_get(state, ch)) {
                printf("Expected digit after decimal\n");
                goto fail;
            }
            if(ch >= '0' && ch <= '9') {
                post_decimal = ch - '0';
            } else {
                printf("Invalid character found after decimal\n");
                goto fail;
            }
            while (true) {
                if (!read_state_get(state, ch)) {
                    goto calc_value;
                }
                if(ch >= '0' && ch <= '9') {
                    post_decimal = post_decimal*10 + ch - '0';
                } else {
                    read_state_put_back(state);
                    break;
                }
            }
        }
        else {
            read_state_put_back(state);
        }
    }
    if (read_state_get(state, ch)) {
        if (ch == 'e' || ch == 'E') {
            if (read_state_get(state, ch)) {
                if (ch == '+') {
                    //pass
                } else if (ch == '-') {
                    negative_exp = true;
                } else {
                    read_state_put_back(state);
                }
            }
            if (!read_state_get(state, ch)) {
                printf("Expected digit in exponent\n");
                goto fail;
            } else if (ch < '0' || ch > '9') {
                printf("Invalid character in exponent\n");
            } else {
                exponent = ch - '0';
                while (true) {
                    if (!read_state_get(state, ch)) {
                        goto calc_value;
                    }
                    if(ch >= '0' && ch <= '9') {
                        exponent = exponent*10 + ch - '0';
                    } else {
                        read_state_put_back(state);
                        break;
                    }
                }
            }
        }
        else {
            read_state_put_back(state);
        }
    }
calc_value:
    return get_value_of(pre_decimal,post_decimal,exponent,negative,negative_exp);

fail:
    return 0;

}

struct json_val *json_parse_value(struct read_state *state, char ch)
{
    struct json_val *value;
    switch (ch) {
        case 't':
            value = json_parse_true(state);
            break;
        case 'f':
            value = json_parse_false(state);
            break;
        case 'n':
            value = json_parse_null(state);
            break;
        case '\"':
            value = json_parse_string(state);
            break;
        case '{':
            value = json_parse_object(state);
            break;
        case '[':
            value = json_parse_array(state);
            break;
        default:
            value = json_parse_number(state, ch);
    }
    return value;
}

struct json_val *json_parse_base(struct read_state *state)
{
    if (read_state_left(state) >= 2) {
        if (*state->read == 0xfe) 
            state->read += 2;//Eat any BOMs
    }
    if (!eat_whitespace(state))
        return NULL;
    return json_parse_value(state, *state->read++);
}

struct json_val *json_parse(char *start, char *end)
{
    struct read_state state;
    read_state_init(&state, start, end);

    struct json_val *root = json_parse_base(&state);
}
