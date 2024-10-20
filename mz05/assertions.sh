#!/usr/bin/bash

SOLUTION="bin/ex5"
REFERENCE="bin/ex5-ref"

assert_same() {
    RESULT="$($SOLUTION $1 $2)"
    EXPECTED="$($REFERENCE $1 $2)" 

    if [[ ! $RESULT = $EXPECTED ]]; then
        echo "TEST relativize_path($1, $2) FAILED: \`'$RESULT' == '$EXPECTED'\`"
    fi
}

assert_same / /
assert_same /a /a
assert_same /a/b /a/b
assert_same /a/b/c /a/b/c
assert_same / /a
assert_same / /a/b
assert_same / /a/b/c
assert_same /a /a/b
assert_same /a /a/b/c
assert_same /a/b /a/b/c
assert_same /a /
assert_same /a/b /
assert_same /a/b/c /
assert_same /a/b/c/d /
assert_same /a/b /a
assert_same /a/b/c /a
assert_same /a/b/c/d /a
assert_same /a/b/c /a/b
assert_same /a/b/c/d /a/b
assert_same /a/b/c/d /a/e/f
assert_same /a/b/c/d /e/f/g
assert_same "" ""
assert_same "" a
assert_same "" a/b
assert_same a ""
assert_same a/b ""

