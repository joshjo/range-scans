import os
import subprocess
import sys
from datetime import datetime


def runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters, random_range_size='false', min_range_size=0, max_range_size=0, percentage_point_queries=0):
    print('iter,strategy,distribution,num queries,domain size,leaf size,range size,range size min, range size avg, range size max,avg node length,# leaf nodes,max tree depth,mapping queries nodes,mapping insert ops,mapping transfer ops,mapping share ops,mapping merge ops,mapping insert time,mapping transfer time,mapping share time,mapping merge time,tree building time,mapping total time,additional tree time,total time')

    for strategy in strategies:
        for query_size in queries:
            for leaf_size in leaf_sizes:
                for range_size in range_sizes:
                    for i in range(iters):
                        command = f"./profiling.out --distribution={distribution} --iter={i} --strategy={strategy} --queries={query_size} --key_domain_size={domain} --leaf_size={leaf_size} --range_size={range_size} --random_range_size={random_range_size} --min_range_size={min_range_size} --max_range_size={max_range_size} --percentage_point_queries={percentage_point_queries}"
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
        10000000,
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


def experiment_1b():
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
    ]
    leaf_sizes = [
        'max_range',
    ]
    range_sizes = [
        10,
        100,
        1000,
    ]
    distribution = 'normal'
    domain = 1000000

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_3a():
    print("############## 3 A #################")
    iters = 5

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        100000,
    ]
    leaf_sizes = [
        1000,
        10000,
        100000,
        250000,
        500000,
    ]
    range_sizes = [
        100000,
    ]
    distribution = 'normal'
    domain = 1000000

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_3b():
    print("############## 3 B #################")
    iters = 5

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        100000,
    ]
    leaf_sizes = [
        1000,
        10000,
        100000,
        250000,
        500000,
    ]
    range_sizes = [
        10000,
    ]
    distribution = 'normal'
    domain = 1000000

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_3c():
    print("############## 3 C #################")
    iters = 5

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        100000,
    ]
    leaf_sizes = [
        "max_range",
        500000,
        250000,
        100000,
        10000,
        1000,
    ]
    range_sizes = [
        1000000,
    ]
    distribution = 'normal'
    domain = 1000000

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters, random_range_size='true', min_range_size=10000, max_range_size=100000)


def experiment_61():
    print("############## 6.1 A #################")
    iters = 5

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        100000,
    ]
    leaf_sizes = [
        "max_range",
    ]
    range_sizes = [
        10,
        100,
        1000,
        10000,
        100000,
    ]
    distribution = 'normal'
    domain = 1000000

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters, percentage_point_queries=50)


def experiment_62():
    print("############## 6.2 A #################")
    iters = 5

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        100000,
    ]
    leaf_sizes = [
        "max_range",
    ]
    range_sizes = [
        10,
        100,
        1000,
        10000,
        100000,
    ]
    distribution = 'normal'
    domain = 1000000

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters, percentage_point_queries=80)


if __name__ == '__main__':
    experiment_1b()
    # experiment_3a()
    # experiment_3b()
    # experiment_3c()
    # experiment_61()
    # experiment_62()
