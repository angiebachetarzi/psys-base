#include "display.h"
// Generation du test.c parsé :
// cpp -traditional-cpp -C -P -ftabstop=8 -DTELECOM_TST -DCONS_READ_CHAR -DWITH_MSG test.c test.i
// sed -i '/./,/^$/!d' test.i
// cat test.h test.i

//XXX Assurer que l'oubli d'une option fait planter la compilation
//XXX Verifier l'absence de caracteres non ASCII

/*******************************************************************************
 * Gestion de liste d'arguments de taille variable (printf)
 ******************************************************************************/
typedef void *__gnuc_va_list;
typedef __gnuc_va_list va_list;
#define va_arg(AP, TYPE)                                                \
 (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),     \
  *((TYPE *) (void *) ((char *) (AP) - __va_rounded_size (TYPE))))
#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
#define va_start(AP, LASTARG)                                           \
 (AP = ((__gnuc_va_list) __builtin_next_arg (LASTARG)))
#define va_end(AP)      ((void)0)

/*******************************************************************************
 * Printf macros
 ******************************************************************************/
#define PRINTF_LEFT_JUSTIFY 1
#define PRINTF_SHOW_SIGN 2
#define PRINTF_SPACE_PLUS 4
#define PRINTF_ALTERNATE 8
#define PRINTF_PAD0 16
#define PRINTF_CAPITAL_X 32

#define PRINTF_BUF_LEN 512

/*******************************************************************************
 * Assert : check a condition or fail
 ******************************************************************************/
#define __STRING(x) #x

#define assert(cond) \
((void)((cond) ? 0 : assert_failed(__STRING(cond), __FILE__, __LINE__)))

#define DUMMY_VAL 78

#define TSC_SHIFT 8

#define FREQ_PREC 50

#define NBSEMS 10000

#define TRUE 1
#define FALSE 0

#define NR_PHILO 5
/* Copyright (C) 1991-2014 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */

/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */

/* wchar_t uses ISO/IEC 10646 (2nd ed., published 2011-03-15) /
   Unicode 6.0.  */

/* We do not support C11 <threads.h>.  */

/*
 * Tests du projet systeme
 *
 * Ce fichier contient du code qui ne depend que des appels systemes
 * ci-dessous. Il n'inclut aucun fichier ".h".
 *
 * Il est possible de placer ce fichier dans le repertoire user pour faire
 * tourner les tests au niveau utilisateur ou, si l'implantation du mode
 * utilisateur ne fonctionne pas, dans le repertoire kernel pour faire
 * tourner les tests au niveau superviseur.
 * Les tests sont separes en 20 fonctions qui testent differentes parties du
 * projet.
 * Aucune modification ne doit etre apportee a ce fichier pour la soutenance.
 *
 * Il existe deux manieres d'appeler les tests :
 *
 * int test_proc(void *arg) :
 * a demarrer comme un processus avec une priorite de 128. Ce processus
 * attend que l'utilisateur saisisse des numeros de tests a executer.
 *
 * int test_run(int n) :
 * a appeler dans un processus de priorite 128 avec en parametre un numero de
 * test a executer.
 *
 * La fonction test_run() vous permet d'appeler facilement un test mais en
 * soutenance il est preferable d'executer test_proc().
 */

// Prototype des appels systeme de la spec
int chprio(int pid, int newprio);
void cons_write(const char *str, unsigned long size);
// int cons_read(void);
void cons_echo(int on);
void exit(int retval);
int getpid(void);
int getprio(int pid);
int kill(int pid);
// int pcount(int fid, int *count);
// int pcreate(int count);
// int pdelete(int fid);
// int preceive(int fid,int *message);
// int preset(int fid);
// int psend(int fid, int message);
void clock_settings(unsigned long *quartz, unsigned long *ticks);
unsigned long current_clock(void);
void wait_clock(unsigned long wakeup);
int start(int (*ptfunc)(void *), unsigned long ssize, int prio, const char *name, void *arg);
int waitpid(int pid, int *retval);

/*
 * Pour la soutenance, devrait afficher la liste des processus actifs, des
 * files de messages utilisees et toute autre info utile sur le noyau.
 */
// void sys_info(void);

