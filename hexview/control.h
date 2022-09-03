#ifndef COMMANDS_H
#define COMMANDS_H

typedef struct state_s state_t;

enum
{
	Continue,
	Exit
};

state_t *create_state();
void destroy_state(state_t *state);

int open_file_on_state(state_t *state, const char *filename);
int run_string(state_t *state, const char *string);

#endif