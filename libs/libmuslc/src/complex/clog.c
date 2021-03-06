/* @LICENSE(MUSLC_MIT) */

#include "libm.h"

// FIXME

/* log(z) = log(|z|) + i arg(z) */

double complex clog(double complex z)
{
	double r, phi;

	r = cabs(z);
	phi = carg(z);
	return cpack(log(r), phi);
}
