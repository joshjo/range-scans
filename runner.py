import os


queries = [
    10,
    25,
    50,
    100,
    250,
    500,
    1000,
    2500,
    5000,
    10000,
    25000,
    50000,
    100000,
    # 250000,
    # 500000,
    # 1000000,
]

range_sizes = [
    # 10,
    # 100,
    # 1000,
    # 10000,
    100000,
]


iters = 5


if __name__ == '__main__':
    for query in queries:
        for range_size in range_sizes:
            for i in range(iters):
                command = "./reads_lazy.out --query_number=%d --range_size=%d" % (query, range_size)
                print(command)
                os.system(command)
    print("\n")
