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
	//if (hdr->e_ident[EI_CLASS] == ELFCLASS32)
	//	exit_error("Unsupported ELF File Class.\n");
	//if (hdr->e_ident[EI_CLASS] != ELFCLASS64)
	//	exit_error("Unsupported ELF File Class.\n");
	//if (hdr->e_ident[EI_DATA] != ELFDATA2LSB)
	//	exit_error("Unsupported ELF File byte order.\n");
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
	//	printf("%2d: %4d '%s'\n", i, shdr[i].sh_name, sh_strtab_p + shdr[i].sh_name);
		if (ft_memcmp(sh_strtab + shdr[i].sh_name, ".text", 5) == 0)
			return (&shdr[i]);
	}
	return NULL;
}

char	print_type(Elf64_Sym sym, Elf64_Shdr *shdr) {
	char c;

	if (ELF64_ST_BIND(sym.st_info) == STB_GNU_UNIQUE)
		c = 'u';
	else if (ELF64_ST_BIND(sym.st_info) == STB_WEAK)
	{
		c = 'W';
		if (sym.st_shndx == SHN_UNDEF)
			c = 'w';
	}
	else if (ELF64_ST_BIND(sym.st_info) == STB_WEAK && ELF64_ST_TYPE(sym.st_info) == STT_OBJECT)
	{
		c = 'V';
		if (sym.st_shndx == SHN_UNDEF)
			c = 'v';
	}
	else if (sym.st_shndx == SHN_UNDEF)
		c = 'U';
	else if (sym.st_shndx == SHN_ABS)
		c = 'A';
	else if (sym.st_shndx == SHN_COMMON)
		c = 'C';
	else if (shdr[sym.st_shndx].sh_type == SHT_NOBITS
			&& shdr[sym.st_shndx].sh_flags == (SHF_ALLOC | SHF_WRITE))
		c = 'B';
	else if (shdr[sym.st_shndx].sh_type == SHT_PROGBITS
			&& shdr[sym.st_shndx].sh_flags == SHF_ALLOC)
		c = 'R';
	else if (shdr[sym.st_shndx].sh_type == SHT_PROGBITS
			&& shdr[sym.st_shndx].sh_flags == (SHF_ALLOC | SHF_WRITE))
		c = 'D';
	else if (shdr[sym.st_shndx].sh_type == SHT_PROGBITS
			&& shdr[sym.st_shndx].sh_flags == (SHF_ALLOC | SHF_EXECINSTR))
		c = 'T';
	else if (shdr[sym.st_shndx].sh_type == SHT_DYNAMIC)
		c = 'D';
	else
		c = '?';
	if (ELF64_ST_BIND(sym.st_info) == STB_LOCAL && c != '?')
		c += 32;
	return c;
}

void	printSymbols(void *file_map)
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)file_map;
	Elf64_Shdr *shdr = (Elf64_Shdr *)(file_map + ehdr->e_shoff);

	Elf64_Shdr *strtab;
	Elf64_Shdr *shstrtab;
	Elf64_Shdr *symtab;
	char *str = (char *)(file_map + shdr[ehdr->e_shstrndx].sh_offset);

	for (int i = 0; i < ehdr->e_shnum; i++) {
		if (shdr[i].sh_size) {
	//		printf("%s\n", &str[shdr[i].sh_name]);
			if (ft_memcmp(&str[shdr[i].sh_name], ".symtab", 7) == 0)
				symtab = (Elf64_Shdr *) &shdr[i];
			if (ft_memcmp(&str[shdr[i].sh_name], ".shstrtab", 9) == 0)
				shstrtab = (Elf64_Shdr *) &shdr[i];
			if (ft_memcmp(&str[shdr[i].sh_name], ".strtab", 7) == 0)
				strtab = (Elf64_Shdr *) &shdr[i];
		}
	}
	Elf64_Sym *sym = (Elf64_Sym *)(file_map + symtab->sh_offset);
	str = (char *)(file_map + strtab->sh_offset);
	for (size_t i = 0; i < symtab->sh_size / sizeof(Elf64_Sym); i++) {
		printf("000000000000ffff %c %s\n", print_type(sym[i], shdr), str + sym[i].st_name);
	}
}

int main(int ac, char **av)
{
	int		fd;
	void	*file_map;
	struct	stat statbuf;
	t_file	*toForge;

	if (ac != 2)
		exit_error("Need at least one argument.\n");
	fd = open(av[1], O_RDONLY);
	if (fd < 0)
		exit_error("Bad file descriptor.\n");

	toForge = malloc(sizeof(t_file));
	if (!toForge)
		exit_error("Malloc failed.\n");
	if (fstat(fd, &statbuf) < 0)
		exit_error("Bad file.\n");

	file_map = mmap(NULL, statbuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file_map == MAP_FAILED)
		exit_error("Map failed.\n");

	elf_check_supported(file_map);
//	getSegments(file_map);

	//	toForge->ehdr = malloc(sizeof(Elf64_Ehdr));
	//	toForge->shdr = malloc(sizeof(Elf64_Shdr));
	//	toForge->phdr = malloc(sizeof(Elf64_Phdr));
	//
	//	toForge->ehdr = (Elf64_Ehdr *)file_map;
	//	toForge->shdr = (Elf64_Shdr *)(file_map + toForge->ehdr->e_shoff);
	//	toForge->phdr = (Elf64_Phdr *)(file_map + toForge->ehdr->e_phoff);

	printSymbols(file_map);

	//Elf64_Shdr *section_text = getSectionText(toForge, file_map);

	//write(fd, file_map, toForge->size);
	close(fd);
	munmap(file_map, statbuf.st_size);
	free(toForge);

	return 0;
}

