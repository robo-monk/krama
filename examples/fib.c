

int a = 5;

unsigned long factorial(unsigned long n)
{
  if (n < 1) {
  return 1;
};
  return factorial(n - 1) * n;
};

void main(long ab)
{
  factorial(5);
};
