import os
import subprocess
import sys
from datetime import datetime


iters = 5

strategies = [
    'lazy',
    'eager',
    'extra',
    'original',
]


def experiment1():
    print("Experiment 1")
    queries = [
        10,
        100,
        1000,
        10000,
        100000,
    ]


    for strategy in strategies:
        for query_size in queries:
            for i in range(iters):
                print(datetime.now())
                command = "./main.out --strategy=%s --queries=%d --key_domain_size=1000000 --leaf_size=100000 --range_size=1000" % (strategy, query_size)
                sys.stdout.write(command)
                os.system(command)


def experiment4():
    distribution = "zipf"
    print("Experiment 4")
    queries = [
        100000,
    ]

    for strategy in strategies:
        for query_size in queries:
            for i in range(iters):
                print(datetime.now())
                command = "./main.out --strategy=%s --distribution=zipf --queries=%d --key_domain_size=1000000 --leaf_size=100000 --range_size=1000" % (strategy, query_size)
                sys.stdout.write(command)
                os.system(command)


if __name__ == '__main__':
    experiment4()


