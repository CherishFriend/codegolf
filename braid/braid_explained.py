from sys import *
N,i=map(int,stdin.readline().split())
h,t=N/2,3*N
f=lambda p:(p>N)*(t/2-(p&-2))+p/2+1
for s in xrange(N):print f((2*s+1+(s>h)*(t-4*s-2)+i*(N+1-N*(s!=h)))%(2*N)),