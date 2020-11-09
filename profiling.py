import os
import subprocess
import sys
from datetime import datetime


# universal_domain = 1000000000
universal_domain = 10**9
# universal_domain = 1000000
# universal_iters = 5
universal_iters = 1


def runner(distributions, domain, strategies, queries, leaf_sizes, range_sizes, iters, random_range_size='false', min_range_size=0, max_range_size=0, percentage_point_queries=0):
    print('iter,strategy,distribution,num queries,domain size,leaf size,range size,range size min, range size avg, range size max,avg node length,# leaf nodes,max tree depth,mapping queries nodes,mapping insert ops,mapping transfer ops,mapping share ops,mapping merge ops,mapping insert time,mapping transfer time,mapping share time,mapping merge time,tree building time,mapping total time,additional tree time,mapping + tree building time,total time')

    for distribution in distributions:
        for strategy in strategies:
            for query_size in queries:
                for leaf_size in leaf_sizes:
                    for range_size in range_sizes:
                        for i in range(iters):
                            command = f"./profiling.out --pre_partitioning --distribution={distribution} --iter={i} --strategy={strategy} --queries={query_size} --key_domain_size={domain} --leaf_size={leaf_size} --range_size={range_size} --random_range_size={random_range_size} --min_range_size={min_range_size} --max_range_size={max_range_size} --percentage_point_queries={percentage_point_queries}"
                            os.system(command)


def runner_grouped(distributions, domain, strategies, queries, leaf_sizes, range_sizes, iters, random_range_size='false', min_range_size=0, max_range_size=0, percentage_point_queries=0):
    print('iter,strategy,distribution,num queries,domain size,leaf size,range size,range size min, range size avg, range size max,avg node length,# leaf nodes,max tree depth,mapping queries nodes,mapping insert ops,mapping transfer ops,mapping share ops,mapping merge ops,mapping insert time,mapping transfer time,mapping share time,mapping merge time,tree building time,mapping total time,additional tree time,mapping + tree building time,total time')

    for distribution in distributions:
        for strategy in strategies:
            for query_size in queries:
                for (leaf_size, range_size) in zip(leaf_sizes, range_sizes):
                    for i in range(iters):
                        command = f"./profiling.out --pre_partitioning --distribution={distribution} --iter={i} --strategy={strategy} --queries={query_size} --key_domain_size={domain} --leaf_size={leaf_size} --range_size={range_size} --random_range_size={random_range_size} --min_range_size={min_range_size} --max_range_size={max_range_size} --percentage_point_queries={percentage_point_queries}"
                        os.system(command)


def experiment_1a():
    print("############## 1 A #################")
    iters = universal_iters

    strategies = [
        'additional',
        # 'lazy',
        # 'eager',
    ]
    queries = [
        # 10,
        # 100,
        1000,
        10000,
        # 100000,
        # 1000000,
        # 10000000,
    ]
    leaf_sizes = [
        100000,
    ]
    range_sizes = [
        100000,
    ]
    distributions = ['normal']
    domain = universal_domain

    runner(distributions, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_1b():
    print("############## 1 B #################")
    iters = universal_iters

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
        10000,
        100000,
        1000000,
        10000000,
    ]
    distribution = 'normal'
    domain = universal_domain

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_2a():
    print("############## 2 A #################")
    iters = universal_iters

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        int(10 ** 5),
    ]
    leaf_sizes = [
        int(universal_domain * 1 / 100),
    ]
    range_sizes = [
        int(universal_domain * 0.001 / 100),
        int(universal_domain * 0.01 / 100),
        int(universal_domain * 0.1 / 100),
        int(universal_domain * 1 / 100),
    ]
    distributions = ['normal']
    domain = universal_domain

    runner(distributions, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_2b():
    print("############## 2 B #################")
    iters = universal_iters

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        int(10 ** 6),
    ]
    leaf_sizes = [
        int(universal_domain * 1 / 100),
    ]
    range_sizes = [
        int(universal_domain * 0.001 / 100),
        int(universal_domain * 0.01 / 100),
        int(universal_domain * 0.1 / 100),
        int(universal_domain * 1 / 100),
    ]
    distributions = ['normal']
    domain = universal_domain

    runner(distributions, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_2c():
    print("############## 2 C #################")
    iters = universal_iters

    strategies = [
        'additional',
        # 'lazy',
        # 'eager',
    ]
    queries = [
        100000,
    ]
    leaf_sizes = [
        int(universal_domain * (10 ** -5) / 100),
        int(universal_domain * (10 ** -4) / 100),
        int(universal_domain * (10 ** -3) / 100),
        int(universal_domain * (10 ** -2) / 100),
        int(universal_domain * (10 ** -1) / 100),
    ]
    range_sizes = [
        int(universal_domain * (10 ** -5) / 100),
        int(universal_domain * (10 ** -4) / 100),
        int(universal_domain * (10 ** -3) / 100),
        int(universal_domain * (10 ** -2) / 100),
        int(universal_domain * (10 ** -1) / 100),
    ]
    distributions = ['normal']
    domain = universal_domain

    runner_grouped(distributions, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_3a():
    print("############## 3 A #################")
    iters = universal_iters

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        100000,
    ]
    leaf_sizes = [
        int(universal_domain * (10 ** -4) / 100),
        int(universal_domain * (10 ** -3) / 100),
        int(universal_domain * (10 ** -2) / 100),
        int(universal_domain * (10 ** -1) / 100),
        int(universal_domain * (10 ** 0) / 100),
    ]
    range_sizes = [
        int(universal_domain * (10 ** -3) / 100),
    ]
    distribution = 'normal'
    domain = universal_domain

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_3b():
    print("############## 3 B #################")
    iters = universal_iters

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        100000,
    ]
    leaf_sizes = [
        int(universal_domain * (10 ** -4) / 100),
        int(universal_domain * (10 ** -3) / 100),
        int(universal_domain * (10 ** -2) / 100),
        int(universal_domain * (10 ** -1) / 100),
        int(universal_domain * (10 ** 0) / 100),
    ]
    range_sizes = [
        int(universal_domain * (10 ** -4) / 100),
    ]
    distributions = ['normal']
    domain = universal_domain

    runner(distributions, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_3c():
    print("############## 3 C #################")
    iters = universal_iters

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        100000,
    ]
    leaf_sizes = [
        int(universal_domain * (10 ** -4) / 100),
        int(universal_domain * (10 ** -3) / 100),
        int(universal_domain * (10 ** -1) / 100),
        int(universal_domain * (10 ** 0) / 100),
        'max_range',
    ]
    range_sizes = [
        0,
    ]
    distributions = ['normal']
    domain = universal_domain

    runner(
        distributions, domain, strategies, queries, leaf_sizes, range_sizes, iters,
        random_range_size='true',
        # min_range_size=int(universal_domain * (10 ** -4) / 100),
        # max_range_size=int(universal_domain * (10 ** -3) / 100),
    )


