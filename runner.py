import os
import subprocess
import sys
from datetime import datetime


iters = 5


def experiment1():
    # print("Experiment 1")
    print('iter,distribution,queries,strategy,domain,leaf size,range size,avg,min,max,tree time,update time,additional time,results separation,query execution,total time')

    strategies = [
        'lazy',
        'eager',
        'extra',
        # 'original',
    ]
    queries = [
        10,
        100,
        1000,
        10000,
        100000,
    ]
    leaf_size = 100000
    range_size = 100000
    domain = 1000000

    for strategy in strategies:
        for num_queries in queries:
            for i in range(iters):
                command = f"./main.out --iter={i} --strategy={strategy} --queries={num_queries} --key_domain_size={domain} --leaf_size={leaf_size} --range_size={range_size}"
                res = subprocess.getoutput(command)
                print(res)
                # os.system(command)


def experiment4():
    distribution = "zipf"
    strategies = [
        'lazy',
        'eager',
        'extra',
        # 'original',
    ]
    print("Experiment 4")
    queries = [
        100000,
    ]

    for strategy in strategies:
        for query_size in queries:
            for i in range(iters):
                # print(datetime.now())
                command = "./main.out --strategy=%s --distribution=zipf --queries=%d --key_domain_size=1000000 --leaf_size=100000 --range_size=1000" % (strategy, query_size)
                sys.stdout.write(command)
                os.system(command)


if __name__ == '__main__':
    # experiment4()
    experiment1()
