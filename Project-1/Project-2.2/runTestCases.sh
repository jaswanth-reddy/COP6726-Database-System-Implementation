#!/bin/bash

./test.out < tc1.txt | tail -n10 > output1.txt
echo "****************************************************************************************************************************************************************************************" >> output1.txt
./test.out < tc2.txt | tail -n3 >> output1.txt
echo "****************************************************************************************************************************************************************************************" >> output1.txt
./test.out < tc3.txt | tail -n10 >> output1.txt
echo "****************************************************************************************************************************************************************************************" >> output1.txt
