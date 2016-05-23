#include <iostate.h>

static bool is_whitespace(char ch)
{
    return ch == '\t' || ch == '\r' || ch == '\n' || ch == ' ';
}

bool eat_whitespace(struct read_state *state)
{
    while (state->read != state->end && is_whitespace(state->read)) {
        state->read++;
    }
    return state->read != state->end;
}


void read_state_init(struct read_state *read, const char *start, const char *end)
{
    read->buf = start;
    read->read = start;
    read->end = end;
}

int read_state_left(struct read_state *read)
{
    return read->end - read->read;
}

bool read_state_get(struct read_state *read, char *ch)
{
    if (read->end != read->read) {
        *ch = *read->read++;
        return true;
    }
    return false;
}

void read_state_put_back(struct read_state *read)
{
    if (read->read != read->buf)
        read->read--;
}


void write_state_init(struct write_state *write)
{
    write->buf = malloc(20);
    write->write = write->buff;
    write->end = write->buf + 20;
}

void write_state_expand(struct write_state *write)
{
    int len;
    int offs;
    len = write->end - write->buf;
    offs = write->write - write->buf;
    write->buf = realloc(write->buf, len*2);
    write->write = write->buf + offs;
    write->end = write->buf + len;
}

void write_state_free(struct write_state *write)
{
    free(write->buf);
    write->buf = NULL;
    write->write = NULL;
    write->end = NULL;
}

int write_state_left(struct write_state *write)
{
    return write->end - write->write;
}
