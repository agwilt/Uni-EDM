#!/usr/bin/env python3

import sys
import random

if (len(sys.argv)) < 3: exit(1)

n = int(sys.argv[1])
f = open(sys.argv[2], mode='w')

f.write(sys.argv[1] + '\n')

# outer circle first
for i in range(n):
    f.write("%d %d\n" %(i, (i+1)%n))

length = 6

"""
# 100 random cycles
for i in range(6):
    start = random.randint(0, n-1)
    next_node = node = random.randint(0, n-1)
    for j in range(length):
        while(next_node == node):
            next_node = random.randint(0, n-1)
        f.write("%d %d\n" % (node, next_node))
    if node != start:
        f.write("%d %d\n" % (node, start))
        """

f.close()
