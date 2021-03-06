
/*
 * -- SuperLU routine (version 2.0) --
 * Univ. of California Berkeley, Xerox Palo Alto Research Center,
 * and Lawrence Berkeley National Lab.
 * November 15, 1997
 *
 */
/*
 * This file defines common arithmetic operations for complex type.
 */
#include <math.h>
#include <stdio.h>
#include "scomplex.h"


/* Approximates the abs */
/* Returns abs(z.r) + abs(z.i) */
double c_abs1(complex *z)
{
    float real = z->r;
    float imag = z->i;
  
    if (real < 0) real = -real;
    if (imag < 0) imag = -imag;

    return (real + imag);
}

