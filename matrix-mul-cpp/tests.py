import numpy as np
import re
import sys
import random
import subprocess

'''
    tests the cpp implementation of the matrix multiplier against numpy
    using randomly generated inputs
'''

def formatMatrix(mat):
    pieces = []
    for row in mat:
        row = re.sub(r'[^0-9\s]', '', str(row))
        row = re.sub(r'\s+', ' ', str(row))
        pieces.append(row.strip() + " ")
    return '\n'.join(pieces) + "\n"

def makeMatrix(r, c, f):
    mat = np.matrix(np.random.randint(1,50,(r, c)))
    f.write(f'{mat.shape[0]} {mat.shape[1]}\n')
    f.write(formatMatrix(mat))
    f.write('\n')
    return mat

if __name__ == '__main__':
    
    for i in range(20):
        mn, mx = 5, 20
        a, b = random.randint(mn, mx), random.randint(mn, mx)
        comm = random.randint(mn, mx)

        with open('input.txt', 'w') as f:
            A = makeMatrix(a, comm, f)
            B = makeMatrix(comm, b, f)

        p = subprocess.run(f'./matrixmul.out input.txt', shell=True, stdout=subprocess.PIPE)
        
        if p.stdout.decode() == formatMatrix(A * B):
            print("Pass!")
        else:
            print("Fail!")
