import os
import filecmp


output_folder = 'output/'

main_folder = output_folder + 'original/'

strategies = ['lazy', 'eager', 'extra']

results = []


for r, d, f in os.walk(main_folder):
    for strategy in strategies:
        strategy_diffs = []
        for query_file in f:
            diff = filecmp.cmp(
                main_folder + query_file,
                '%s%s/%s' % (output_folder, strategy, query_file),
            )
            strategy_diffs.append(diff)
        if all(strategy_diffs):
            print("✓ %s is correct :)" % strategy)
        else:
            print("[Error] %s is presenting diff results", strategy)
