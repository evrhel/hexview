# hexview

`hexview` is a high-performance command line tool for viewing binary files on Windows and Linux. It supports both viewing raw bytes as well as multi-byte values (such as large integer types, floating point types, and strings) in both little and big endian.

## Building

### Windows

Open the Visual Studio 2019 solution file `.\hexview.sln`. Change the build target to whatever you need and build the project. The executable `hexview.exe` will output in the respective directories:

- x86 Debug: `.\Debug`
- x86 Release: `.\Release`
- x64 Debug: `.\x64\Debug`
- x64 Release: `.\x64\Release`

To use in the command line, put the executable in some directory (perhaps `C:\hexview`) and add the directory to the `PATH`.

### Linux

`cd` into the directory `./hexview` and run `make`. The executable `hexview` will output in `./hexview`.

## Usage

Run `hexview --help`, or read here.

### Command Line

Usage: `hexview [options...] <filename>`.

`<filename>` specifies the path of the file to view.

`[options...]` includes the switches:

- `--help`, `-h`: Display the help message.
- `--version`, `-v`: Display version information.

### Commands

Once a file is loaded, a command interface will be avaliable. To see commands, run `help`, or read below:

- `exit`: Exit the program.
- `tell`: Display the current offset and file size.
- `seek [<offset>|end]`: Seek to a new location in the file. Supports decimal, hexadecimal, and octal absolute or relative offsets. Use no, `0x`, or `0` prefixes to specify decimal, hexadecimal, and octal offsets, respectively. Prefix with `+` or `-` to do a relative seek, the following value will add or subtract from the current offset, respectively. Specifying `end` will seek to the end of the file.
- `vals`: Displays a list of common byte and multi-byte interpretations. Will display signed and unsigned integers of widths 8, 16, 32, and 64, 32-bit and 64-big IEEE-754 floating point numbers, and null-terminated UTF-8 and UTF-16 strings. Endianess is determined using the `endi` command.
- `endi [little|big|native]`: Sets the endianess mode. The `vals` command will use this to change how it should interpret multi-byte values. `little`, `big`, and `native` represent little endian, big endian, and the local machine's endianess, respectively.
- `strl <length>`: Sets the maximum string length to display when the `vals` string is ran to `<length>`.
- `darr <type> <length>`: Interprets the current offset as an array of length `<length>` containing values of type `<type>`. `<type>` can be one of: `int8`, `uint8`, `int16`, `uint16`, `int32`, `uint32`, `int64`, `uint64`, `float32`, `float64`, `utf8`, or `utf16`.
- `bind <name> <value, optional>`: Binds a name to an integer value. The binding then can be subsequently used in any future jump calls. If `<value>` is not specified, the binding will be set to the current file offset. If a binding with `<name>` already exists, the old binding will be overwritten.
- `jump <name>`: Jumps to a file offset previously saved using bind. If the binding does not exist, nothing will change.
- `find pattern...`: Searches for a pattern in the file at the current offset. `pattern...` specifies the pattern to search for in the file. It takes the form of a space-delimeted value to search for. Each value should be its own argument, the entire pattern should not be one string. For each pattern argument, the argument can be one of the following:
	- `?[(nothing)|<count>]`: Always match. `<count>` can be used to match more than one byte.
	- `<byte>`: Match a single byte value.
	- `i8<int8>`: Match int8.
	- `ui8<uint8>`: Match uint8.
	- `i16<int16>`: Match int16.
	- `ui16<uint16>`: Match uint16.
	- `i32<int32>`: Match int32.
	- `ui32<uint32>`: Match uint32.
	- `i64<int64>`: Match int64.
	- `ui64<uint64>`: Match uint64.
	- `f32<f32>`: Match float32.
	- `f64<f64>`: Match float64.
	- `c<char>`: Match a single char8 character.
	- `wc<char>`: Match a single char16 character.
	- `s<string>`: Match a sequence of char8 characters.
	- `sn<string>`: Match a null-terminated char8 string.
	- `ws<string>`: Match a sequence of char16 characters.
	- `wsn<string>`: Match a null-terminated char16 string.