#include "control.h"

#include <string.h>
#include <stdio.h>

#include "tokenizer.h"
#include "file.h"
#include "util.h"

#define BYTES_TO_DISPLAY 128
#define sayhelp printf("Invalid usage, try \033[95mhelp\033[m.\n")

typedef int(*cmd_exec_fn)(state_t *, token_list_t *);

struct cmd
{
	cmd_exec_fn proc;
	char name[8];

	struct cmd *next;
};

struct state_s
{
	file_t *file;
	alist_t *bindings;
	unsigned int off;
	int current_endianess;
	int max_strlen;

	struct cmd *first;  // linked list of avaliable commands
};

static void create_cmd(state_t *state, cmd_exec_fn proc, const char *name);

static int exit_cmd(state_t *state, token_list_t *tokens);
static int tell_cmd(state_t *state, token_list_t *tokens);
static int seek_cmd(state_t *state, token_list_t *tokens);
static int peek_cmd(state_t *state, token_list_t *tokens);
static int vals_cmd(state_t *state, token_list_t *tokens);
static int endi_cmd(state_t *state, token_list_t *tokens);
static int strl_cmd(state_t *state, token_list_t *tokens);
static int darr_cmd(state_t *state, token_list_t *tokens);
static int help_cmd(state_t *state, token_list_t *tokens);
static int bind_cmd(state_t *state, token_list_t *tokens);
static int jump_cmd(state_t *state, token_list_t *tokens);

state_t *
create_state()
{
	state_t *state = malloc(sizeof(state_t));
	if (!state)
		return NULL;

	state->file = NULL;
	state->bindings = alist_create(STRCMP, STRCPY, STRFREE, NULL, NULL);
	if (!state->bindings)
	{
		free(state);
		return NULL;
	}

	state->off = 0;
	state->current_endianess = NATIVE_ENDIANESS;
	state->max_strlen = 32;
	state->first = NULL;

	create_cmd(state, &exit_cmd, "exit");
	create_cmd(state, &tell_cmd, "tell");
	create_cmd(state, &seek_cmd, "seek");
	create_cmd(state, &peek_cmd, "peek");
	create_cmd(state, &vals_cmd, "vals");
	create_cmd(state, &endi_cmd, "endi");
	create_cmd(state, &strl_cmd, "strl");
	create_cmd(state, &darr_cmd, "darr");
	create_cmd(state, &help_cmd, "help");
	create_cmd(state, &bind_cmd, "bind");
	create_cmd(state, &jump_cmd, "jump");

	return state;
}

void
destroy_state(state_t *state)
{
	struct cmd *cmd;

	if (state->file)
		close_file(state->file);

	while (state->first)
	{
		cmd = state->first->next;
		free(state->first);
		state->first = cmd;
	}

	free(state);
}

int
open_file_on_state(state_t *state, const char *filename)
{
	char sizestr[16];

	if (state->file)
	{
		close_file(state->file);
		state->file = NULL;
	}

	if (!filename)
		return 1;

	state->file = open_file(filename);
	if (!state->file)
		return 0;

	/* make file size string */
	if (state->file->size < 1024)
		snprintf(sizestr, sizeof(sizestr), "%d B", state->file->size);
	else if (state->file->size < 1024 * 1024)
		snprintf(sizestr, sizeof(sizestr), "%d KiB", state->file->size / 1024);
	else
		snprintf(sizestr, sizeof(sizestr), "%d MiB", state->file->size / 1024 / 1024);

	printf("File: \033[33m'%s'\033[m\n", filename);
	printf("Size: \033[94m%s\033[m [\033[92m0x00000000\033[m, \033[92m0x%08x\033[m)\n", sizestr, state->file->size);
	printf("Mode is %s endian.\n", state->current_endianess == LittleEndian ? "little" : "big");

	return 1;
}

int
run_string(state_t *state, const char *string)
{
	struct cmd *cmd;
	token_list_t *tokens;

	tokens = tokenize(string);
	if (!tokens)
		return Continue;

	for (cmd = state->first; cmd; cmd = cmd->next)
	{
		if (!strcmp(cmd->name, tokens->token.string))
			return cmd->proc(state, tokens);
	}

	printf("?\n");
	return Continue;
}

static void
create_cmd(state_t *state, cmd_exec_fn proc, const char *name)
{
	struct cmd *cmd = malloc(sizeof(struct cmd));
	if (cmd)
	{
		cmd->proc = proc;
#if _WIN32
		strcpy_s(cmd->name, sizeof(cmd->name), name);
#elif __linux__
		strncpy(cmd->name, name, sizeof(cmd->name));
#endif

		cmd->next = state->first;
		state->first = cmd;
	}
}

