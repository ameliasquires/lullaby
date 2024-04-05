#!/bin/bash
for ((i=1; i < 500; i++))  do
echo $i
curl localhost:8090 -F "a=@llib.so"
done
