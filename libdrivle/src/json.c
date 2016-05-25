#include <json.h>

#include <iostate.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>




struct json_val *json_parse_true(struct read_state *state);
struct json_val *json_parse_false(struct read_state *state);
struct json_val *json_parse_null(struct read_state *state);
struct json_val *json_parse_number(struct read_state *state);
struct json_val *json_parse_string(struct read_state *state);
struct json_val *json_parse_object(struct read_state *state);
struct json_val *json_parse_array(struct read_state *state);
struct json_val *json_parse_value(struct read_state *state);

void json_free_value(struct json_val *val);
void json_error_print(struct read_state *val, const char* error);

struct json_val *json_parse_true(struct read_state *state)
{
    struct json_val *value;
    value = malloc(sizeof(*value));
    if (state->end - state->read < 4) {
        json_error_print(state, "Unexpected end of file parsing true literal\n");
        goto fail;
    }
    if (state->read[0] == 't' && state->read[1] == 'r' && state->read[2] == 'u' && state->read[3] == 'e') {
        state->read += 4;
        value->type = JSON_TRUE;
    } else {
        json_error_print(state, "Unexpected character in true literal\n");
        goto fail;
    }
    return value;

fail:
    free(value);
    return 0;
}
struct json_val *json_parse_false(struct read_state *state)
{
    struct json_val *value;
    value = malloc(sizeof(*value));
    if (state->end - state->read < 5) {
        json_error_print(state, "Unexpected end of file parsing false literal\n");
        goto fail;
    }
    if (state->read[0] == 'f' && state->read[1] == 'a' && state->read[2] == 'l' && state->read[3] == 's' && state->read[4] == 'e') {
        state->read += 5;
        value->type = JSON_FALSE;
    } else {
        json_error_print(state, "Unexpected character in false literal\n");
        goto fail;
    }
    return value;

fail:
    free(value);
    return 0;
}
struct json_val *json_parse_null(struct read_state *state)
{
    struct json_val *value;
    value = malloc(sizeof(*value));
    if (state->end - state->read < 4) {
        json_error_print(state, "Unexpected end of file parsing null literal\n");
        goto fail;
    }
    if (state->read[0] == 'n' && state->read[1] == 'u' && state->read[2] == 'l' && state->read[3] == 'l') {
        state->read += 4;
        value->type = JSON_NULL;
    } else {
        json_error_print(state, "Unexpected character in null literal\n");
        goto fail;
    }
    return value;

fail:
    free(value);
    return 0;
}

static struct json_field json_parse_object_field(struct read_state *state)
{
    struct json_field ret = {0,0};
    
    if (!eat_whitespace(state)) {
        json_error_print(state, "Unexpected end of file parsing object field key\n");
        goto fail;
    }
    ret.key = json_parse_value(state);
    if (ret.key == 0)
        goto fail;
    if (ret.key->type != JSON_STRING) {
        json_error_print(state, "Objected field key must be of type string\n");
        goto fail;
    }
    ;
    if (!eat_whitespace(state)) {
        json_error_print(state, "Unexpected end of file parsing object field\n");
        goto fail;
    }
    if (*state->read++ != ':') {
        json_error_print(state, "Didn't find expected ':' reading object\n");
        goto fail;
    }
    if (!eat_whitespace(state)) {
        json_error_print(state, "Unexpected end of file parsing object field value\n");
        goto fail;
    }
    ret.value = json_parse_value(state);
    if (ret.value == 0)
        goto fail;
    return ret;
        
fail:
    free(ret.key);
    ret.key == 0;
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
    value->object = 0;

    if (*state->read++ != '{') {
        json_error_print(state, "Internal error parsing object\n");
        goto fail;
    }

    if (!eat_whitespace(state)) {
        json_error_print(state, "Unexpected end of file parsing object\n");
        goto fail;
    }
    ch = *state->read++;
    if (ch == '}') {
        //pass
    } else if(ch == '\"'){
        do {
            read_state_put_back(state);
            value->length++;
            value->object = realloc(value->object, value->length * sizeof(*value->object));
            value->object[value->length-1] = json_parse_object_field(state);
            if (value->object[value->length-1].key == 0 || value->object[value->length-1].value == 0)
                goto fail;

            if (!eat_whitespace(state)) {
                json_error_print(state, "Unexpected end of file parsing object\n");
                goto fail;
            }
            ch = *state->read++;
            if (ch != '}' && ch != ',') {
                json_error_print(state, "Unexpected character reading object\n");
                goto fail;
            }
            if (!eat_whitespace(state) && ch != '}') {
                json_error_print(state, "Unexpected end of file parsing object\n");
                goto fail;
            }
        } while(ch == ',');
    }
    return value;

