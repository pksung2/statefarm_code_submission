import sys
from sys import stdin
import pandas as pd
import numpy as np
import itertools
        
def solveCells(data, k):
    partition_start = 0
    partition_range = int(len(data[0])/k)
    
    while partition_start != len(data[0]) - partition_range:
        
        # Set the intervals
        try:
            partition_end += partition_range
            partition_start += partition_range
        except:
            partition_end = partition_range
        
        solvePartition([row[partition_start:partition_end] for row in data])
        
        # Separate partitions
        if partition_end < len(data[0]):
            print()
    
def solvePartition(data):    
    
    # Increment from cell size of 1 to the max cell size in this partition
    for cell_size in range(1,len(data[0])+1):

        cells = {}
        combinations = [list(combo) for combo in itertools.combinations( [i for i in range(0,len(data[0]))] , cell_size )]

        # Trying out combinations!
        for combination in combinations:
            
            # Set a temporary dictionary to hold new values
            tempdict = {}
            
            # Going through all the rows..
            for row in data:

                # Making the key for the dictionary
                key = []

                for value in combination:
                    key.append(row[value])
                
                key = ' '.join(key)
                
                # Add key to dictionary if it doesn't exist, otherwise increment
                try:
                    cells[key] += 1
                except:
                    cells[key] = 1
                    tempdict[key] = 1
            
            try:
                keyvals += sorted(tempdict.keys())
                #print(keyvals)
            except:
                keyvals = sorted(tempdict.keys())
                #print(keyvals)
            
        # Print out all of the cells of this size!
        for key in keyvals:
            print("{0} : {1}".format(key,cells[key]))
        
        keyvals = []

            
# Retrieve partition value
k = int(stdin.readline())

# Retrieve the rest of the data
data = []
for line in sys.stdin:
    data.append(line.split())

solveCells(data, k)
            