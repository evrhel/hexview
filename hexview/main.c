#include "control.h"
#include "util.h"

#include <stdio.h>

#define HEXVIEW_VERSION "1.0.0-rc6"

struct command_line
{
	const char *filename;
};

static int parse_command_line(int argc, char *argv[], struct command_line *const out);
static void print_help(int full);
static void print_version();

int
main(int argc, char *argv[])
{
	struct command_line command_line;
	state_t *state;
	char line[256];
	int result;

	if (parse_command_line(argc, argv, &command_line))
		return 0;

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

	printf("Use \033[95mhelp\033[m for help.\n");
	do
	{
		printf("> ");
		readline(line, sizeof(line));
		result = run_string(state, line);
	} while (result == Continue);

	destroy_state(state);

	return 0;
}

static int
parse_command_line(int argc, char *argv[], struct command_line *const out)
{
	int i;

	memset(out, 0, sizeof(struct command_line));
	for (i = 1; i < argc; i++)
	{
		if (equals_ignore_case(argv[i], "--help") || equals_ignore_case(argv[i], "-h"))
		{
			print_help(1);
			return 1;
		}
		else if (equals_ignore_case(argv[i], "--version") || equals_ignore_case(argv[i], "-v"))
		{
			print_version();
			return 1;
		}
		else if (argv[i][0] == '-')
		{
			printf("Unknown switch '%s', use --help for help.\n", argv[i]);
			return 1;
		}
		else
		{
			out->filename = argv[i];
			break;
		}
	}

	if (!out->filename)
	{
		print_help(0);
		return 1;
	}

	return 0;
}

static void
print_help(int full)
{
	printf("Usage: hexview [options...] <filename>\n");
	if (!full)
	{
		printf("Try hexview --help\n");
		return;
	}

	printf("Where <filename> is the file to view.\n");
	printf("Where options include:\n");
	printf(" --help -h      Display this message.\n");
	printf(" --version -v   Display version information.\n");
}

static void
print_version()
{
	printf("hexview %s\n", HEXVIEW_VERSION);
}