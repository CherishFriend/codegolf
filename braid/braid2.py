from sys import stdin;n,m=map(int,stdin.readline().split(' '));b=range(1,n+1)
for k in range(m):v=b.pop(n-1 if k%2 else 0);b.insert(n/2,v)
print ' '.join(map(str,b))