/* static int
strcmp(const char *str1, const char *str2)
{
	while (*str1 == *str2) {
		if (*str1 == 0)
			return 0;
		str1++;
		str2++;
	}
	return *str1 - *str2;
} */

static unsigned long
strlen(const char *s)
{
	unsigned long l = 0;
	while (*s++) l++;
	return l;
}

void cons_echo(int on) {
	switch (on) {
		case 1:
			console_putbytes("1", 1);
			break;
		case 0:
			console_putbytes("0", 0);
			break;
	}
}

void cons_write(const char *str, unsigned long size) {
	console_putbytes(str, size);
}

static void
cons_puts(const char *s)
{
	cons_write(s, strlen(s));
}

/*******************************************************************************
static int _printf( char *s, unsigned long n, const char *format, va_list ap )
    La grosse fonction du module. Une version interne du printf, qui imprime
soit sur la sortie standard, soit dans un buffer.
Si s == 0 : on imprime sur la sortie standard.
Si s != 0 : on imprime dans la chaine s sur une taille maximale n.
*******************************************************************************/
struct printf_st {
	/* Flags obtenus a l'interpretation de la chaine de format. */
	int flags;
	int width;
	int precision;
	char modifier;
	int count;

	/* Ce buffer permet de contenir le texte correspondant a l'affichage
	   d'un nombre. */
	char buffer_nombre[20];

	/* Buffer pour l'ecriture a la console. */
	char buffer_ecr[PRINTF_BUF_LEN];
	int pos_ecr;

	char *str;
	unsigned long strl;
};

static void
print(struct printf_st *pf, char c)
{
	while (1) {
		if (pf->str == 0) {
			/* Cas de l'ecriture sur un fichier. */
			if (c == 0)
				return;
		
			if (pf->pos_ecr < PRINTF_BUF_LEN - 1) {
				pf->count++;
				pf->buffer_ecr[pf->pos_ecr++] = c;
			} else {
				pf->buffer_ecr[PRINTF_BUF_LEN - 1] = 0;
				cons_puts(pf->buffer_ecr);
				pf->buffer_ecr[0] = c;
				pf->pos_ecr = 1;
			}
		} else {
			/* Cas de l'ecriture dans un buffer. */
			if ((c != 0) && (pf->strl != 0)) {
				pf->count++;
				*pf->str++ = c;
				pf->strl--;
			}
		}
		if (c != '\n') return;
		c = '\r';
	}
}

/****************************************************************************
 * Pour afficher les "%s".
 ***************************************************************************/
static void
print_string(struct printf_st *pf, char *s) {
	int size = 0;
	char *ptr = s;

	/* Calcule la taille de la partie de la chaine a afficher. */
	if (pf->precision >= 0)
		while ((size < pf->precision) && (*ptr++ != 0))
			size++;
	else
		/* Pas besoin d'aller trop loin dans le calcul de la taille de la
			partie a afficher. Si la valeur est superieure a width, elle ne nous
			sert a rien, bien qu'on va tout afficher. */
		while ((size < pf->width) && (*ptr++ != 0))
			size++;

	if (!(pf->flags & PRINTF_LEFT_JUSTIFY))
		while (pf->width-- > size)
			print(pf, ' ');
	while ((pf->precision-- != 0) && (*s != 0))
		print(pf, *s++);
	while (pf->width-- > size)
		print(pf, ' ');
}

/*******************************************************************************
 * Pour afficher les "%c".
 ******************************************************************************/
static void
print_char(struct printf_st *pf, char c) {
	if (!(pf->flags & PRINTF_LEFT_JUSTIFY))
		while (pf->width-- > 1)
			print(pf, ' ');
	print(pf, c);
	while (pf->width-- > 1)
		print(pf, ' ');
}

/*******************************************************************************
 * Pour afficher les "%x", "%X".
 ******************************************************************************/
