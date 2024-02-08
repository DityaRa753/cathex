/*
	Author: DityaRa753
	The program is designed for output file in text representation or
	hexdecimal.
	Usage one module: string.h in dir lib.
	Program compile: usage make.
	Labwork #1
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "string.h"
#include <sys/stat.h>

/* usage printf(_()) - for localization text */
#ifndef TORUS
#define _(STR) (STR)
#endif

/* low-order=00001111; high-order=11110000 */
#define HI_TETRA(b) (((b) >> 4) & 0x0f)
#define LO_TETRA(b) ((b) & 0x0f)

/* find a bug with a large file in hex format, newline don't print,
	 why is that so...
UPD:  This is a bag related to stack overflow error. Check has been added*/

enum Mode { text, hex };
/* constant time compilation. */
enum {
	buffer_size = 1024, stdout_ = 1, exit_success = 0, val_n_specified = 1,
	few_arguments = 2, key_n_specified = 3, long_a_err = 4, path_missing = 5,
	not_recognized = 6, large_file = 7
};

static char tetr2ascii(char tetr) { return tetr < 10 ? tetr+48 : tetr+55; }

static void do_file_hex(int fd, char (*buf)[buffer_size], unsigned int nl)
{
	int i;
	static int size;
	size = read(fd, buf, buffer_size);
	if(size <= 0)
		return;
	for(i = 0; i < size; i++) {
		if(nl != 0 && nl % 20 == 0)
			printf("\n");
		printf("%c%c ", tetr2ascii(HI_TETRA((*buf)[i])),
				tetr2ascii(LO_TETRA((*buf)[i])));
		/* new line counter */
		++nl;
	}
	do_file_hex(fd, buf, nl);
}

static void print_file_hex(int fd)
{
	static char buf[buffer_size];
	do_file_hex(fd, &buf, 0);
}

static void do_print_file(int fd, char (*buf)[buffer_size])
{
	/* static used so as not to create every time */
	static int size;
	size = read(fd, buf, buffer_size);
	if(size <= 0)
		return;
	write(stdout_, buf, size);
	do_print_file(fd, buf);
}

static void print_file_text(int fd)
{
	static char buf[buffer_size];
	do_print_file(fd, &buf);
}

#ifdef CYCLE_IMP
static void cycle_print_file(int fd)
{
	ssize_t size;
	char buf[buffer_size];

	while(1) {
		size = read(fd, &buf, buffer_size);
		if(size <= 0)
			return;
		write(stdout_, &buf, size);
	}
}
#endif

static void print_help_message(const char * const *argv)
{
	printf(_("This program output content of a file to stdout\n"
						"(screen by default) in different modes:\ntext or hexdecimal."
						" In hexdecimal case, print high-order first.\nText mode by"
						" default\nUsage %s:\n"
						"Required arguments:\n\t"
						"<path to file> : Absolute or relative.\n"
						"Additional arguments:\n\t"
						"-m or --mode : output mode, available two mode:\n\t  text and"
						" hexdecimal, specify hex, h, text, txt.\n\t"
						"-h or --help : print this message.\n"
						"Example:\n\t%s ./somefile --mode=hex\n\t"
						"%s -m h /home/user1/notes\n\t"
						"%s Makefile -mh\n\t"
						"%s Makefile --mode=txt\n"), argv[0], argv[0], argv[0], argv[0], argv[0]);
}

static void
print_help_message_short(const char *text, const char * const *argv)
{
	fprintf(stderr, _("%s\nUsage %s -h or --help\n"), text, argv[0]);
}

/*  */

static void print_arg_err(const char *pname, const char *arg)
{
	fprintf(stderr, _("Unrecognized argument: %s\nUsage:%s -h or --help\n"), 
		 arg, pname);
}

static void
check_arguments(int argc, const char * const *argv, enum Mode *mode,
		const char **file_path)
{
	int i;
	/* empty check. */
	if(argc < 2) {
		/* argv is name program */
		print_help_message_short("Few command line arguments!", argv);
		exit(few_arguments);
	}
	/* check help */
	if(compare(argv[1], "-h") || compare(argv[1], "--help")) {
		print_help_message(argv);
		exit(exit_success);
	}
	for(i = 1; i < argc; i++) {
		/* long arg */
		if(argv[i][0] == '-' && argv[i][1] == '-') {
			if(compare(argv[i], "--mode=hex") || compare(argv[i], "--mode=h")) {
				*mode = hex;
				continue;
			}
			else if(compare(argv[i], "--mode=text") ||
					compare(argv[i], "--mode=txt")) {
				*mode = text;
				continue;
			}
			else if(compare(argv[i], "--mode")) {
				if(!argv[i+1]) {
					fprintf(stderr, _("Missing value for long key: %s\n"), argv[i]);
					exit(val_n_specified);
				}
				if(compare(argv[i+1], "hex") || compare(argv[i+1], "h")) {
					*mode = hex;
				} else if(compare(argv[i+1], "text") || compare(argv[i+1], "txt")) {
					*mode = text;
				}
				else {
					fprintf(stderr, _("Error value for long key: %s '%s'\n"),
							argv[i], argv[i+1]);
					exit(val_n_specified);
				}
			++i;
			}
			else {
				print_arg_err(argv[0], argv[i]);
				exit(not_recognized);
			}
		}
		/* short argument */
		else if(argv[i][0] == '-') {
			if(argv[i][1] == 'm') {
				if(argv[i][2] == 'h' || compare(&argv[i][2], "hex")) {
					*mode = hex;
					continue;
				} else if(argv[i][2] == 't' || compare(&argv[i][2], "text") ||
						compare(&argv[i][2], "txt")) {
					*mode = text;
					continue;
				}
				if(!argv[i+1]) {
					printf(_("Missing value for short key: %s\n"), argv[i]);
					exit(val_n_specified);
				}
				if(compare(argv[i+1], "hex") || compare(argv[i+1], "h")) {
					*mode = hex;
				}
				else if(compare(argv[i+1], "text") || compare(argv[i+1], "txt")) {
					*mode = text;
				}
				else {
					print_arg_err(argv[0], argv[i]);
					exit(not_recognized);
				}
				++i;
			}
			else {
				print_arg_err(argv[0], argv[i]);
				exit(not_recognized);
			}
		}
		/* required argument */
		else {
			if(!*file_path) *file_path = argv[i];
		}
	}
}

int main(int argc, const char * const * argv)
{
	int fd, check_stat;
	enum Mode mode = text;
	const char *file_path = NULL;
	struct stat info;
	check_arguments(argc, argv, &mode, &file_path);
	if(file_path == NULL) {
		print_help_message_short("Missing required path to file!", argv);
		exit(path_missing);
	}
	fd = open(file_path, O_RDONLY);
	if(fd < 0) {
		perror("Error with open file ");
		exit(errno);
	}
	check_stat = fstat(fd, &info);
	if(check_stat < 0) {
		fprintf(stderr, ("Some error with try get info about file %s"),
				file_path);
	}
	/* doesn't work with files larger than 170mb. Maybe fix it :)*/
	if(info.st_size > 178057520) {
		printf(_("Sorry, file is too large, greater 170mb. It doesn't work!\n"));
		return large_file;
	}
	/* just display text to stdout */
	if(mode == text) {
		print_file_text(fd);
		printf("\n");
	}
	/* display hex format */
	else {
		print_file_hex(fd);
		printf("\n");
	}
	close(fd);
	return 0;
}
