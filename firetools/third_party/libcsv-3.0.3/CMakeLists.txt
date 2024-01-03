cmake_minimum_required(VERSION 3.9)

project(csv VERSION 3.0.3 DESCRIPTION "csv")
add_library(csv STATIC
    libcsv.c
    csv.h
)
