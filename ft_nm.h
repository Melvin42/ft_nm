#define FT_NM_H
#ifndef ft_NM_H

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/mman.h>

void	ft_bzero(void *s, size_t n);
void	*ft_memcpy(void *dest, const void *src, size_t n);
int		ft_memcmp(const void *s1, const void *s2, size_t n);

typedef struct s_file {
	Elf64_Ehdr	*ehdr;
	Elf64_Shdr	*shdr;
	Elf64_Phdr	*phdr;
	off_t		size;
} t_file;

#endif

