#!/bin/bash
clang -Ofast gl_parser.cpp -o gl_parser
clang -ggdb gl_parser.cpp -o gl_parser_debug
