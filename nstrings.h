/*
 * Warning: This code is considered incomplete. Use at your own risk.
 *
 * The purpose of this is to provide wrappers (and more) to the
 * standard C library's string functions that don't use
 * null-terminated strings.
 *
 * Potential TODOs:
 *   - Support unicode and utf8 runes.
 *   - Use a context struct for printing for gaining formatting accross multiple lines.
 */

#ifndef _NSTRINGS_H_
#define _NSTRINGS_H_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

typedef struct string {
	size_t length;
	const char* s;
} string;

#define String(s)     ((string){sizeof(s) - 1, (s)})
#define StringC(s)    ((string){strlen(s), (s)})
#define StringN(s, n) ((string){(n), (char*)(s)})

#define $(s)      String(s)
#define $$(s)     StringC(s)
#define $$$(s, n) StringN(s, n)


string Slice(string s, size_t start, size_t end);

#define Prefix(s, index) Slice((s), 0, (index))
#define Suffix(s, index) Slice((s), s.length - index, s.length)

void ToStrings(string* dest, char** strings, size_t n);
int Equals(string a, string b);
int HasPrefix(string prefix, string s);
int Compare(string a, string b);
ssize_t IndexOf(string haystack, char needle);
string Concat(char* buf, string x, string y);
string Chomp(string s);
string Itoa(size_t number, char buf[32], size_t len);
size_t Input(char* buf, size_t max);

#define Equals$(a, b) Equals($(a), (b))
#define HasPrefix$(p, s) HasPrefix($(p), (s))
#define Compare$(a, b) Compare($(a), (b))
#define IndexOf$(haystack, needle) IndexOf($(haystack), (needle))


void PrintFd(int fd, string fmt, ...);
void Print(string fmt, ...);
void Println(string fmt, ...);

// TODO: Redefine these so that Println$("hi") can be used without violating C99.
#define Print$(fmt, ...) Print($(fmt), ##__VA_ARGS__)
#define Println$(fmt, ...) Println($(fmt), ##__VA_ARGS__)
#define PrintFd$(fd, fmt, ...) PrintFd(fd, $(fmt), ##__VA_ARGS__)

#define PrintErr(fmt, ...) PrintFd(STDERR_FILENO, (fmt), ##__VA_ARGS__)
#define PrintErr$(fmt, ...) PrintFd(STDERR_FILENO, $(fmt), ##__VA_ARGS__)

#ifdef NSTRINGS_IMPLEMENTATION
#undef NSTRINGS_IMPLEMENTATION

string
Slice(string s, size_t start, size_t end)
{
	return ((string){end - start, s.s + start});
}

void
ToStrings(string* dest, char** strings, size_t n)
{
	size_t i;
	for (i = 0; i < n; ++i) {
		dest[i] = $$(strings[i]);
	}
}

int
Equals(string a, string b)
{
	return (a.length == b.length) && !strncmp(a.s, b.s, a.length);
}

int
HasPrefix(string prefix, string s)
{
	return Equals(prefix, Slice(s, 0, prefix.length));
}

int
Compare(string a, string b)
{
	return strncmp(a.s, b.s, a.length);
}

ssize_t
IndexOf(string haystack, char needle)
{
	char* p = memchr(haystack.s, needle, haystack.length);

	if (p) {
		return p - haystack.s;
	}

	return -1;
}

string
Concat(char* buf, string x, string y)
{
	memcpy(buf, x.s, x.length);
	memcpy(buf + x.length, y.s, y.length);

	return $$$(buf, x.length + y.length);
}

string
Chomp(string s)
{
	size_t i;
	
	if (s.length == 0) {
		return s;
	}

	// TODO: This could be faster.
	for (i = s.length - 1; s.s[i] == '\n' && i >= 0; --i);
	
	return $$$(s.s, i + 1);
}

string
Itoa(size_t number, char buf[32], size_t len)
{
	assert(len >= 32);
	size_t length = 0;

	buf += sizeof(char) * 31;

	if (number == 0) {
		*buf = '0';
		return $$$(buf, 1);
	}

	while (number) {
		*buf = (number % 10) + '0';
		--buf;
		number /= 10;
		++length;
	}

	return $$$(buf+1, length);
}

int
Atoi()
{
	// TODO: Implement.
	return 0;
}

size_t
Input(char* buf, size_t max)
{
	return read(STDIN_FILENO, buf, max);
}



void static
_Print(int fd, string fmt, va_list args)
{
	// TODO: Buffer this like libc does for printf. Switch to fwrite?

	// TODO: Add more formating features.

	size_t index = IndexOf(fmt, '%');
	if (index == -1) {
		write(fd, fmt.s, fmt.length);
		return;
	}

	// Print out everything until the '%'
	write(fd, fmt.s, index);

	// Handle the '%' formatter
	char nbuf[32];
	ssize_t param;
	switch (fmt.s[++index]) {
	case '%': { // Escape
		write(fd, "%", 1);
		++index;
	} break;
	case 's': { // String
		string s = va_arg(args, string);
		write(fd, s.s, s.length);
		++index;
	} break;
	case 'S': {// C String
		char* s = va_arg(args, char*);
		write(fd, s, strlen(s));
		++index;
	} break;
	case 'd': { // 32 Number
		param = va_arg(args, int);

		if (param < 0) {
			write(fd, "-", 1);
			param *= -1;
		}

		string s = Itoa((unsigned int)param, nbuf, sizeof nbuf);
		write(fd, s.s, s.length);
		++index;
	} break;
	case 'l': { // 64 Number
		param = va_arg(args, size_t);

		if (param < 0) {
			write(fd, "-", 1);
			param *= -1;
		}

		string s = Itoa((size_t)param, nbuf, sizeof nbuf);
		write(fd, s.s, s.length);
		++index;
	} break;
	case 'c': { // Character
		char c = (char) va_arg(args, int);
		write(fd, &c, 1);
		++index;
	} break;
	default: {
		// Bug-free code should never reach this.
		PrintFd(STDERR_FILENO, $("_Print: invalid formatter '%%%c'\n"), fmt.s[index]);
		assert(0);
	} break;
	}

	if (index > fmt.length)
		return;

	string t = Slice(fmt, index, fmt.length);
	_Print(fd, t, args);
}

void
PrintFd(int fd, string fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	_Print(fd, fmt, args);
	va_end(args);
}

void
Print(string fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	_Print(STDOUT_FILENO, fmt, args);
	va_end(args);
}

static void
_Println(int fd, string fmt, va_list args)
{
	_Print(fd, fmt, args);
	write(fd, "\n", 1);
}

void
Println(string fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	_Println(STDOUT_FILENO, fmt, args);
	va_end(args);
}

#endif // NSTRINGS_IMPLEMENTATION

#ifdef NSTRINGS_MAIN
#undef NSTRINGS_MAIN
/* Create a version of main called Main whose signature is Main(int, string*); */
#define Main(c, v)						\
	Main(c, v);						\
	int main(int argc, char** argv) {			\
		string args[argc];				\
		ToStrings(args, argv, argc);			\
		return Main(argc, args);			\
	}							\
	int Main(c, v)
#endif // NSTRINGS_MAIN

#ifdef NSTRINGS_UNIX
/*
 TODO: Implement versions of the Print functions that use context to
       control the terminal output. They should possess features such
       as formatting output into columns, displaying progress
       bars/wheels, and other dynamic terminal output.

       These features will require termios.h and will break
       portability.
*/
#endif // NSTRINGS_UNIX

#endif // _NSTRINGS_H_
