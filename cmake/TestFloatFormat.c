int main(int argc, char **argv)
{
  int ret = 0;

  double bin1[] = {
    // "*TAGLIB*" encoded as a little-endian floating-point number
    (double)3.9865557444897601e-105, (double)0.0
  };
  float bin2[] = {
    // "*TL*" encoded as a little-endian floating-point number
    (float)1.81480400e-013, (float)0.0
  };
  ret += ((int*)bin1)[argc];
  ret += ((int*)bin2)[argc];

  return ret;
}