static void
print_hexa(struct printf_st *pf, unsigned long i) {
	int pos = 0;
	int n;

	/* On ne met pas le "0x" si le nombre est nul. */
	if (i == 0)
		pf->flags &= ~PRINTF_ALTERNATE;

	/* La pf->precision par defaut pour un entier est 1. */
	if (pf->precision == -1)
		pf->precision = 1;
	else
		pf->flags &= ~PRINTF_PAD0;

	/* On ecrit l'entier dans le buffer. */
	while (i != 0) {
		n = i % 16;
		i = i / 16;

		/* On calcule le chiffre de poids faible. */
		if (n < 10)
			n += '0';
		else if (pf->flags & PRINTF_CAPITAL_X)
			n += 'A' - 10;
		else
			n += 'a' - 10;

		/* On le met en buffer. */
		pf->buffer_nombre[pos++] = n;
	}

	/* On met a jour la precision avec celle que demande le nombre affiche. */
	pf->precision = (pos > pf->precision) ? pos : pf->precision;

	/* Si on doit remplir avec des 0, on modifie la precision en consequence. */
	if ((!(pf->flags & PRINTF_LEFT_JUSTIFY)) && (pf->flags & PRINTF_PAD0)) {
		n = pf->width;

		if ((pf->flags & PRINTF_SHOW_SIGN) || (pf->flags & PRINTF_SPACE_PLUS))
			n--;
		if (pf->flags & PRINTF_ALTERNATE)
			n -= 2;
		pf->precision = (pf->precision > n) ? pf->precision : n;
		n = pf->width;
	} else {
		n = pf->precision;
		if ((pf->flags & PRINTF_SHOW_SIGN) || (pf->flags & PRINTF_SPACE_PLUS))
			n++;
		if (pf->flags & PRINTF_ALTERNATE)
			n += 2;
	}
	/* Ici n = nombre de caracteres != ' ' affiches. */

	/* Doit-on mettre des espaces de remplissage avant le nombre ? */
	if (!(pf->flags & PRINTF_LEFT_JUSTIFY)) {
		while (pf->width-- > n)
			print(pf, ' ');
	}

	/* On place eventuellement le signe. */
	if (pf->flags & PRINTF_SHOW_SIGN)
		print(pf, '+');
	else if (pf->flags & PRINTF_SPACE_PLUS)
		print(pf, ' ');

	/* On ecrit l'eventuel "0x" ou "0X". */
	if (pf->flags & PRINTF_ALTERNATE) {
		print(pf, '0');
		if (pf->flags & PRINTF_CAPITAL_X)
			print(pf, 'X');
		else
			print(pf, 'x');
	}

	/* On met les eventuels 0 de remplissage. */
	while (pf->precision-- > pos)
		print(pf, '0');

	/* On copie le reste du nombre. */
	while (pos-- != 0)
		print(pf, pf->buffer_nombre[pos]);

	/* On met enfin les eventuels espaces de fin. */
	while (pf->width-- > n)
		print(pf, ' ');
}

/*******************************************************************************
 * Pour afficher les "%d", "%i" et "%u". Le signe doit etre '+' ou '-'.
 ******************************************************************************/
static void
print_dec(struct printf_st *pf, unsigned long i, char sign) {
	int pos = 0;
	int n;

	/* La precision par defaut pour un entier est 1. */
	if (pf->precision == -1)
		pf->precision = 1;
	else
		pf->flags &= ~PRINTF_PAD0;

	/* On determine le signe a afficher. */
	if ((sign == '+') && (!(pf->flags & PRINTF_SHOW_SIGN))) {
		if (pf->flags & PRINTF_SPACE_PLUS)
			sign = ' ';
		else
			sign = 0;
	}

	/* On ecrit l'entier dans le buffer. */
	while (i != 0) {
		/* On le met en buffer. */
		pf->buffer_nombre[pos++] = (i % 10) + '0';
		i = i / 10;
	}

	/* On met a jour la precision avec celle que demande le nombre affiche. */
	pf->precision = (pos > pf->precision) ? pos : pf->precision;

	/* Si on doit remplir avec des 0, on modifie la precision en consequence. */
	if ((!(pf->flags & PRINTF_LEFT_JUSTIFY)) && (pf->flags & PRINTF_PAD0)) {
		n = pf->width;

		if (sign != 0)
			n--;
		pf->precision = (pf->precision > n) ? pf->precision : n;
		n = pf->width;
	} else {
		n = pf->precision;
		if (sign != 0)
			n++;
	}
	/* Ici n = nombre de caracteres != ' ' affiches. */

	/* Doit-on mettre des espaces de remplissage avant le nombre ? */
	if (!(pf->flags & PRINTF_LEFT_JUSTIFY)) {
		while (pf->width-- > n)
			print(pf, ' ');
	}

	/* On place eventuellement le signe. */
	if (sign != 0)
		print(pf, sign);

	/* On met les eventuels 0 de remplissage. */
	while (pf->precision-- > pos)
		print(pf, '0');

	/* On copie le reste du nombre. */
	while (pos-- != 0)
		print(pf, pf->buffer_nombre[pos]);

	/* On met enfin les eventuels espaces de fin. */
	while (pf->width-- > n)
		print(pf, ' ');
}

