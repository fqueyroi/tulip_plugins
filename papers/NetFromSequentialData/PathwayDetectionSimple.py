from collections import Counter
import random

##############################################################
def get_most_commons(path): 
    c = Counter(path)
    max_count = c.most_common(1)[0][1]
    res = [x for x, c in c.most_common() if c == max_count]
    return res

##############################################################
def is_valid_subpath(path): 
    c = Counter(path)
    return c.most_common(1)[0][1] <= 2

##############################################################
def get_cycles_pivot(path, pivot): 
    res = []    
    ind = [i for i, x in enumerate(path) if x == pivot]
    if ind[0] > 0: 
        res.append(path[:ind[0]])
    if len(ind) >= 2: 
        for i in range(len(ind)-1): 
            res.append(path[ind[i]:ind[i+1]])
    if ind[-1] < len(path)-1: 
        res.append(path[ind[-1]:])
    else: 
        res.append([pivot])
    return res

##############################################################
def find_valid_subpath_decomp(path): 
    if is_valid_subpath(path): 
        return [path]
    else: 
        max_visited = get_most_commons(path)
        pivot = random.sample(max_visited, 1)[0]
        subpaths = get_cycles_pivot(path, pivot)
        res = []
        for sp in subpaths: 
            r_sp = find_valid_subpath_decomp(sp)
            if isinstance(r_sp[0], list): 
                res.extend(r_sp)
            else: 
                res.append(r_sp)
        return res

##############################################################
def find_pathways(path):
    subpaths = find_valid_subpath_decomp(path)
    res = []
    for i in range(len(subpaths)):
        res.append(list(subpaths[i]))
        if i < len(subpaths) - 1:
          res[i].append(subpaths[i+1][0])
    return res
