import pycr
# import numpy
import time
import dis
import json
print("Module file:", getattr(pycr.pycrlib, "__file__", "n/a"))

'''
This file contains a sample program, which computes values of a given closed
form expression over an n-dimensional tensor, using MCR technique, and shows the time 
of the computation performed. Parallelization is not fully implemented

User has to provide a function string and a matrix of parameters.

The pycr.py file contains all of the necessary parsing and construction
of CR's

There are two important functions: evalcr and crgen that evaluate a function
or construct minimal python code 

'''

e = "x"
params = ["x,1,1,1000000"]

print(10000000.0**2+2*10000000+4)
code,t = pycr.evalcr(e,params)

# print(code)
print(f"{t} ms for code evaluation")

results = [0] * 10000
ncode = pycr.naiveinit(e,params)
c_ncode = compile(ncode, "<generated>", "exec")
start = time.perf_counter()
exec(c_ncode)

print(f"{(time.perf_counter() - start)*1000} ms for code evaluation (standard)")
print(results[-1])

# CODE PRINTING
90, 1099, 10587
41, 252, 2821




# NAIVE CODE EXECUTION

# # start = time.perf_counter()
# # for i in range(100000):
# #     b = float(i)
# #     b**5+b**3+b**2
# # print((time.perf_counter() - start)*1000)

# res,t = pycr.evalcr(e,params)
# print(t)



