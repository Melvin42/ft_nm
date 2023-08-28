#include "./ft_nm.h"

void exit_error(char *error)
{
	dprintf(2, "%s", error);
	exit(EXIT_FAILURE);
}

bool elf_check_file(Elf64_Ehdr *hdr)
{
	if (!hdr)
		exit_error("Invalid ELF File.\n");
	if (ft_memcmp((char *)hdr->e_ident, ELFMAG, SELFMAG))
		exit_error("ELF Header Magic incorrect.\n");
	return true;
}

bool elf_check_supported(Elf64_Ehdr *hdr)
{
	elf_check_file(hdr);
	if (hdr->e_ident[EI_CLASS] == ELFCLASS32)
		exit_error("Unsupported ELF File Class.\n");
	if (hdr->e_ident[EI_CLASS] != ELFCLASS64)
		exit_error("Unsupported ELF File Class.\n");
	if (hdr->e_ident[EI_DATA] != ELFDATA2LSB)
		exit_error("Unsupported ELF File byte order.\n");
	//if (hdr->e_type != ET_REL && hdr->e_type != ET_EXEC)
	//	exit_error("Unsupported ELF File type.\n");
	return true;
}

bool getSegments(void *file_map)
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)file_map;
	Elf64_Phdr *phdr = (Elf64_Phdr *)(file_map + ehdr->e_phoff);
	char *segments = file_map + phdr->p_offset;
	int phnum = ehdr->e_phnum;
	printf("%d\n", phnum);
	for (int i = 0; i < phnum; i++) {
		if (phdr[i].p_type == PT_LOAD)
			printf("%ld\n", phdr[i].p_filesz);
		//printf("%d\n", phdr[i].p_memsz);
	}

}

Elf64_Shdr *getSectionText(t_file *toForge, void *file_map)
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)file_map;
	Elf64_Shdr *shdr = (Elf64_Shdr *)(file_map + ehdr->e_shoff);
	int shnum = ehdr->e_shnum;
	Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
	const char *const sh_strtab_p = file_map + sh_strtab->sh_offset;
	for (int i = 0; i < shnum; ++i) {
		printf("%2d: %4d '%s'\n", i, shdr[i].sh_name, sh_strtab_p + shdr[i].sh_name);
		if (ft_memcmp(sh_strtab + shdr[i].sh_name, ".text", 5) == 0)
			return (&shdr[i]);
	}
	return NULL;
}

int main(int ac, char **av)
{
	int		fd;
	void	*file_map;
	t_file	*toForge;

	if (ac != 2)
		exit_error("Need at least one argument.\n");
	fd = open(av[1], O_RDONLY);
	if (fd < 0)
		exit_error("Bad file descriptor.\n");

	toForge = malloc(sizeof(t_file));
	if (!toForge)
		exit_error("Malloc failed.\n");

	toForge->size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);

	printf("%ld\n", sizeof(t_file));
	file_map = mmap(NULL, toForge->size, PROT_READ, MAP_PRIVATE, fd, 0);

	elf_check_supported(file_map);
	getSegments(file_map);

	toForge->ehdr = malloc(sizeof(Elf64_Ehdr));
	toForge->shdr = malloc(sizeof(Elf64_Shdr));
	toForge->phdr = malloc(sizeof(Elf64_Phdr));

	toForge->ehdr = (Elf64_Ehdr *)file_map;
	toForge->shdr = (Elf64_Shdr *)(file_map + toForge->ehdr->e_shoff);
	toForge->phdr = (Elf64_Phdr *)(file_map + toForge->ehdr->e_phoff);

	Elf64_Shdr *section_text = getSectionText(toForge, file_map);

	fd = open("./woody", O_CREAT | O_TRUNC | O_WRONLY, 0744);
	if (fd < 0)
		exit_error("Can't create file.\n");

	write(fd, file_map, toForge->size);
	close(fd);
	munmap(file_map, toForge->size);
	free(toForge);

	return 0;
}

