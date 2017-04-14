#!/bin/bash
cd reload
clang -ggdb parse_data_layout.cpp -o parse_data_layout
./parse_data_layout ../source/app.cpp ../generated/reload_data_layout.generated.cpp
cd ..
