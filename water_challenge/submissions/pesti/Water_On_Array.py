# -*- coding: utf-8 -*-
"""
Made by Pesti

Q: What do cats and programmers have in common?
A: When either one is unusually happy and excited, an appropriate question would be, “did you find a bug?”
"""


def Calc(a):
    p1=0
    p2=a.size-1
    w=0
    while(p1!=p2):
        a=a-1
        while((a[p1]<0)and(p1!=p2)):
            p1=p1+1
        if(p1!=p2):
            while(a[p2]<0):
                p2=p2-1
        if(p1!=p2):
            w=w+np.count_nonzero(a[p1:p2+1]<0)
    return w

if __name__ == '__main__':
    import numpy as np
    import time,sys
    #a=np.genfromtxt(sys.argv[1],delimiter=",")
    with open(sys.argv[1]) as file:
        a = np.array([int(i) for i in file.read().split(',')])
    start = time.clock()
    result = Calc(a)
    end = time.clock()
    time = end - start
    print(result)
    print("{0:.4f}".format(time * 1000))
