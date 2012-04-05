#!/bin/sh

echo "#include \"test.h\""

sed 's/TEST_DATA(\(.*\)\,.*).*;/extern const test_item_t TEST_UID(\1);/p;d' "$@"

echo "const test_item_t* tests[] = {"

sed 's/TEST_DATA(\(.*\)\,.*).*;/\&TEST_UID(\1),/p;d' "$@"

echo "NULL"
echo "};"
