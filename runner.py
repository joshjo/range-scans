import os
from datetime import datetime


iters = 5

strategies = [
    'lazy',
    'eager',
    'extra',
    # 'original',
]


if __name__ == '__main__':
    print("Experiment 1")
    queries = [
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
    ]


    for strategy in strategies:
        for query_size in queries:
            for i in range(iters):
                print(datetime.now())
                command = "./main.out --strategy=%s --queries=%d --key_domain_size=1000000 --leaf_size=100000 --range_size=100000" % (strategy, query_size)
                print(command)
                os.system(command)

    # print("Experiment 2")
    # queries = [
    #     10000,
    #     100000,
    # ]
    # rqs = [
    #     10,
    #     100,
    #     1000,
    #     10000,
    #     100000,
    # ]
    # for strategy in strategies:
    #     for query_size in queries:
    #         for range_size in rqs:
    #             for i in range(5):
    #                 command = "./main.out --query_number=%d --key_domain_size=1000000 --leaf_size=100000 --range_size=%d" % (query_size, range_size)
    #                 print(command)
    #                 os.system(command)
