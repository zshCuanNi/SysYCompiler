
;; Function factor (factor, funcdef_no=0, decl_uid=2248, cgraph_uid=0, symbol_order=0)

factor (int n)
{
  int D.2259;

  if (n <= 1) goto <D.2257>; else goto <D.2258>;
  <D.2257>:
  D.2259 = 1;
  goto <D.2260>;
  <D.2258>:
  _1 = n + -1;
  _2 = factor (_1);
  D.2259 = n * _2;
  goto <D.2260>;
  <D.2260>:
  return D.2259;
}



;; Function main (main, funcdef_no=1, decl_uid=2250, cgraph_uid=1, symbol_order=1)

main ()
{
  int i;
  int D.2266;

  i = 0;
  goto <D.2253>;
  <D.2255>:
  i = i + 1;
  if (i == 3) goto <D.2261>; else goto <D.2263>;
  <D.2263>:
  if (i == 5) goto <D.2261>; else goto <D.2262>;
  <D.2261>:
  // predicted unlikely by continue predictor.
  goto <D.2253>;
  <D.2262>:
  if (i == 8) goto <D.2264>; else goto <D.2265>;
  <D.2264>:
  goto <D.2254>;
  <D.2265>:
  _1 = factor (i);
  printf ("%d! = %d", i, _1);
  <D.2253>:
  if (i <= 9) goto <D.2255>; else goto <D.2254>;
  <D.2254>:
  D.2266 = 0;
  goto <D.2267>;
  D.2266 = 0;
  goto <D.2267>;
  <D.2267>:
  return D.2266;
}

