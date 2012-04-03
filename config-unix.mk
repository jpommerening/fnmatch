# Copyright (c) 2012, Jonas Pommerening <jonas.pommerening@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met: 
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer. 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution. 
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

E =
CSTDFLAG = --std=c89 -pedantic -Wall -Wextra
CFLAGS += -g
LINKFLAGS = 

CPPFLAGS += -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64

OBJS += src/pattern.o
OBJS += src/context.o
OBJS += src/scanner.o
OBJS += src/util.o
OBJS += src/compiler.o
OBJS += src/vm.o
OBJS += src/posix.o

ifeq (SunOS,$(uname_S))
CPPFLAGS +=
LINKFLAGS +=
OBJS +=
endif

ifeq (Darwin,$(uname_S))
CPPFLAGS +=
LINKFLAGS +=
OBJS +=
endif

ifeq (Linux,$(uname_S))
endif

ifeq (FreeBSD,$(uname_S))
endif

ifeq (DragonFly,$(uname_S))
endif

ifeq (NetBSD,$(uname_S))
endif

ifeq (OpenBSD,$(uname_S))
endif

ifneq (,$(findstring CYGWIN,$(uname_S)))
endif

fnmatch.a: $(OBJS)
	$(AR) rcs fnmatch.a $(OBJS)

src/%.o: src/%.c include/fnmatch.h src/internal.h
	$(CC) $(CSTDFLAG) $(CPPFLAGS) -Isrc $(CFLAGS) -c $< -o $@

clean-platform:
	-#

distclean-platform:
	-#