/*******************************************************************************
 *   Pour afficher les "%x", "%X".
 ******************************************************************************/
static void
print_oct(struct printf_st *pf, unsigned int i) {
	int pos = 0;
	int n;

	/* La precision par defaut pour un entier est 1. */
	if (pf->precision == -1)
		pf->precision = 1;
	else
		pf->flags &= ~PRINTF_PAD0;

	/* On ecrit l'entier dans le buffer. */
	while (i != 0) {
		pf->buffer_nombre[pos++] = (i % 8) + '0';
		i = i / 8;
	}

	/* On verifie si on doit mettre un zero en tete. */
	if (pf->flags & PRINTF_ALTERNATE)
		pf->buffer_nombre[pos++] = '0';

	/* On met a jour la precision avec celle que demande le nombre affiche. */
	pf->precision = (pos > pf->precision) ? pos : pf->precision;

	/* Si on doit remplir avec des 0, on modifie la precision en consequence. */
	if ((!(pf->flags & PRINTF_LEFT_JUSTIFY)) && (pf->flags & PRINTF_PAD0)) {
		n = pf->width;

		if ((pf->flags & PRINTF_SHOW_SIGN) || (pf->flags & PRINTF_SPACE_PLUS))
			n--;
		pf->precision = (pf->precision > n) ? pf->precision : n;
		n = pf->width;
	} else {
		n = pf->precision;
		if ((pf->flags & PRINTF_SHOW_SIGN) || (pf->flags & PRINTF_SPACE_PLUS))
			n++;
	}
	/* Ici n = nombre de caracteres != ' ' affiches. */

	/* Doit-on mettre des espaces de remplissage avant le nombre ? */
	if (!(pf->flags & PRINTF_LEFT_JUSTIFY)) {
		while (pf->width-- > n)
			print(pf, ' ');
	}

	/* On place eventuellement le signe. */
	if (pf->flags & PRINTF_SHOW_SIGN)
		print(pf, '+');
	else if (pf->flags & PRINTF_SPACE_PLUS)
		print(pf, ' ');

	/* On met les eventuels 0 de remplissage. */
	while (pf->precision-- > pos)
		print(pf, '0');

	/* On copie le reste du nombre. */
	while (pos-- != 0)
		print(pf, pf->buffer_nombre[pos]);

	/* On met enfin les eventuels espaces de fin. */
	while (pf->width-- > n)
		print(pf, ' ');
}

/*******************************************************************************
 * Pour afficher les "%p".
 ******************************************************************************/
static void
print_pointer(struct printf_st *pf, void *p) {
	if (p == 0) {
		print_string(pf, "(nil)");
	} else {
		pf->flags |= PRINTF_ALTERNATE;
		print_hexa(pf, (unsigned long) p);
	}
}

/*******************************************************************************
 * Voici la fonction "principale".
 ******************************************************************************/