def experiment_3d():
    print("############## 3 D #################")
    iters = universal_iters

    strategies = [
        'additional',
        # 'lazy',
        # 'eager',
    ]
    queries = [
        100000,
    ]
    leaf_sizes = [
        10**3,
        # 10**4,
        # 10**5,
        # 10**6,
        # 10**7,
        # 10**8,
        # 10**9,
    ]
    range_sizes = [
        10**5,
    ]
    distributions = ['normal']
    domain = universal_domain

    runner(distributions, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_4():
    print("############## 4 #################")
    iters = universal_iters

    strategies = [
        'additional',
        'lazy',
        'eager',
    ]
    queries = [
        1000,
        10000,
        100000,
    ]
    leaf_sizes = [
        int(universal_domain * 1 / 100),
    ]
    range_sizes = [
        int(universal_domain * (10 ** -4) / 100),
    ]
    distributions = ['zipf', 'normal']
    domain = universal_domain

    runner(distributions, domain, strategies, queries, leaf_sizes, range_sizes, iters)


def experiment_6():
    point_queries = [
        50,
        80,
        90,
        99,
    ]
    iters = universal_iters

    strategies = [
        'additional',
        # 'lazy',
        # 'eager',
        # 'qat',
    ]
    queries = [
        10000,
    ]
    leaf_sizes = [
        "max_range",
    ]
    range_sizes = [
        100,
        1000,
        10000,
        100000,
    ]
    distribution = 'normal'
    domain = universal_domain

    for i in point_queries:
        print(f"############## 6 {i} #################")
        runner(
            distribution, domain, strategies, queries,
            leaf_sizes, range_sizes, iters,
            percentage_point_queries=i)


def experiment_61():
    print("############## 6.1 A #################")
    iters = universal_iters

    strategies = [
        # 'additional',
        # 'lazy',
        # 'eager',
        'qat',
    ]
    queries = [
        10000,
    ]
    leaf_sizes = [
        "max_range",
    ]
    range_sizes = [
        100,
        1000,
        10000,
        100000,
    ]
    distribution = 'normal'
    domain = universal_domain

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters, percentage_point_queries=50)


def experiment_62():
    print("############## 6.2 A #################")
    iters = universal_iters

    strategies = [
        # 'additional',
        # 'lazy',
        # 'eager',
        'qat',
    ]
    queries = [
        10000,
    ]
    leaf_sizes = [
        "max_range",
    ]
    range_sizes = [
        100,
        1000,
        10000,
        100000,
    ]
    distribution = 'normal'
    domain = universal_domain

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters, percentage_point_queries=80)


def experiment_63():
    print("############## 6.3 A #################")
    iters = universal_iters

    strategies = [
        'additional',
        # 'lazy',
        # 'eager',
        # 'qat',
    ]
    queries = [
        10000,
    ]
    leaf_sizes = [
        "max_range",
    ]
    range_sizes = [
        100,
        1000,
        10000,
        100000,
    ]
    distribution = 'normal'
    domain = universal_domain

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters, percentage_point_queries=90)


def experiment_64():
    print("############## 6.4 A #################")
    iters = universal_iters

    strategies = [
        'additional',
        # 'lazy',
        # 'eager',
        # 'qat',
    ]
    queries = [
        10000,
    ]
    leaf_sizes = [
        "max_range",
    ]
    range_sizes = [
        100,
        1000,
        10000,
        100000,
    ]
    distribution = 'normal'
    domain = universal_domain

    runner(distribution, domain, strategies, queries, leaf_sizes, range_sizes, iters, percentage_point_queries=99)


if __name__ == '__main__':
    experiment_1a()
    # experiment_1b()
    # experiment_2a()
    # experiment_2b()
    # experiment_2c()
    # experiment_3a()
    # experiment_3b()
    # experiment_3c()
    # experiment_3d()
    # experiment_4()
    # experiment_6()
    # experiment_61()
    # experiment_62()
    # experiment_63()
    # experiment_64()
