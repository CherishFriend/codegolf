n=5
m=10

b=range(1,n+1)
print ' '.join(map(str,b))
for x in range(m):
	if x%2==0: # take left
		v=b.pop(0)
	else: #take right
		v=b.pop()
	b.insert(n/2,v)
	print ' '.join(map(str,b))