static int
exit_cmd(state_t *state, token_list_t *tokens)
{
	return Exit;
}

static int
tell_cmd(state_t *state, token_list_t *tokens)
{
	printf("Offset: \033[92m0x%08x\033[m\nSize:   \033[92m0x%08x\033[m\n", state->off, state->file->size);
	return Continue;
}

static int
seek_cmd(state_t *state, token_list_t *tokens)
{
	enum
	{
		ModeSet = 0,
		ModeAdd = 1,
		ModeSub = 2
	};

	int mode;
	token_list_t *otok;
	char *toread;
	char *end;
	int num;

	mode = ModeSet;

	otok = offset_token(tokens, 1);
	if (!otok)
	{
		sayhelp;
		return Continue;
	}

	toread = otok->token.string;
	if (strcmp(toread, "end"))
	{
		switch (toread[0])
		{
		case '+':
			mode = ModeAdd;
			toread++;
			break;
		case '-':
			mode = ModeSub;
			toread++;
			break;
		}

		num = strtol(toread, &end, 0);

		switch (mode)
		{
		case ModeSet:
			state->off = num;
			break;
		case ModeAdd:
			state->off += num;
			break;
		case ModeSub:
			state->off -= num;
			break;
		}
	}
	else
		state->off = state->file->size;

	if (state->off < 0)
		state->off = 0;
	else if (state->off >= state->file->size)
	{
		state->off = state->file->size - 1;
		if (state->off < 0)
			state->off = 0;
	}

	printf("Now looking at offset \033[92m0x%08x\033[m\n", state->off);

	return Continue;
}

static int
peek_cmd(state_t *state, token_list_t *tokens)
{
	unsigned int i, at;
	unsigned int m;

	printf("           ");
	printf("\033[4m");
	for (i = 0; i < 16; i++)
		printf(" %02hhx", i);
	printf("\033[m\n");

	for (i = 0; i < BYTES_TO_DISPLAY; i++)
	{
		at = state->off + i;
		m = i % 16;
		if (at >= state->file->size)
		{
			for (i = m; i < 16; i++)
				printf(" \033[41m??\033[m");
			break;
		}

		if (m == 0)
		{
			if (i > 0)
				putchar('\n');
			printf("\033[90m0x%08x \033[m", at);
		}
		printf(" %02hhx", state->file->data[at]);
	}
	printf("\n");

	return Continue;
}

static int
vals_cmd(state_t *state, token_list_t *tokens)
{
	value_u *valin;
	outvalues_t valout;
	int read;
	union { utf8_t *cursor8; utf16_t *cursor16; } strs;

	valin = (value_u *)(state->file->data + state->off);
	read = to_native_endianess(valin, state->file->size - state->off, state->current_endianess, &valout);
	switch (read)
	{
	case 8:
		break;
	case 4:
		break;
	case 2:
		break;
	case 1:
		break;
	}

	if (read > 0)
	{
		printf("int8:    %hhd\n", valout.i8);
		printf("uint8:   %hhu\n", valout.ui8);
		if (read < 2) goto below16;
	
		printf("int16:   %hd\n", valout.i16);
		printf("uint16:  %hu\n", valout.ui16);
		if (read < 4) goto below32;

		printf("int32:   %d\n", valout.i32);
		printf("uint32:  %u\n", valout.ui32);
		if (read < 8) goto below64;

		printf("int64:   %lld\n", valout.i64);
		printf("uint64:  %llu\n", valout.ui64);

		goto floats;

		below16:
		printf("int16:   \033[41m?\033[m\n");
		printf("uint16:  \033[41m?\033[m\n");
		below32:
		printf("int32:   \033[41m?\033[m\n");
		printf("uint32:  \033[41m?\033[m\n");
		below64:
		printf("int64:   \033[41m?\033[m\n");
		printf("uint64:  \033[41m?\033[m\n");

		floats:
		if (read < 4)
		{
			printf("float32: \033[41m?\033[m\n");
			printf("float64: \033[41m?\033[m\n");;
		}
		else
		{
			printf("float32: %g\n", (double)valout.f32);
			if (read < 8) printf("float64: \033[41m?\033[m\n");
			else printf("float64: %g\n", (double)valout.f64);
		}

		printf("utf8:    ");
		strs.cursor8 = valout.utf8;
		for (long i = 0; i < state->max_strlen && i < read; i++, strs.cursor8++)
		{
			if (!*strs.cursor8)
				break;
			printf("%c", *strs.cursor8);
		}
		printf("\n");

		printf("utf16:   ");
		strs.cursor16 = valout.utf16;
		for (long i = 0; i < state->max_strlen && i < read / 2; i++, strs.cursor16++)
		{
			if (!*strs.cursor16)
				break;
			wprintf(L"%lc", *strs.cursor16);
		}
		printf("\n");
	}
	else
	{

	}

	return Continue;
}

