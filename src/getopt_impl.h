#ifndef CHAD_INTERPRETER_GETOPT_IMPL_H
#define CHAD_INTERPRETER_GETOPT_IMPL_H

#include <stdio.h>
#include <string.h>

/*
* Copyright © 2005-2014 Rich Felker, et al.
* 
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
* 
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

char* optarg;
int optind = 1, opterr = 1, optopt, __optpos, optreset = 0;

#define optpos __optpos

static void __getopt_msg(const char* a, const char* b, const char* c, size_t l) {
    FILE* f = stderr;
#if !defined(WIN32) && !defined(_WIN32)
    flockfile(f);
#endif
    fputs(a, f);
    fwrite(b, strlen(b), 1, f);
    fwrite(c, 1, l, f);
    fputc('\n', f);
#if !defined(WIN32) && !defined(_WIN32)
    funlockfile(f);
#endif
}

int getopt(int argc, char* const argv[], const char* optstring) {
    int i, c, d;
    int k, l;
    char* optchar;

    if (!optind || optreset) {
        optreset = 0;
        __optpos = 0;
        optind = 1;
    }

    if (optind >= argc || !argv[optind])
        return -1;

    if (argv[optind][0] != '-') {
        if (optstring[0] == '-') {
            optarg = argv[optind++];
            return 1;
        }
        return -1;
    }

    if (!argv[optind][1])
        return -1;

    if (argv[optind][1] == '-' && !argv[optind][2])
        return optind++, -1;

    if (!optpos) optpos++;
    c = argv[optind][optpos], k = 1;
    optchar = argv[optind] + optpos;
    optopt = c;
    optpos += k;

    if (!argv[optind][optpos]) {
        optind++;
        optpos = 0;
    }

    if (optstring[0] == '-' || optstring[0] == '+')
        optstring++;

    i = 0;
    d = 0;
    do {
        d = optstring[i], l = 1;
        if (l > 0) i += l;
        else
            i++;
    } while (l && d != c);

    if (d != c) {
        if (optstring[0] != ':' && opterr)
            __getopt_msg(argv[0], ": unrecognized option: ", optchar, k);
        return '?';
    }
    if (optstring[i] == ':') {
        if (optstring[i + 1] == ':') optarg = 0;
        else if (optind >= argc) {
            if (optstring[0] == ':') return ':';
            if (opterr) __getopt_msg(argv[0],
                                     ": option requires an argument: ",
                                     optchar, k);
            return '?';
        }
        if (optstring[i + 1] != ':' || optpos) {
            optarg = argv[optind++] + optpos;
            optpos = 0;
        }
    }
    return c;
}

#endif
