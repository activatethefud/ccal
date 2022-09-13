cmake_minimum_required(VERSION 3.5)

project(ccal LANGUAGES C)

set(CMAKE_C_CLANG_TIDY "clang-tidy; -checks=*")

add_executable( ccal
        llist.c
        llist.h
        random.c
        random.h
        types.h
        tokenizer.c
        tokenizer.h
        main.h
        main.c
)
