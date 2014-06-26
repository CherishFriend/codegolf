from math import floor as f
with open('a') as m:
	l=m.read();t=map(int,l.split());n=t.pop(0);q=n*n;r,b,u=range(q),[1]*q,1
	while u!=0:
		u=0
		for j in r:
			d=min((t[i],i) for i in [x for x in [j,j-1,j+1,j-n,j+n] if abs(f(j/n)-f(x/n))+abs(j%n-x%n)<=1 and x in r])[1]
			if j!=d:
				u+=b[j];b[d]+=b[j];b[j]=0
	print " ".join([str(x) for x in sorted(b)[::-1] if x])