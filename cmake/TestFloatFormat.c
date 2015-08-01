int main(int argc, char **argv)
{
  int ret = 0;

  volatile double bin1[] = {
    // "*TAGLIB*" encoded as a little-endian floating-point number
    (double)3.9865557444897601e-105, (double)0.0
  };
  volatile float bin2[] = {
    // "*TL*" encoded as a little-endian floating-point number
    (float)1.81480400e-013, (float)0.0
  };
  ret += (int)(bin1[1]+bin2[1]);
  return ret;
}
