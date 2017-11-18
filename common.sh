#!/bin/bash
ignored="-Wno-variadic-macros -Wno-old-style-cast -Wno-missing-prototypes -Wno-gnu-zero-variadic-macro-arguments -Wno-c++98-compat-pedantic -lc++ -Wc++14-binary-literal -std=c++11 -Wno-writable-strings -Wno-documentation -Wno-global-constructors -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-missing-braces -Wno-overlength-strings -Wno-cast-align -Wno-switch-enum -Wno-gnu-empty-initializer -Wno-char-subscripts -Wno-implicit-fallthrough" # -fno-stack-protector"
temp_ignored="-Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-double-promotion -Wno-padded"
flags="-Werror -Weverything" # -Wpadded"
common="$flags $ignored $temp_ignored"
debug="-ggdb -D DEVELOPMENT"
release="-Ofast"

if [ -z ${engine_dir} ]; then
	echo "engine_dir is not set";
else
	fiber_bin="$engine_dir/utils/fibers/out/jump_fcontext.o $engine_dir/utils/fibers/out/make_fcontext.o"
	echo "engine_dir is set, setting fiber_bin"
fi
