cmake_minimum_required(VERSION 3.14.0)

add_library(chess
  SHARED
  piece.hpp piece.cpp
  position.hpp position.cpp
  types.hpp
  evaluate.hpp evaluate.cpp
  )
