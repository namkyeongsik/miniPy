x = 2
y = 3
px = &x
py = &y
tmp = *px + *py
*py = tmp
*px = *py + 1
print(x)
print(y)
print(*px)
print(*py)
