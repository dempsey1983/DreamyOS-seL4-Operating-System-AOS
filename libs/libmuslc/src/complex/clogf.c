/* @LICENSE(MUSLC_MIT) */

#include "libm.h"

// FIXME

float complex clogf(float complex z)
{
	float r, phi;

	r = cabsf(z);
	phi = cargf(z);
	return cpackf(logf(r), phi);
}
