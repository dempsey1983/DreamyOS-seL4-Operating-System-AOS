/* @LICENSE(MUSLC_MIT) */

#include <fcntl.h>
#include "internal/syscall.h"

int posix_fallocate(int fd, off_t base, off_t len)
{
	return -__syscall(SYS_fallocate, fd, __SYSCALL_LL_O(base),
		__SYSCALL_LL_E(len));
}
