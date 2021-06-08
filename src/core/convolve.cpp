/*
 * convolve.c: Implements Kyrre Strom's algorithm for convolutions of
 *   B-spline defined functions with other B-spline defined functions.
 *   The algorithm can be found in "On convolutions of B-splines", Journal
 *   of Computational and Applied Mathematics, 55(1):1-29, 1994.
 */

#include <stdlib.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include <vector>

#include "photospline/bspline.h"

namespace photospline{

double
divdiff(const double* x, const double* y, size_t n)
{
	if (n == 1)
		return y[0];
	
	return ((divdiff(&x[1], &y[1], n-1) - divdiff(x, y, n-1))
	    / (x[n-1] - x[0]));
}

unsigned int
factorial(unsigned int n)
{
	int acc = n;
	
	for (unsigned int i = n-1 ; i > 1; i--)
		acc *= i;
	
	return (acc);
}
	
/* 
 * The local blossom of the convolution of the splines defined on knot
 * vectors x and y can be evaluated at point z via iterated divided
 * differences.
 * 
 * This is analogous to Stroem Equation 13 and Lemma 9, but with the prefactor
 * adapted to account for the fact that one of the splines is de-Boor 
 * normalized and the other unit normalized.
 *
 * There exists a recurrence relation for the convoluted blossom (see Stroem 
 * Theorem 12 and Corollary 13) that could speed up this calculation 
 * significantly by never calculating the blossom for argument bags known to
 * return 0. Since we only do this once (and even then, only along one axis),
 * it's simply not worth the headache.
 */ 
double
convoluted_blossom(const double* x, size_t nx, const double* y, size_t ny, double z,
    const double* bags, size_t nbags)
{
	double scale;
	std::vector<double> fun_x(nx), fun_y(ny);
	unsigned i, j, k;
	
	/*
	 * If the supports of the source and target spline do not overlap, the
	 * coeffienct is zero. Short-cut here to avoid round-off errors that can 	
	 * lead to negative entries in the transfer matrix. 
	 */
	if ((x[0] + y[0] > z) || (x[nx-1] + y[ny-1] < bags[nbags-1]))
		return 0.;
	
	scale = x[nx-1] - x[0];
	
	for (i = 0; i < nx; i++) {
		for (j = 0; j < ny; j++) {
			if (x[i] + y[j] - z > 0.0) {
				double det = 1.0;
				for (k = 0; k < nbags; k++)
					det *= (x[i] + y[j] - bags[k]);
				fun_y[j] = det;
			} else {
				fun_y[j] = 0.0;
			}
		}
		fun_x[i] = divdiff(y, fun_y.data(), ny);
	}
	return (scale*divdiff(x, fun_x.data(), nx));
}

} //namespace photospline
