import os
import subprocess
import pandas as pd
from datetime import datetime


iters = 5

strategies = [
    'lazy',
    'eager',
    'extra',
    # 'original',
]


if __name__ == '__main__':
    # print("Experiment 1")
    # queries = [
    #     10,
    #     100,
    #     1000,
    #     10000,
    #     100000,
    #     1000000,
    # ]


    # for strategy in strategies:
    #     for query_size in queries:
    #         for i in range(iters):
    #             print(datetime.now())
    #             command = "./main.out --strategy=%s --queries=%d --key_domain_size=1000000 --leaf_size=100000 --range_size=100000" % (strategy, query_size)
    #             print(command)
    #             os.system(command)

    print("Experiment 2")
    queries = [
        1000000,
        # 10000000,
    ]
    rqs = [
        # 10,
        # 100,
        # 1000,
        # 10000,
        100000,
        # 200000,
        # 400000,
        # 800000,
    ]
    for query_size in queries:
        print('Queries', query_size)
        for range_size in rqs:
            print('RQS', range_size)
            for strategy in strategies:
                results = []
                for i in range(5):
                    command = "./main.out --strategy=%s --queries=%d --key_domain_size=1000000 --leaf_size=200000 --range_size=%d" % (strategy, query_size, range_size)
                    output = [float(i) for i in subprocess.check_output(command, shell=True).decode('ascii').split(',')]
                    results.append(output)
                df = pd.DataFrame(results[2:])
                mean = [str(i) for i in list(df.mean())]
                mean.insert(0, strategy)
                print(','.join([str(i) for i in mean]))
