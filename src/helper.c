
#include <stdio.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <openssl/md5.h>
#include <limits.h>
#include "../header/config.h"
#include "../header/helper.h"


bool is_file(const char *path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISREG(buf.st_mode);
}

bool is_dir(const char *path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISDIR(buf.st_mode);
}

void fatal_error(char *message) {
    fprintf(stderr, "%s \n", message);
    exit(-1);
}

void destroy_file_contents(FILE_CONTENTS *fc) {
    free(fc->filename);
    if (fc->data != NULL)
        free(fc->data);
    free(fc);
}


FILE_CONTENTS *read_file(char *filename) {

    FILE *handle = getFileHandle(filename);

    FILE_CONTENTS *to_return = calloc(1, sizeof(FILE_CONTENTS));

    if (to_return == NULL) {
        fprintf(stderr, "Failed to allocate FILE_CONTENTS\n");
        exit(1);
    }

    to_return->filename = malloc(sizeof(char) * (strlen(filename) + 1));
    if (to_return->filename == NULL) {
        fprintf(stderr, "Failed to allocate file name for FILE_CONTENTS\n");
        exit(1);
    }

    strcpy(to_return->filename, filename);
    to_return->filesize = find_file_size(handle);

    if (to_return->filesize > 0) {

        //allocate buffer
        if ((to_return->data = (unsigned char *) malloc(sizeof(unsigned char) * to_return->filesize)) == NULL) {
            fprintf(stderr, "Failed to allocate space for ");
        }

        // bytes_read stores the number of characters we read from our file
        fseek(handle, 0L, SEEK_SET);
        if ((fread(to_return->data, sizeof(unsigned char), to_return->filesize, handle)) == 0)
            return NULL;
    } else {
        destroy_file_contents(to_return);
        return NULL;
    }
    fclose(handle);

    return to_return;
}

FILE *getFileHandle(char *filename) {
    FILE *handle = NULL;

    if ((handle = fopen(filename, "r")) == NULL) {
        fprintf(stderr, "[*] Error in opening file: %s\n", filename);
        exit(-1);
    }

    return handle;
}

unsigned int find_file_size(FILE *fh) {
    unsigned int size;
    if (fh != NULL) {
        if (fseek(fh, 0, SEEK_END)) {
            return -1;
        }
        size = ftell(fh);
        //printf("FILE SIZE: %d \n", size);
        return size;
    }
    return -1;
}


