#pragma once

#include "pebble.h"

#define LENGTH 12
#define WIDTH 3
#define TAIL true

static const GPathInfo HORIZONTAL_CELL = {
  6, (GPoint []){
    {1, 0},
    {11, 0},
    {12, -1},
    {11, -2},
    {1, -2},
    {0, -1}
  }
};

static const GPathInfo VERTICAL_CELL = {
  6, (GPoint []){
    {1, 0},
    {2, -1},
    {2, -11},
    {1, -12},
    {0, -11},
    {0, -1}
  }
};

static const bool ILLUMINATION_TABLE[10][7] = {
  //   a,    b,   c,     d,    e,    f,    g 
  { true, true, true, true, true, true,false},   // 0
  {false, true, true,false,false,false,false},   // 1          aaaa
  { true, true,false, true, true,false, true},   // 2         f    b
  { true, true, true, true,false,false, true},   // 3         f    b
  {false, true, true,false,false, true, true},   // 4          gggg
  { true,false, true, true,false, true, true},   // 5         e    c
  { TAIL,false, true, true, true, true, true},   // 6         e    c
  { true, true, true,false,false,false,false},   // 7          dddd 
  { true, true, true, true, true, true, true},   // 8
  { true, true, true, TAIL,false, true, true},   // 9
};