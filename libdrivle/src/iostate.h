#ifndef IOSTATE_H
#define IOSTATE_H

#include <stdbool.h>

struct read_state
{
    const char *buf;
    const char *read;
    const char *end;
};

struct write_state
{
    char *buf;
    char *write;
    char *end;
};



extern bool eat_whitespace(struct read_state *state);

extern void read_state_init(struct read_state *read, const char *start, const char *end);
extern int read_state_left(struct read_state *read);
extern bool read_state_get(struct read_state *read, char *ch);
extern void read_state_put_back(struct read_state *read);

extern void write_state_init(struct write_state *write);
extern void write_state_extend(struct write_state *write);
extern void write_state_free(struct write_state *write);
extern int write_state_left(struct write_state *write);
#endif /* IOSTATE_H */
