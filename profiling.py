import os
import subprocess
import sys
from datetime import datetime


def runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters, random_range_size='false'):
    print('iter,strategy,num queries,domain size,leaf size,range size,distribution,avg node length,# leaf nodes,max tree depth,mapping queries nodes,mapping insert ops,mapping transfer ops,mapping share ops,mapping merge ops,mapping insert time,mapping transfer time,mapping share time,mapping merge time,tree building time,mapping total time,additional tree time,total time')

    for strategy in strategies:
        for query_size in queries:
            for leaf_size in leaf_sizes:
                for range_size in range_sizes:
                    for i in range(iters):
                        command = f"./profiling.out --distribution={distribution} --iter={i} --strategy={strategy} --queries={query_size} --key_domain_size={domain} --leaf_size={leaf_size} --range_size={range_size} --random_range_size={random_range_size}"
                        # sys.stdout.write(command)
                        os.system(command)


def experiment_1a():
    iters = 5

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
    ]
    leaf_sizes = [
        100000,
    ]
    range_sizes = [
        100000,
    ]
    distribution = 'normal'
    domain = 1000000

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_3a():
    iters = 5

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        1000000,
    ]
    leaf_sizes = [
        1000,
        10000,
        100000,
        250000,
    ]
    range_sizes = [
        100000,
    ]
    distribution = 'normal'
    domain = 1000000

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_3b():
    iters = 5

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        1000000,
    ]
    leaf_sizes = [
        1000,
        10000,
        100000,
        250000,
    ]
    range_sizes = [
        10000,
    ]
    distribution = 'normal'
    domain = 1000000

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_3c():
    iters = 5

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        1000000,
    ]
    leaf_sizes = [
        100000,
        250000,
        750000,
    ]
    range_sizes = [
        10000,
    ]
    distribution = 'normal'
    domain = 1000000

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters, random_range_size='true')

if __name__ == '__main__':
    print("############# 3A #################")
    experiment_3a()
    # print("############# 3B #################")
    # experiment_3b()