static int
__printf(struct printf_st *pf, const char *format, va_list ap) {
	pf->count = 0;
	while (*format != 0) {
		if (*format == '%') {
			const char *ptr = format + 1;

			/* On lit le champ optionnel flags. */
			pf->flags = 0;
			flags_l:
			switch (*ptr) {
			case '-':
				pf->flags |= PRINTF_LEFT_JUSTIFY;
				ptr++;
				goto flags_l;

			case '+':
				pf->flags |= PRINTF_SHOW_SIGN;
				ptr++;
				goto flags_l;

			case ' ':
				pf->flags |= PRINTF_SPACE_PLUS;
				ptr++;
				goto flags_l;

			case '#':
				pf->flags |= PRINTF_ALTERNATE;
				ptr++;
				goto flags_l;

			case '0':
				pf->flags |= PRINTF_PAD0;
				ptr++;
				goto flags_l;
			}

			/* On lit le champ optionnel width. */
			if (*ptr == '*') {
				pf->width = va_arg(ap, int);
				ptr++;
			} else {
				pf->width = 0;
				while ((*ptr >= '0') && (*ptr <= '9'))
					pf->width =
						pf->width * 10 + (*ptr++) - '0';
			}

			/* On lit le champ optionnel de precision. */
			if (*ptr == '.') {
				ptr++;
				if (*ptr == '*') {
					pf->precision = va_arg(ap, int);
					ptr++;
				} else {
					pf->precision = 0;
					while ((*ptr >= '0')
						&& (*ptr <= '9'))
						pf->precision +=
							pf->precision * 10 +
							(*ptr++) - '0';
				}
			} else
				pf->precision = -1;

			/* On lit le champ optionnel modifier. */
			pf->modifier = 0;
			if ((*ptr == 'h') || (*ptr == 'l')
				|| (*ptr == 'L'))
				pf->modifier = *ptr++;

			/* On lit enfin le champ obligatoire. */
			switch (*ptr) {
			case 'p':
				print_pointer(pf, va_arg(ap, void *));
				break;

			case 'X':
				pf->flags |= PRINTF_CAPITAL_X;
				/* FALLTHROUGH */
			case 'x':
				if (pf->modifier == 'h')
					print_hexa(pf, va_arg(ap, int));
				else if (pf->modifier == 'l')
					print_hexa(pf, va_arg
							(ap, unsigned long));
				else
					print_hexa(pf, va_arg
							(ap, unsigned int));
				break;

			case 'd':
			case 'i':
				{
					int i;

					if (pf->modifier == 'h')
						i = va_arg(ap, int);
					else if (pf->modifier == 'l')
						i = va_arg(ap, long);
					else
						i = va_arg(ap, int);
					if (i < 0)
						print_dec(pf, -i, '-');
					else
						print_dec(pf, i, '+');
					break;
				}

			case 'u':
				{
					int i;

					if (pf->modifier == 'h')
						i = va_arg(ap, int);
					else if (pf->modifier == 'l')
						i = va_arg(ap, long);
					else
						i = va_arg(ap, int);
					if (i < 0)
						print_dec(pf, -i, '-');
					else
						print_dec(pf, i, '+');

					break;
				}

			case 's':
				print_string(pf, va_arg(ap, char *));
				break;

			case 'c':
				print_char(pf, va_arg(ap, int));
				break;

			case '%':
				print(pf, '%');
				break;

			case 'o':
				if (pf->modifier == 'h')
					print_oct(pf, va_arg(ap, int));
				else if (pf->modifier == 'l')
					print_oct(pf, va_arg
							(ap, unsigned long));
				else
					print_oct(pf, va_arg
							(ap, unsigned int));
				break;

			case 'n':
				*va_arg(ap, int *) = pf->count;
				break;

			case 'f':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
				pf->flags = 0;
				pf->width = 0;
				pf->precision = -1;
				print_string
					(pf, "<float format not implemented>");
				break;

			default:
				/* Comme le format n'est pas valide, on l'affiche ! */
				while (format < ptr)
					print(pf, *format++);
				ptr--;
			}
			format = ptr + 1;
		} else
			print(pf, *format++);
	}
	return pf->count;
}

static int
_printf(char *s, unsigned long n, const char *format, va_list ap)
{
	struct printf_st pf;
	pf.pos_ecr = 0;
	pf.str = s;
	pf.strl = n;

	if (s != 0) {
		/* Cas du print dans un buffer. */
		if (n) {
			/* On reserve un caractere pour le 0 terminal. */
			n--;
			__printf(&pf, format, ap);
			*s = 0;

			/* On renvoie la taille de la chaine ecrite. */
			return pf.count;
		}

		return 0;
	} else {
		/* Cas du print dans sur la console. */
		__printf(&pf, format, ap);
		pf.buffer_ecr[pf.pos_ecr] = 0;
		cons_puts(pf.buffer_ecr);

		/* On renvoie la taille de la chaine ecrite. */
		return pf.count;
	}
}

/*******************************************************************************
static int printf( const char *format, ... )
*******************************************************************************/
static int
printf(const char *format, ...)
{
	int ret;
	va_list ap;

	va_start(ap, format);
	ret = _printf(0, 0, format, ap);

	va_end(ap);
	return ret;
}

