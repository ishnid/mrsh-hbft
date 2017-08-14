/* 
 * File:   util.h
 *
 * Created on 5. Juni 2012, 14:38
 */
#ifndef HELPER_H
#define	HELPER_H
#include <stdio.h>
#include <stdint.h>
#include "../header/config.h"

typedef struct {
    unsigned char *data;
    unsigned long filesize;
    char* filename;
} FILE_CONTENTS;

FILE_CONTENTS   *read_file(char*);

void            destroy_file_contents(FILE_CONTENTS *);
FILE    		*getFileHandle(char *filename);
unsigned int	find_file_size(FILE *fh);

bool is_file(const char* path);
bool is_dir(const char* path);
unsigned int find_file_size(FILE *fh);

#endif	/* HELPER_H */