    fail:
    json_free_value(value);
    return 0;
}
struct json_val *json_parse_array(struct read_state *state)
{
    struct json_val *value;
    char ch;
    int i;
    value = malloc(sizeof(*value));
    value->type = JSON_ARRAY;
    value->length = 0;
    value->array = 0;

    if (*state->read++ != '[') {
        json_error_print(state, "Internal error parsing array\n");
        goto fail;
    }

    if(!eat_whitespace(state)) {
        json_error_print(state, "Unexpected end of file parsing array\n");
        goto fail;
    }
    ch = *state->read++;
    if (ch == ']') {
        //pass
    } else {
            read_state_put_back(state);
        do {
            value->length++;
            value->array = realloc(value->object, value->length * sizeof(*value->object));
            value->array[value->length-1] = json_parse_value(state);
            if (value->array[value->length-1] == 0)
                goto fail;

            if (!eat_whitespace(state)) {
                json_error_print(state, "Unexpected end of file parsing array\n");
                goto fail;
            }
            ch = *state->read++;
            if (ch != ']' && ch != ',') {
                json_error_print(state, "Unexpected character found parsing array\n");
                goto fail;
            }
            if (!eat_whitespace(state) && ch != ']') {
                json_error_print(state, "Unexpected end of file parsing array\n");
                goto fail;
            }
        } while (ch == ',');
    }
    return value;

fail:
    json_free_value(value);
    return 0;
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

static bool ishex(char ch)
{
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9');
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
    if (*state->read++ != '\\') {
        json_error_print(state, "Internal error parsing escape sequence\n");
    }

    if (read_state_left(state) < 1) {
        json_error_print(state, "Unexpected end of file in escaped sequence\n");
        return 0;
    }
    ch = *state->read++;
    if (ch == 'u') {
        if (read_state_left(state) < 4) {
            json_error_print(state, "Unexpected end of file in unicode escape sequence\n");
            return 0;
        }
        ret = unencode_hex_codepoint(state->read[0],state->read[1],state->read[2],state->read[3]);
        state->read += 4;
    } else {
        if (read_state_left(state) < 1) {
            json_error_print(state, "Unexpected end of file in character escape sequence\n");
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
                json_error_print(state, "invalid escape sequence found\n");
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
    struct write_state write;
    char ch, decoded_char;

    if (*state->read++ != '\"') {
        json_error_print(state, "Internal error parsing string value\n");
        goto fail;
    }
    
    write_state_init(&write);

    while (true) {
        if (read_state_left(state) < 1) {
            json_error_print(state, "Unexpected end of file parsing string\n");
            goto fail;
        }
        ch = *state->read++;
        if (ch == '\\') {
            read_state_put_back(state);
            decoded_char = json_parse_unescape_char(state);
            //TODO: this has failure states
        } else if (ch == '\"') {
            break;
        } else if (is_invalid_string_char(ch)) {
            json_error_print(state, "Invalid unescaped character in string\n");
            goto fail;
        } else {
            decoded_char = ch;
        }
        if (write_state_left(&write) < 1)
            write_state_extend(&write);
        *write.write++ = decoded_char;
    }

    if (write_state_left(&write) < 1)
        write_state_extend(&write);
    *write.write++ = '\0';
    return write.buf;

fail:
    write_state_free(&write);
    return 0;
}

struct json_val *json_parse_string(struct read_state *state)
{
    struct json_val *value;

    value = malloc(sizeof(*value));
    value->type = JSON_STRING;

    value->string = json_parse_chars(state);
    if (value->string == 0)
        goto fail;
    return value;

fail:
    free(value);
    return 0;
}

static long int ten_to(int power)
{
    long int ret = 1;
    if (power < 0)
        return 0;
    while (power--)
        ret *= 10;
    return ret;
}

static struct json_val *json_create_number(double value)
{
    struct json_val *ret;
    ret = malloc(sizeof(*ret));

    ret->type = JSON_NUMBER;
    ret->number = value;
    return ret;
}

struct json_val *json_parse_number(struct read_state *state)
{
    char dbl_buf[30];
    char *write = dbl_buf;
    char ch = *state->read++;

    if (ch == '-') {
        *write++ = '-';
    }

    if (!read_state_get(state, &ch))
        goto fail;
    if (ch == '0') {
        //pass
    } else if (ch >= '1' && ch <= '9') {
        *write++ = ch;

        while (true) {
            if (!read_state_get(state, &ch))
                goto calc_value;
            if (ch < '0' || ch > '9') {
                read_state_put_back(state);
                break;
            }
            *write++ = ch;
        }
    } else {
        json_error_print(state, "Invalid character found parsing number\n");
        goto fail;
    }
    
    if (read_state_get(state, &ch)) {
        if (ch == '.') {
            *write++ = '.';
            if (!read_state_get(state, &ch)) {
                json_error_print(state, "Expected digit after decimal\n");
                goto fail;
            }
            if(ch >= '0' && ch <= '9') {
                *write++ = ch;
            } else {
                json_error_print(state, "Invalid character found after decimal\n");
                goto fail;
            }
            while (true) {
                if (!read_state_get(state, &ch)) {
                    goto calc_value;
                }
                if(ch >= '0' && ch <= '9') {
                    *write++ = ch;
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
    if (read_state_get(state, &ch)) {
        if (ch == 'e' || ch == 'E') {
            *write++ = 'e';
            if (read_state_get(state, &ch)) {
                if (ch == '+' || ch == '-') {
                    *write++ = ch;
                } else {
                    read_state_put_back(state);
                }
            }
            if (!read_state_get(state, &ch)) {
                json_error_print(state, "Expected digit in exponent\n");
                goto fail;
            } else if (ch < '0' || ch > '9') {
                json_error_print(state, "Invalid character in exponent\n");
            } else {
                *write++ = ch;
                while (true) {
                    if (!read_state_get(state, &ch)) {
                        goto calc_value;
                    }
                    if(ch >= '0' && ch <= '9') {
                        *write++ = ch;
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
    *write++ = '\0';
    return json_create_number(strtod(dbl_buf, 0));

fail:
    return 0;

}

struct json_val *json_parse_value(struct read_state *state)
{
    struct json_val *value;
    char ch;
    
    if (read_state_left(state) < 1) {
        json_error_print(state, "Unexpected end of file parsing generic value\n");
        return 0;
    }
    ch = *state->read;
    
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
            value = json_parse_number(state);
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
        return 0;
    return json_parse_value(state);
}

struct json_val *json_parse(char *start, char *end)
{
    struct read_state state;
    read_state_init(&state, start, end);

    struct json_val *root = json_parse_base(&state);
}

void json_free_value(struct json_val *value)
{
    int i;
    if (!value)
        return;
    switch (value->type) {
        case JSON_OBJECT:
            for (i = 0; i < value->length; i++) {
                json_free_value(value->object[i].key);
                json_free_value(value->object[i].value);
            }
                free(value->object);
                break;
            case JSON_ARRAY:
                for (i = 0; i < value->length; i++) {
                    json_free_value(value->array[i]);
                }
                free(value->array);
                break;
            case JSON_STRING:
                free(value->string);
                break;
            case JSON_NUMBER: case JSON_TRUE: case JSON_FALSE:
            case JSON_NULL: default:
                break;
        }
        free(value);
    }

    void json_error_location(struct read_state *state, int offset, int *row, int *col)
    {
        int i;
        int _row = 1, _col = 1;
        bool newline, tab;
        
        for (i = 0; i < offset; i++) {
            newline = false;
            tab = false;
            switch (state->buf[i]) {
                case '\r':
                    if (state->buf[i+1] == '\n') {
                        i++;
                        newline = true;
                    }
                    break;
                case '\n':
                    newline = true;
                    break;
                case '\t':
                    tab = true;
                    break;
                default:
                    break;
            }
            if (newline) {
                _col = 1;
            _row++;
        } else if (tab) {
            _col = ((_col-1)&0xfc) + 5;
        } else {
            _col++;
        }
    }
    if (col)
        *col = _col;
    if (row)
        *row = _row;
}

void json_error_location_here(struct read_state *state, int *row, int *col)
{
    json_error_location(state, state->read - state->buf, row, col);
}

void json_error_print(struct read_state *state, const char* error)
{
    int row, col;
    json_error_location_here(state, &row, &col);
    printf("ERR:%d:%d - %s\n", row, col, error);
}