static int
assert_failed(const char *cond, const char *file, int line)
{
	printf("%s:%d: assertion '%s' failed.\n", file, line, cond);
	*(char *)0 = 0;
	exit(-1);
	while (1) ;
}

// static void
// cons_gets(char *s, unsigned long length)
// {
// 	unsigned long n = 0;
// 	cons_echo(0);
// 	while (n < (length-1)) {
// 		int c = cons_read();
// 		if ((c <= 126) && (c >= 32)) {
// 			s[n] = c;
// 			cons_write(s + n, 1);
// 			n++;
// 		} else if ((c == '\n') || (c == 13)) {
// 			s[n] = 0;
// 			cons_write("\n", 1);
// 			cons_echo(1);
// 			return;
// 		} else if ((c == 8) || (c == 127)) {
// 			if (n > 0) {
// 				n--;
// 				printf("%c %c", 8, 8);
// 			}
// 		}
// 	}
// 	s[n] = 0;
// 	cons_echo(1);
// }

/*******************************************************************************
 * Division 64 bits
 ******************************************************************************/
unsigned long long
div64(unsigned long long x, unsigned long long div, unsigned long long *rem)
{
	unsigned long long mul = 1;
	unsigned long long q;

	if ((div > x) || !div) {
		if (rem) *rem = x;
		return 0;
	}

	while (!((div >> 32) & 0x80000000ULL)) {
		unsigned long long newd = div + div;
		if (newd > x) break;
		div = newd;
		mul += mul;
	}

	q = mul;
	x -= div;
	while (1) {
		mul /= 2;
		div /= 2;
		if (!mul) {
			if (rem) *rem = x;
			return q;
		}
		if (x < div) continue;
		q += mul;
		x -= div;
	}
}

/*******************************************************************************
 * Pseudo random number generator
 ******************************************************************************/
/* static unsigned long long mul64(unsigned long long x, unsigned long long y)
{
	unsigned long a, b, c, d, e, f, g, h;
	unsigned long long res = 0;
	a = x & 0xffff;
	x >>= 16;
	b = x & 0xffff;
	x >>= 16;
	c = x & 0xffff;
	x >>= 16;
	d = x & 0xffff;
	e = y & 0xffff;
	y >>= 16;
	f = y & 0xffff;
	y >>= 16;
	g = y & 0xffff;
	y >>= 16;
	h = y & 0xffff;
	res = d * e;
	res += c * f;
	res += b * g;
	res += a * h;
	res <<= 16;
	res += c * e;
	res += b * f;
	res += a * g;
	res <<= 16;
	res += b * e;
	res += a * f;
	res <<= 16;
	res += a * e;
	return res;
}

typedef unsigned long long uint_fast64_t;
typedef unsigned long uint_fast32_t;

static const uint_fast64_t _multiplier = 0x5DEECE66DULL;
static const uint_fast64_t _addend = 0xB;
static const uint_fast64_t _mask = (1ULL << 48) - 1;
static uint_fast64_t _seed = 1; */

// Assume that 1 <= _bits <= 32
/* static uint_fast32_t
randBits(int _bits)
{
	uint_fast32_t rbits;
	uint_fast64_t nextseed = (mul64(_seed, _multiplier) + _addend) & _mask;
	_seed = nextseed;
	rbits = nextseed >> 16;
	return rbits >> (32 - _bits);
} */

/* static void
setSeed(uint_fast64_t _s)
{
	_seed = _s;
} */

/* static unsigned long
rand()
{
	return randBits(32);
} */

/*******************************************************************************
 * Unmask interrupts for those who are working in kernel mode
 ******************************************************************************/
/* static void test_it()
{
	__asm__ volatile("pushfl; testl $0x200,(%%esp); jnz 0f; sti; nop; cli; 0: addl $4,%%esp\n":::"memory");
} */

/*******************************************************************************
 * Test 1
 *
 * Demarrage de processus avec passage de parametre
 * Terminaison normale avec valeur de retour
 * Attente de terminaison (cas fils avant pere et cas pere avant fils)
 ******************************************************************************/
static int
dummy1(void *arg)
{
	printf("1");
	assert((int) arg == DUMMY_VAL);
	return 3;
}

