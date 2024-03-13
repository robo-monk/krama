def fac(a):
    if a < 2: return a;
    return fac(a-1) + fac(a-2)

print(fac(35))
