#include "control.h"
#include "util.h"

#include <stdio.h>

int
main(int argc, char *argv[])
{
	state_t *state;
	char line[256];
	int result;

	if (argc < 2)
	{
		printf("Usage: viewbin <filename>\n");
		return 0;
	}

	state = create_state();
	if (!state)
	{
		printf("Initialization failed.\n");
		return 0;
	}

	if (!open_file_on_state(state, argv[1]))
	{
		printf("Failed to open file.\n");
		return 0;
	}

	do
	{
		printf("> ");
		readline(line, sizeof(line));
		result = run_string(state, line);
	} while (result == Continue);

	destroy_state(state);

	return 0;
}
