#!/usr/bin/env bash

for numThreads in {1..512} 
do
  echo -n "$numThreads: "
  ./mandelbrot_ispc -t $numThreads -v 2 \
    | rg -o '\d+\.\d+x'
done