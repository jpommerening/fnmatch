#!/bin/sh

TEST_VOID="^ *TEST_VOID(\(.*\)) *;"
TEST_DATA="^ *TEST_DATA(\(.*\)\,.*) *;"

TEST_EXTERN="extern const test_item_t TEST_UID(\1);"
TEST_REF="\&TEST_UID(\1),"

echo "#include \"test.h\""

sed "s/${TEST_VOID}/${TEST_EXTERN}/p;s/${TEST_DATA}/${TEST_EXTERN}/p;d" "$@"

echo "const test_item_t* tests[] = {"

sed "s/${TEST_VOID}/${TEST_REF}/p;s/${TEST_DATA}/${TEST_REF}/p;d" "$@"

echo "NULL"
echo "};"
