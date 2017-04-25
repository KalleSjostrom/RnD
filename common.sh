#!/bin/bash
ignored="-Wno-variadic-macros -Wno-old-style-cast -Wno-missing-prototypes -Wno-gnu-zero-variadic-macro-arguments -Wno-c++11-long-long -Wno-c++98-compat-pedantic -Wno-c++11-compat -Wno-documentation -Wno-global-constructors -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-missing-braces -Wno-overlength-strings -Wno-cast-align -Wno-switch-enum" # -fno-stack-protector"
flags="-Werror -Weverything -Wpadded"
common="$flags $ignored"

debug="-ggdb"
release="-Ofast"
