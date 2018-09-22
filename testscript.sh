#!/bin/bash

mkdir -p in out

for file in $(ls -rS in);
do
    echo "Testing input from $file..."
    bin/setcover in/$file > out/$file
    echo "Done."
done