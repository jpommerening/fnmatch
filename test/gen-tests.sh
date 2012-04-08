#!/bin/sh

TEST_PATTERN="^ *TEST( *\([^ ,]*\).*).*$"

TEST_EXTERN="extern const test_t TEST_UID(\1);"
TEST_REF="\&TEST_UID(\1),"

echo "#include \"test.h\""
echo "#include <stddef.h>"

sed "s/${TEST_PATTERN}/${TEST_EXTERN}/p;d" "$@"

echo "const test_t* tests[] = {"

sed "s/${TEST_PATTERN}/${TEST_REF}/p;d" "$@"

echo "NULL,"
echo "};"