static int
endi_cmd(state_t *state, token_list_t *tokens)
{
	token_list_t *it;

	it = offset_token(tokens, 1);
	if (!it)
	{
		sayhelp;
		return Continue;
	}

	if (!strcmp(it->token.string, "little"))
	{
		state->current_endianess = LittleEndian;
		printf("Now interpreting as little endian\n");
	}
	else if (!strcmp(it->token.string, "big"))
	{
		state->current_endianess = BigEndian;
		printf("Now interpreting as big endian\n");
	}
	else if (!strcmp(it->token.string, "native"))
	{
		state->current_endianess = NATIVE_ENDIANESS;
		if (NATIVE_ENDIANESS == LittleEndian)
			printf("Now interpreting as little endian\n");
		else
			printf("Now interpreting as big endian\n");
	}
	else
		printf("Endianess must be: little, big, or native\n");

	return Continue;
}

static int
strl_cmd(state_t *state, token_list_t *tokens)
{
	token_list_t *it;

	it = offset_token(tokens, 1);
	if (!it)
	{
		sayhelp;
		return Continue;
	}

	if (it->token.integer > 0)
		state->max_strlen = it->token.integer;

	printf("Max string length set to: %d\n", state->max_strlen);

	return Continue;
}

static int
darr_cmd(state_t *state, token_list_t *tokens)
{
	token_list_t *it;
	int elemtype, elemsize;
	int arrlen;
	int i;
	byte *loc;
	value_u *inval;
	outvalues_t outval;

	it = offset_token(tokens, 1);
	if (!it)
	{
		sayhelp;
		return Continue;
	}

	if (!strcmp(it->token.string, "int8"))
		elemtype = Int8;
	else if (!strcmp(it->token.string, "uint8"))
		elemtype = Uint8;
	else if (!strcmp(it->token.string, "int16"))
		elemtype = Int16;
	else if (!strcmp(it->token.string, "uint16"))
		elemtype = Uint16;
	else if (!strcmp(it->token.string, "int32"))
		elemtype = Int32;
	else if (!strcmp(it->token.string, "uint32"))
		elemtype = Uint32;
	else if (!strcmp(it->token.string, "int64"))
		elemtype = Int64;
	else if (!strcmp(it->token.string, "uint64"))
		elemtype = Uint64;
	else if (!strcmp(it->token.string, "float32") || !strcmp(it->token.string, "float"))
		elemtype = Float32;
	else if (!strcmp(it->token.string, "float64") || !strcmp(it->token.string, "double"))
		elemtype = Float64;
	else if (!strcmp(it->token.string, "utf8"))
		elemtype = Utf8;
	else if (!strcmp(it->token.string, "utf16"))
		elemtype = Utf16;
	else
	{
		sayhelp;
		return Continue;
	}

	switch (elemtype)
	{
	case Int8:
	case Uint8:
	case Utf8:
		elemsize = 1;
		break;
	case Int16:
	case Uint16:
	case Utf16:
		elemsize = 2;
		break;
	case Int32:
	case Uint32:
	case Float32:
		elemsize = 4;
		break;
	case Int64:
	case Uint64:
	case Float64:
		elemsize = 8;
		break;
	}

	it = offset_token(it, 1);
	if (!it)
	{
		sayhelp;
		return Continue;
	}

	arrlen = it->token.integer;
	if (arrlen < 0)
	{
		printf("Invalid array length.\n");
		return Continue;
	}

	printf("[");

	for (i = 0; i < arrlen; i++)
	{
		loc = state->file->data + state->off + (i * elemsize);
		if (loc + elemtype > state->file->data + state->off + state->file->size)
			break;
		inval = (value_u *)loc;

		to_native_endianess(inval, 8, state->current_endianess, &outval);

		switch (elemtype)
		{
		case Int8:
			printf(" %hhd", outval.i8);
			break;
		case Uint8:
			printf(" %hhd", outval.ui8);
			break;
		case Int16:
			printf(" %hd", outval.i16);
			break;
		case Uint16:
			printf(" %hu", outval.ui16);
			break;
		case Int32:
			printf(" %d", outval.i32);
			break;
		case Uint32:
			printf(" %u", outval.ui32);
			break;
		case Int64:
			printf(" %lld", outval.i64);
			break;
		case Uint64:
			printf(" %llu", outval.ui64);
			break;
		case Float32:
			printf(" %g", (double)outval.f32);
			break;
		case Float64:
			printf(" %g", (double)outval.f64);
			break;
		case Utf8:
			printf(" %c", *outval.utf8);
			break;
		case Utf16:
			wprintf(L" %lc", *outval.utf16);
			break;
		}
	}

	printf(" ]\n");

	return Continue;
}

