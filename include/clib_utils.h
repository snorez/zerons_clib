#ifndef UTILS_H_NOWJRQGI
#define UTILS_H_NOWJRQGI

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/user.h>
#include <linux/limits.h>

#define clib__round_mask(x, y)	((__typeof__(x))((y)-1))
#define clib_round_up(x, y)	((((x)-1) | clib__round_mask(x, y)) + 1)
#define clib_round_down(x, y)	((x) & ~clib__round_mask(x, y))

extern uint32_t min_32(uint32_t a, uint32_t b);
extern uint64_t min_64(uint64_t a, uint64_t b);
extern uint32_t max_32(uint32_t a, uint32_t b);
extern uint64_t max_64(uint64_t a, uint64_t b);

extern void *malloc_s(size_t size);
extern void free_s(void **addr);
extern int hex2int(char *hex);

static inline int get_online_cpus(void)
{
	return sysconf(_SC_NPROCESSORS_ONLN);
}

static inline char *get_arg(char *argv[], char *target)
{
	int i = 1;
	while (argv[i]) {
		if (strcmp(argv[i], target))
			i++;
		else
			return argv[i+1];
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: UTILS_H_NOWJRQGI */