

unsigned int factorial(unsigned int number)
{
  if (number < 1) {
  return 1;
};
  return number * factorial(number - 1);
};

int main()
{
  unsigned int result = factorial(12);
};
