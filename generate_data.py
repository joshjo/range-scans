from random import randint, random


def n(number):
    return len(str(number))


def randomizer(max_random=10000000, range_size=100000, queries=100):
    max_random = max_random - range_size - 1

    for i in range(queries):
        d = randint(1, max_random)
        print('%s %s' % (d, int(d + range_size)))


if __name__ == '__main__':
    randomizer(max_random=1000000, range_size=100000, queries=100000)
