import os
import subprocess
import sys
from datetime import datetime


universal_domain = 10**9
universal_iters = 5

EXEC = 'cooplanning'
EXEC_HEADER = 'csvheader'


def runner(
    databases,
    distributions,
    domain,
    strategies,
    queries,
    leaf_sizes,
    range_sizes,
    iters,
    percentage_point_queries,
    random_range_size='false',
    min_range_size=0,
    max_range_size=0,
    bool_args=[]
):
    extraflags = ' '.join(bool_args)

    os.system(f'./{EXEC_HEADER}.out {extraflags} --percentage_point_queries={1 if len(percentage_point_queries) else 0}')

    for database in databases:
        for percentage in percentage_point_queries:
            for distribution in distributions:
                for strategy in strategies:
                    for query_size in queries:
                        for leaf_size in leaf_sizes:
                            for range_size in range_sizes:
                                for i in range(iters):
                                    command = f"./{EXEC}.out --database={database} --distribution={distribution} --iter={i} --strategy={strategy} --queries={query_size} --domain={domain} --leaf_size={leaf_size} --range_size={range_size} --random_range_size={random_range_size} --min_range_size={min_range_size} --max_range_size={max_range_size} --percentage_point_queries={percentage} {extraflags}"
                                    os.system(command)


def avg_case():
    databases = ['any']
    distributions = ['normal']
    domain = 100000000
    strategies = ['lazy']
    # queries = [10, 100, 1000, 10000, 100000, 300000]
    queries = [i for i in range(10, 300001, 100)]
    leaf_sizes = [100]
    range_sizes = [100]
    iters = 2
    bool_args = ['--exec_database=false']
    percentage_point_queries = [0]

    runner(
        databases,
        distributions,
        domain,
        strategies,
        queries,
        leaf_sizes,
        range_sizes,
        iters,
        percentage_point_queries,
        bool_args=bool_args,
    )


def experiment_1():
    databases = ['rocksdb', 'duckdb']
    distributions = ['normal']
    domain = 1000000
    strategies = ['lazy']
    queries = [10, 100, 1000, 10000, 100000, 1000000]
    # queries = [10000]
    leaf_sizes = [100000]
    range_sizes = [100000]
    iters = universal_iters
    percentage_point_queries = [0]
    bool_args=['--exec_postfiltering=false']

    runner(
        databases,
        distributions,
        domain,
        strategies,
        queries,
        leaf_sizes,
        range_sizes,
        iters,
        percentage_point_queries,
        bool_args=bool_args,
    )


def experiment_6():
    iters = universal_iters

    strategies = [
        'lazy',
    ]
    queries = [
        100000,
    ]
    leaf_sizes = [
        "max_range",
    ]
    range_sizes = [
        1000,
        10000,
        100000,
        1000000,
        10000000,
    ]
    bool_args = ['--exec_database=false']
    distributions = ['normal']
    domain = universal_domain
    percentage_point_queries = [50, 80, 90, 99]
    databases = ['any']
    # percentage_point_queries = [50]

    runner(
        databases,
        distributions,
        domain,
        strategies,
        queries,
        leaf_sizes,
        range_sizes,
        iters,
        percentage_point_queries,
        bool_args=bool_args,
    )

if __name__ == '__main__':
    # avg_case()
    experiment_1()
    # experiment_6()
