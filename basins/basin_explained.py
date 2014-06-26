from math import floor
with open('a') as f:
	l = f.read()
	terrain = map(int,l.split()) # read in all the numbers into an array (treating the 2D array as flattened 1D)
	n = terrain.pop(0) # pop the first value: the size of the input
	valid_indices = range(n*n) # 0..(n*n)-1 are the valid indices of this grid
	water=[1]*(n*n) # start with 1 unit of water at each grid space. it will trickle down and sum in the basins.
	updates=1 # keep track of whether each iteration included an update

	# helper functions
	def dist(i,j):
		# returns the manhattan (L1) distance between two indices
		row_dist = abs(floor(j/n) - floor(i/n))
		col_dist = abs(j % n - i % n)
		return row_dist + col_dist

	def neighbors(j):
		# returns j plus up to 4 valid neighbor indices
		possible = [j,j-1,j+1,j-n,j+n]
		# validity criteria: neighbor must be in valid_indices, and it must be one space away from j
		return [x for x in possible if dist(x,j)<=1 and x in valid_indices]

	def down(j):
		# returns j iff j is a sink, otherwise the minimum neighbor of j
		# (works by constructing tuples of (value, index) which are min'd
		# by their value, then the [1] at the end returns its index)
		return min((terrain[i],i) for i in neighbors(j))[1]

	while updates!=0: # break when there are no further updates
		updates=0 # reset the update count for this iteration
		for j in valid_indices: # for each grid space, shift its water 
			d =down(j)
			if j!=d: # only do flow if j is not a sink
				updates += water[j] # count update (water[j] is zero for all non-sinks when the sinks are full!)
				water[d] += water[j] # move all of j's water into the next lowest spot
				water[j] = 0 # indicate that all water has flown out of j
	# at this point, `water` is zeros everywhere but the sinks.
	# the sinks have a value equal to the size of their watershed.
	# so, sorting `water` and printing nonzero answers gives us the result we want!
	water = sorted(water)[::-1] # [::-1] reverses the array (high to low)
	nonzero_water = [w for w in water if w] # 0 evaulates to false.
	print " ".join([str(w) for w in nonzero_water]) # format as a space-separated list