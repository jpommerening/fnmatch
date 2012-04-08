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

uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

ifdef MSVC
uname_S := MINGW
endif

CPPFLAGS += -Iinclude

ifneq (,$(findstring MINGW,$(uname_S)))
include config-mingw.mk
else
include config-unix.mk
endif

TESTS = test/test-*.c
BENCHMARKS = test/benchmark-*.c

all: fnmatch.a

test/gen-tests.c: test/gen-tests.sh $(TESTS)
	test/gen-tests.sh $(TESTS) > test/gen-tests.c

test/run-tests$(E): test/*.h test/run-tests.c test/gen-tests.c test/test.c $(TESTS) fnmatch.a
	$(CC) $(CPPFLAGS) -o test/run-tests test/run-tests.c test/gen-tests.c test/test.c \
	  $(TESTS) fnmatch.a

.PHONY: clean clean-platform distclean distclean-platform test bench

test: test/run-tests$(E)
	test/run-tests

bench: test/run-benchmarks$(E)
	test/run-benchmarks

clean: clean-platform
	$(RM) -f src/*.o *.a test/run-tests$(E) test/run-benchmarks$(E)

distclean: distclean-platform
	$(RM) -f src/*.o *.a test/run-tests$(E) test/run-benchmarks$(E)
