/*
 * vbfConfig.c -- simple program that spits out configuration options for
 *                the compiler and linker.
 * by Filip Pizlo, 2003, 2004
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *usage_msg=
"Usage: vbfConfig <commands> ...\n"
"Where the available commands are:\n"
"  --cppflags     Lists the flags you would want to pass to a C preprocessor\n"
"                 when compiling a program that uses VBF.\n"
"  --cxxflags     Lists the flags you would want to pass to a C++ compiler when\n"
"                 compiling a program that uses VBF.\n"
"  --ldflags      Lists the flags you would want to pass to the linker when\n"
"                 linking a program that uses VBF.\n"
"  --libs         Lists the library link commands that you would want to pass to\n"
"                 the linker when linking a program that uses VBF.\n"
"  --prefix       Prints the VBF install prefix.\n"
"  --version      Prints the VBF version that this program corresponds to.\n";

void usage() {
    fprintf(stderr,"%s",usage_msg);
    exit(1);
}

int main(int c,char **v) {
    int i;
    int print_usage=0;
    
    for (i=1;i<c;++i) {
        if (!strcmp(v[i],"--cppflags")) {
            printf("-I%s %s %s\n",INCLUDE_DIR,XTRA_CPPFLAGS,CPPFLAGS);
        } else if (!strcmp(v[i],"--cxxflags")) {
            printf("%s\n",CXXFLAGS);
        } else if (!strcmp(v[i],"--ldflags")) {
            printf("-L%s %s %s\n",LIB_DIR,XTRA_LDFLAGS,LDFLAGS);
        } else if (!strcmp(v[i],"--libs")) {
            printf("-lVBF %s %s\n",XTRA_LIBS,LIBS);
        } else if (!strcmp(v[i],"--prefix")) {
            printf("%s\n",PREFIX);
        } else if (!strcmp(v[i],"--version")) {
            printf("%s\n",PACKAGE_STRING);
        } else {
            fprintf(stderr,"%s is not an option.\n",v[i]);
            print_usage=1;
        }
    }
    
    if (print_usage) {
        fprintf(stderr,"\n");
    }
    
    if (c<2 || print_usage) {
        usage();
    }
    
    return 0;
}


// **************************************************************************
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 
// 2011, 2012 Purdue University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, 
//  this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
// The views and conclusions contained in the software and documentation are 
// those of the authors and should not be interpreted as representing official
// policies, either expressed or implied, of Purdue University or the VERITAS 
// Collaboration.
// **************************************************************************
