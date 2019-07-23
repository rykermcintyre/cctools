/*
  Copyright (C) 2019- The University of Notre Dame This software is
  distributed under the GNU General Public License.  See the file
  COPYING for details.


  Given a file, replace environment variable with actual values
  into new file. If no output-file is specified the original will
  be replaced.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "debug.h"
#include "stringtools.h"
#include "xxmalloc.h"

void show_help(const char *exe) {
	fprintf(stderr, "Usage:\n%s input-file [output-file]\n", exe);
}

int main(int argc, char **argv) {

	if(argc < 2 || argc > 3) {
		show_help(argv[0]);
		fatal("ARGC %d: %s", argc, argv[1]);
		//exit(1);
	}


	const char *input = argv[1];
	FILE *INPUT = fopen(input, "r");
	if (INPUT == NULL) {
		fatal("unable to open %s: %s", input, strerror(errno));
	}

	char *output = NULL;
	if(argc > 2) {
		output  = xxstrdup(argv[2]);
	} else {
		output  = string_format("%s.XXXXXX",input);
		int output_fd = mkstemp(output);
		if (output_fd == -1){
			fatal("could not create `%s': %s", output, strerror(errno));
		}
		close(output_fd);
	}

	FILE *OUTPUT = fopen(output, "w");
	if (OUTPUT == NULL) {
		fatal("unable to open %s: %s", output, strerror(errno));
	}

	
	char variable[1024];
	int var_index = 0;
	int valid_var = 0;

	char c = fgetc(INPUT);
	while (c != EOF)
	{
		if (c == '$'){
			valid_var = 1;
		} else if (valid_var && (
				(c >= (char)65 && c <= (char)90) ||  // A-Z
				(c == (char)95) || // _
				(var_index > 1 && c >= (char)48 && c <= (char)57) // 0-9
			)) {
			variable[var_index] = c;
			var_index++;
		} else if (valid_var && var_index > 0) {
			variable[var_index] = '\0';
			const char *var = getenv(variable);
			if (var) {
				fprintf(OUTPUT, "%s", var);
			} else {
				debug(D_NOTICE, "failed to resolve %s environment variable, restoring string", variable);
			}
			valid_var = 0;
			var_index = 0;
		} else {
			if (valid_var) {
				fprintf(OUTPUT, "$");
			}
			valid_var = 0;
			var_index = 0;
		}

		if (!valid_var) {
			fprintf(OUTPUT, "%c", c);
		}
		c = fgetc(INPUT);
	}
	fclose(OUTPUT);
	fclose(INPUT);

}

/* vim: set noexpandtab tabstop=4: */
