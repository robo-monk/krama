def fac(a):
    if a == 0: return 0;
    if a == 1: return 1;
    return fac(a-1)*fac(a-2)

print(fac(33))
