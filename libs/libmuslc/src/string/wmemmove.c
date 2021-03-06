/* @LICENSE(MUSLC_MIT) */

#include <string.h>
#include <wchar.h>

wchar_t *wmemmove(wchar_t *d, const wchar_t *s, size_t n)
{
	if ((size_t)(d-s) < n) {
		while (n--) d[n] = s[n];
		return d;
	}
	return wmemcpy(d, s, n);
}
