0x1234
013
1234

/*aa
a*/  /*

*/
// */ // comment, not syntax error
f = g/**//h; // equivalent to f = g / h;
//\
i(); // part of a two-line comment
/\
/ j(); // part of a two-line comment

/*//*/ l(); // equivalent to l();
m = n//**/o
+ p; // equivalent to m = n + p;