static int
dummy1_2(void *arg)
{
	printf(" 5");
	assert((int) arg == DUMMY_VAL + 1);

	return 4;
}

static void
test1(void)
{
	int pid1;
	int r;
	int rval;

	pid1 = start(dummy1, 4000, 192, "paramRetour", (void *) DUMMY_VAL);
	assert(pid1 > 0);
	printf(" 2");
	r = waitpid(pid1, &rval);
	assert(r == pid1);
	assert(rval == 3);
	printf(" 3");
	pid1 = start(dummy1_2, 4000, 100, "paramRetour", (void *) (DUMMY_VAL + 1));
	assert(pid1 > 0);
	printf(" 4");
	r = waitpid(pid1, &rval);
	assert(r == pid1);
	assert(rval == 4);
	printf(" 6.\n");
}

/*******************************************************************************
 * Test 2
 *
 * kill() de fils suspendu pas demarre
 * waitpid() de ce fils termine par kill()
 * waitpid() de fils termine par exit()
 ******************************************************************************/
static int
dummy2(void *args)
{
	printf(" X");
	return (int)args;
}

static int
dummy2_2(void *args)
{
	printf(" 5");
	exit((int) args);
	assert(0);
	return 0;
}

static void
test2(void)
{
	int rval;
	int r;
	int pid1;
	int val = 45;

	printf("1");
	pid1 = start(dummy2, 4000, 100, "procKill", (void *) val);
	assert(pid1 > 0);
	printf(" 2");
	r = kill(pid1);
	assert(r == 0);
	printf(" 3");
	r = waitpid(pid1, &rval);
	assert(rval == 0);
	assert(r == pid1);
	printf(" 4");
	pid1 = start(dummy2_2, 4000, 192, "procExit", (void *) val);
	assert(pid1 > 0);
	printf(" 6");
	r = waitpid(pid1, &rval);
	assert(rval == val);
	assert(r == pid1);
	assert(waitpid(getpid(), &rval) < 0);
	printf(" 7.\n");
}




/*******************************************************************************
 * Fin des tests
 ******************************************************************************/

// static void auto_test(void);

static void
quit(void)
{
	exit(0);
}

static struct {
	const char *name;
	void (*f) (void);
} commands[] = {
	{"1", test1},
	{"2", test2},
	// {"3", test3},
	// {"4", test4},
	// {"5", test5},
	// {"6", test6},
	// {"7", test7},
	// {"8", test8},
	// {"9", test9},
	// {"10", test10},
	// {"11", test11},
	// {"12", test12},
	// {"13", test13},
	// {"14", test14},
	// {"15", test15},
	// {"16", test16},
	// {"17", test17},
	// {"18", test18},
	// {"19", test19},
	// {"20", test20},
	// {"si", sys_info},
	// {"a", auto_test},
	// {"auto", auto_test},
	{"q", quit},
	{"quit", quit},
	{"exit", quit},
	{0, 0},
};

/* static void
auto_test(void)
{
	int i = 0;

	while (commands[i].f != sys_info) {
		printf("Test %s : ", commands[i].name);
		commands[i++].f();
	}
} */

int
test_run(int n)
{
	assert(getprio(getpid()) == 128);
	if ((n < 1) || (n > 20)) {
		printf("%d: unknown test\n", n);
	} else {
		commands[n - 1].f();
	}
	return 0;
}

/* int
test_proc(void *arg)
{
	char buffer[20];

	(void)arg;
	assert(getprio(getpid()) == 128);

	unsigned long flags;
	unsigned long cs;
	unsigned long long seed;

	__asm__ volatile("pushfl; popl %0; movl %%cs,%1\n":"=r" (flags), "=r" (cs) ::"memory");
	printf("EFLAGS = %#lx, CS = %#lx\n", flags, cs);

	__asm__ __volatile__("rdtsc":"=A"(seed));
	setSeed(seed);

	while (1) {
		int i = 0;
		printf("Test (1-20, auto) : ");
		cons_gets(buffer, 20);
		while (commands[i].name && strcmp(commands[i].name, buffer)) i++;
		if (!commands[i].name) {
			printf("%s: unknown test\n", buffer);
		} else {
			commands[i].f();
		}
	}
} */
