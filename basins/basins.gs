' '% # split stdin on whitespace
{~}%  # convert to integers (map eval to array of strings)
(\:t\:n.*:q; # pop front, square it -> q is n*n size of input
#[1]q* # initialize array of waters
4:j; # index j
[j-1 j+1 j-n j+n].
`