/*
 *
 *  TOTP: Time-Based One-Time Password Algorithm
 *  Copyright (c) 2017, fmount <fmount9@autistici.org>
 *
 *  This software is distributed under MIT License
 *
 *  Compute the hmac using openssl library.
 *  SHA-1 engine is used by default, but you can pass another one,
 *
 *  e.g EVP_md5(), EVP_sha224, EVP_sha512, etc
 *
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "plist.h"
#include "parser.h"

PROVIDER split_str(char *spl, char delim)
{
    char *tmp_name;
    char *tmp_secret;
    PROVIDER p;
    size_t count = 0;

    size_t totlen = strlen(spl) - 2;

    //Get break point
    do {
        count++;
    } while (spl[count] != delim);

    tmp_name = (char *) malloc(count * sizeof(char) + 1);
    tmp_secret = (char *) malloc((totlen-count) * sizeof(char) + 1);

    /*
     * Get first part of the string
     */
    memcpy(tmp_name, spl, count);
    tmp_name[count] = '\0';
    /*
     * Get second part of the string
     */
    memcpy(tmp_secret, spl+(count+1), (totlen-count));
    tmp_secret[(totlen-count)] = '\0';

#ifdef DEBUG
    printf("[GOT LEN]: %ld\n", strlen(spl));
    printf("[PROVIDER SECTION]: %ld characters\n", count);
    printf("[GOT NAME]: %s\n", tmp_name);
    printf("[SECRET SECTION]: %ld\n", (strlen(spl)-count+1));
    printf("[GOT SECRET]: %s\n", tmp_secret);
#endif

    p = (PROVIDER) {
        .pname = tmp_name,
        .psecret = tmp_secret,
        .otpvalue = NULL
    };

    return p;
}

void process_provider(NODE **plist, char *line)
{
    PROVIDER p;
    p = split_str(line, ':');
    push(plist, p.pname, p.psecret, p.otpvalue);
}

// reimplementation of getline() to stop the compiler from crying (<https://stackoverflow.com/a/76142157>)
ssize_t getline(char **restrict buffer, size_t *restrict size, FILE *restrict fp) {
    register int c;
    register char *cs = NULL;

    if (cs == NULL) {
        register int length = 0;
        while ((c = getc(fp)) != EOF) {
            cs = (char *)realloc(cs, ++length+1);
            if ((*(cs + length - 1) = c) == '\n') {
                *(cs + length) = '\0';
                *buffer = cs;
                break;
            }
        }
        return (ssize_t)(*size = length);
    } else {
        while (--(*size) > 0 && (c = getc(fp)) != EOF) {
            if ((*cs++ = c) == '\n')
                break;
        }
        *cs = '\0';
    }
    return (ssize_t)(*size=strlen(*buffer));
}

void load_providers(char *fname)
{
    FILE *f;
    size_t len = 1024;

    if (fname == NULL)
        exit(ENOENT);
    f = fopen(fname, "r");
    if (f == NULL)
        exit(ENOENT);
    char *line = NULL;

    while (getline(&line, &len, f) != -1) {
        if (line[0] != '#')
            process_provider(&provider_list, line);
    }

    free(line);
    fclose(f);
}
