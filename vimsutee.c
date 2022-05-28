/*
Copyright 2022 Lucio Tomarchio
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#define BLOCK_SIZE 512

struct node {
	char *exe;
	struct node *next;
};

struct {
	struct node *head;
} global;

static int
needles( void )
{
	char *exe[] = {"gvim", "vim", "vim.gtk3", "vim.tiny", NULL};
	char *buffer;
	struct stat stats;
	struct node *ring = NULL;

	for(size_t counter=0; exe[counter] != NULL; counter++) {
		char *path = strdup(getenv( "PATH" ));
		for ( char *dir = strtok( path, ":" ); dir!=NULL; dir = strtok( NULL, ":" ) ) {
			asprintf(&buffer, "%s/%s", dir, exe[counter]);
			if ((lstat(buffer, &stats) < 0) || !(S_ISREG(stats.st_mode)) || !(stats.st_mode&S_IXOTH)) {
				free(buffer);
				continue;
			}
			ring = (struct node *) malloc (sizeof(struct node));
			ring->next = global.head;
			ring->exe = buffer;
			global.head = ring;
		}
		free(path);
	}
	return global.head != NULL;
}

static pid_t
parent_pid(pid_t pid)
{
	FILE *fp;
	char *filename;

	asprintf(&filename, "/proc/%d/stat", pid);
	fp=fopen(filename, "r");
	free(filename);
	if(fp == NULL)
		return -1;
	// Ignore one number, then two strings, then read a number
	fscanf(fp, "%*d %*s %*s %d", &pid);
	fclose(fp);
	return pid;
}

static int
vim(pid_t pid)
{
	char *exe;
	char buffer[BLOCK_SIZE];
	int result = 0;

	memset(buffer, '\0', BLOCK_SIZE);
	asprintf(&exe, "/proc/%d/exe", pid);
	if (readlink(exe, buffer, BLOCK_SIZE-1) < 1)
		result = -1;
	else for (struct node *current = global.head; current!=NULL; current=current->next)
		if (!strcmp(current->exe, buffer)) {
			result = 1;
			break;
		}
	free(exe);
	return result;
}

static int
write_out(char *filename)
{
	size_t b_in;
	char buffer[BLOCK_SIZE];
	FILE *fp = fopen(filename, "wb");

	if (fp == NULL)
		return EXIT_FAILURE+0;
	do {
		b_in = fread(buffer, sizeof(char), BLOCK_SIZE, stdin);
		fwrite(buffer, sizeof(char), b_in, fp);
	} while (b_in == BLOCK_SIZE);
	fclose(fp);
	return EXIT_SUCCESS;
}

int
main(int argc, char **argv)
{
	int exit_code = EXIT_SUCCESS;
	char buffer[BLOCK_SIZE];

	global.head = NULL;
	if (readlink("/proc/self/exe", buffer, BLOCK_SIZE-1) < 1)
		exit_code = EXIT_FAILURE+1;
	else {
		struct stat stats;

		if (stat(buffer, &stats) < 0)
			exit_code = EXIT_FAILURE+2;
		else if (!group_member(stats.st_gid))
			exit_code = EX_NOPERM;
		else if (!needles())
			exit_code = EXIT_FAILURE+3;
		else if (argc>1)
			for (pid_t pid = getpid(); pid>1; ) {
				pid = parent_pid(pid);
				switch (vim(pid)) {
					case -1:
						exit_code = EXIT_FAILURE+4;
						pid = -1;
						break;
					case 1:
						exit_code = write_out(argv[1]);
						pid = -1;
						break;
				}
			}
	}
	return exit_code;
}
