# MSU TECHARENA-2024
My solution for techarena-2024. Scored ```1899670``` on open-closed testsets.
# Build
CMake generates two solutions I tried to solve the competition, wich are ```naive.cpp``` and ```smart.cpp```.
## Example of usage
```sh
./smart samples/open.txt samples/output.txt
```
# Solution
## Basic idea
The problem is NP-hard, so I tried to find a solution that is in constant times worse than the best one. I used intervals of equal length and a slightly modified round-robin and greedy schedulers for the naive and smart approaches.
## Naive
Lets sort the users according to their ```rbNeed```s (see task description) and split them in groups of ```L```. Lets use the same group of users for each new interval until the sum of its ```rbNeed```s is the greatest, otherwise lets go to the next group.
## Smart (not enough yet)
Lets try to improve the previous approach and always peek L users with maximum ```rbNeed```s. The only problem with both approaches is that we cannot grant users disjoint intervals, so we wont consider them on next iterations of our algorithm.
## Improvement
Given the last statement, we can put removed (from consideration) users to a separate group and try to add them to  intervals that have less than L users. I got TL on that one and didn't have enough time to fix it