static int
help_cmd(state_t *state, token_list_t *tokens)
{
	printf("\033[95mexit\033[m\n");
	printf(" Exit the program\n\n");

	printf("\033[95mtell\033[m\n");
	printf(" Display current offset and file size\n\n");

	printf("\033[95mseek\033[m [\033[92m<offset>\033[m|\033[33mend\033[m]\n");
	printf(" Seeks to a new location in the file. Supports decimal, hexadecimal, and\n");
	printf(" octal absolute and relative offsets. Use none, '0x', or '0' prefixes to\n");
	printf(" specify decimal, hexadecimal, and octal vals, respectively. Prefix with\n");
	printf(" '+' or '-' to do a relative seek. Use 'end' to seek to the end of the\n");
	printf(" file while still displaying as many bytes as possible.\n\n");

	printf("\033[95mpeek\033[m\n");
	printf(" Displays bytes at the current seek location.\n\n");

	printf("\033[95mvals\033[m\n");
	printf(" Displays a list of common byte and multi-byte interpretations in the\n");
	printf(" current endianess mode.\n\n");

	printf("\033[95mendi\033[m [\033[33mlittle\033[m|\033[33mbig\033[m|\033[33mnative\033[m]\n");
	printf(" Sets the endianess mode; how vals should interpret multi-byte\n");
	printf(" values. Native endianess means use the endianess of the\n");
	printf(" local machine.\n\n");

	printf("\033[95mstrl\033[m \033[92m<length>\033[m\n");
	printf(" Sets the maximum string length to display when using vals, for both utf8\n");
	printf(" and utf16 strings.\n\n");

	printf("\033[95mdarr\033[m \033[36m<type>\033[m \033[92m<length>\033[m\n");
	printf(" Interprets the current offset as an array with the give type and length.\n");
	printf(" type can be one of: int8, uint8, int16, uint16, int32, uint32, int64,\n");
	printf(" uint64, float32, float64, utf8, or utf16.\n\n");

	printf("\033[95mbind\033[m \033[36m<name>\033[m \033[36m<value, optional>\033[m\n");
	printf(" Binds a name to an integer value. The binding can then be subsequently\n");
	printf(" used in any future jump calls. If <value> is not specified, the binding\n");
	printf(" will be set to the current offset. If a binding with <name> already exists,\n");
	printf(" the old binding will be overwritten.\n\n");

	printf("\033[95mjump\033[m \033[36m<name>\033[m\n");
	printf(" Jumps to a file offset previously saved using bind. If the binding does not\n");
	printf(" exist, nothing will change.\n");

	return Continue;
}

static int
bind_cmd(state_t *state, token_list_t *tokens)
{
	token_list_t *it;
	const char *name;
	unsigned int value;

	it = offset_token(tokens, 1);
	if (!it)
	{
		sayhelp;
		return Continue;
	}

	name = it->token.string;

	it = offset_token(it, 1);
	value = it ? it->token.integer : state->off;

	alist_insert(state->bindings, KEY(name), VALUE(value));

	printf("Bound \033[33m%s\033[m -> \033[92m0x%08x\033[m\n", name, value);

	return Continue;
}

static int
jump_cmd(state_t *state, token_list_t *tokens)
{
	token_list_t *it;
	const char *name;
	value_t *value;

	it = offset_token(tokens, 1);
	if (!it)
	{
		sayhelp;
		return Continue;
	}

	name = it->token.string;

	value = alist_find(state->bindings, KEY(name));
	if (!value)
	{
		printf("\033[33m%s\033[m is not bound.\n", name);
		return Continue;
	}

	state->off = *(unsigned int *)value;

	if (state->off < 0)
		state->off = 0;
	else if (state->off >= state->file->size)
	{
		state->off = state->file->size - 1;
		if (state->off < 0)
			state->off = 0;
	}

	printf("Jumped to \033[92m0x%08x\033[m\n", state->off);

	return Continue;
}