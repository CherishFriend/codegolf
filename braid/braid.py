from sys import stdin as s;n,m=map(int,s.readline().split(' '));w=2*n
b=reduce(lambda x,y:x+y,[[q,q] for q in range(n/2+1)+range(n-1,n/2,-1)])
print ' '.join([str(b[((b.index(k)+(k<n/2))%w+m*(1 if k!=n/2 else n+1))%w]+1) for k in range(n)])