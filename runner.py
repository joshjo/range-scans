import os
import subprocess
import sys
from datetime import datetime


# def measure():
#     print('iter,strategy,distribution,num queries,domain size,leaf size,range size,range size min, range size avg, range size max,avg node length,# leaf nodes,max tree depth,mapping queries nodes,mapping insert ops,mapping transfer ops,mapping share ops,mapping merge ops,mapping insert time,mapping transfer time,mapping share time,mapping merge time,tree building time,mapping total time,additional tree time,mapping + tree building time,exec time,post filtering time,total time')

#     iters = 3
#     strategies = [
#         'additional',
#     ]
#     queries = [
#         i for i in range(10, 100000, 1000)
#     ]
#     # queries = [10, 100, 1000]
#     distribution = 'normal'
#     leaf_size = 100
#     range_size = 100
#     domain = 100000000

#     for num_queries in queries:
#         for strategy in strategies:
#             for i in range(iters):
#                 command = f"./main.out --distribution={distribution} --iter={i} --strategy={strategy} --queries={num_queries} --key_domain_size={domain} --leaf_size={leaf_size} --range_size={range_size}"
#                 os.system(command)




def experiment1():
    print('iter,strategy,distribution,num queries,domain size,leaf size,range size,range size min, range size avg, range size max,avg node length,# leaf nodes,max tree depth,mapping queries nodes,mapping insert ops,mapping transfer ops,mapping share ops,mapping merge ops,mapping insert time,mapping transfer time,mapping share time,mapping merge time,tree building time,mapping total time,additional tree time,mapping + tree building time,exec time,post filtering time,total time')

    iters = 5
    strategies = [
        'lazy',
        'eager',
        'additional',
    ]
    queries = [
        # 10,
        # 100,
        # 1000,
        # 10000,
        # 100000,
        # 1000000,
        10000000,
    ]
    distribution = 'normal'
    leaf_size = 100000
    range_size = 100000
    domain = 1000000

    for num_queries in queries:
        for strategy in strategies:
            for i in range(iters):
                command = f"./main.out --distribution={distribution} --iter={i} --strategy={strategy} --queries={num_queries} --key_domain_size={domain} --leaf_size={leaf_size} --range_size={range_size}"
                os.system(command)


def experiment4():
    iters = 5
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

    for query_size in queries:
        for strategy in strategies:
            for i in range(iters):
                # print(datetime.now())
                command = "./main.out --strategy=%s --distribution=zipf --queries=%d --key_domain_size=1000000 --leaf_size=100000 --range_size=1000" % (strategy, query_size)
                sys.stdout.write(command)
                os.system(command)


def profiling():
    iters = 5
    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000,
    ]
    leaf_sizes = [
        # 100,
        # 1000,
        100000,
        250000,
        500000,
        750000,
        1000000,
    ]
    range_sizes = [
        # 100,
        1000,
        10000,
        # 100000,
    ]
    distribution = 'normal'
    leaf_size = 100000
    range_size = 100000
    domain = 1000000
    query_size = 10000

    # query_size = queries[0]

    print('iter,strategy,num queries,domain size,leaf size,range size,distribution,avg node length,# leaf nodes,max tree depth,queries indexed,insert ops,transfer ops,share ops,merge ops,insert time,transfer time,share time,merge time,mapping time,tree time,total time')

    for strategy in strategies:
        # for query_size in queries:
        for leaf_size in leaf_sizes:
        # for range_size in range_sizes:
            for i in range(iters):
                # print(datetime.now())
                command = f"./profiling.out --distribution={distribution} --iter={i} --strategy={strategy} --queries={query_size} --key_domain_size={domain} --leaf_size={leaf_size} --range_size={range_size}"
                # sys.stdout.write(command)
                os.system(command)


if __name__ == '__main__':
    # measure()
    experiment1()
    # profiling()
