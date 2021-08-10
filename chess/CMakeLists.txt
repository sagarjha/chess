cmake_minimum_required(VERSION 3.14.0)

add_library(chess
  SHARED
  board.hpp board.cpp
  evaluate.hpp evaluate.cpp
  piece.hpp piece.cpp
  position.hpp position.cpp
  types.hpp
  )
