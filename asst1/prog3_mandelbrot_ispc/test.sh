#!/usr/bin/env bash

for numTasks in {1..64} 
do
  echo -n "uniform int nTasks = $numTasks;" > ntasks.config.in
  make clean > /dev/null 2>&1
  make -j > /dev/null 2>&1
  make -j > /dev/null 2>&1
  echo -n "$numTasks: "
  ./mandelbrot_ispc -v 2 -t \
    | rg -o '\d+\.\d+x.+task' | awk '{ print $1 }'
done