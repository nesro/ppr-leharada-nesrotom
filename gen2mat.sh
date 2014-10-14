#!/bin/bash
f=$1
sf=${1%%.*}

cat <(echo "Export[\"$sf.png\",AdjacencyGraph[{") <(sed 1d $f | sed 's/0/0,/g' \
    | sed 's/1/1,/g' | sed 's/^/{/g' | sed 's/,$/},/g' | tr -d '\n' | \
    sed 's/,$//g') <(echo "},VertexLabels->{1->"x"}]]") | tr -d '\n'
echo " "
