/*% cc -g -Wall -o # avl.c %
 */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#define _XOPEN_SOURCE /* getopt() */
#include <unistd.h>
#include <time.h>

enum {
	Chain = 3,
	C1 = Chain - 1,
	NChar = 128,
	NBuck = 1000,
	MinCap = 128, /* power of 2 */
};

typedef short wid;
typedef struct Freq Freq;
typedef struct Buck Buck;
typedef struct FMap FMap;
typedef struct Tree Tree;

struct Freq {
	wid w;
	int n;
};

struct FMap {
	wid c[Chain];
	int all;
	int nf;
	Freq *f;
};

struct Buck {
	char a[NChar];
	wid w;
	Buck *link;
};

Buck *wh[NBuck];
char **wtoa;
wid newwid;
Tree *map;
int hit0, hit1;
int nmap, sort;

Tree *avlnew(int (*)(void *, void *));
void *avlget(void *, Tree *);
void avlins(void *, Tree *);

int
ccmp(void *a, void *b)
{
	wid *c1, *c2;
	int i, c;

	c1 = a;
	c2 = b;
	for (c=0, i=0; i<Chain && c==0; i++)
		c = c1[i] - c2[i];
	return c;
}

int
full(unsigned n)
{
	if (n < MinCap)
		return 0;
	return (n & (n-1)) == 0;
}

void
grow(int n, void *p, size_t sz)
{
	if (full(n))
		*(void **)p = realloc(*(void **)p, 2*n * sz);
}

FMap *
chain(wid c[])
{
	FMap *r;

	r = avlget(c, map);
	if (!r) {
		r = malloc(sizeof *r);
		memcpy(r->c, c, Chain * sizeof c[0]);
		r->f = calloc(MinCap, sizeof r->f[0]);
		r->nf = 0;
		r->all = 0;
		avlins(r, map);
	}
	return r;
}

Buck **
buck(char *a)
{
	unsigned h;

	h = 0x27A2015;
	for (; *a; a++)
		h = h * 13 + *a;
	return &wh[h % NBuck];
}

int
isword(int c)
{
	return isalnum(c) || c == '-';
}

int
isterm(int c)
{
	return c == '.' || c == '?' || c == ':';
}

wid
freadw(FILE *f)
{
	static char w[NChar];
	Buck *b, **pb;
	char *p;
	int c;

	do {
		c = getc(f);
		if (isterm(c))
			return 0;
		if (c == EOF)
			return -1;
	} while (!isword(c));
	p = w;
	do {
		if (p < &w[NChar-1])
			*p++ = tolower(c);
		c = getc(f);
	} while (isword(c));
	*p = 0;
	ungetc(c, f);

	for (b=*buck(w); b; b=b->link)
		if (strcmp(w, b->a) == 0)
			break;
	if (!b) {
		b = malloc(sizeof *b);
		strcpy(b->a, w);
		pb = buck(w);
		b->link = *pb;
		*pb = b;
		grow(newwid, &wtoa, sizeof wtoa[0]);
		b->w = newwid++;
		wtoa[b->w] = b->a;
	}
	return b->w;
}

void
shift(wid w, wid *ws, wid **pw)
{
	if (*pw == &ws[Chain * 10]) {
		memcpy(ws, *pw - C1, C1 * sizeof ws[0]);
		*pw = ws + C1;
	}
	if (w == 0)
		memset(*pw - C1, 0, C1 * sizeof ws[0]);
	*(*pw)++ = w;
}

int
main(int ac, char *av[])
{
	wid ws[Chain * 10], *pw, w;
	int i, l, c;
	int o;
	FILE *in;
	FMap *m;
	Freq *q;

	srand(time(0));

	newwid = 1;
	map = avlnew(ccmp);
	wtoa = calloc(MinCap, sizeof wtoa[0]);
	wtoa[0] = "TERM";

	l = -1;
	c = 1;
	while ((o = getopt(ac, av, "l:c:")) != -1)
		switch (o) {
		case 'l':
			l = atoi(optarg);
			break;
		case 'c':
			c = atoi(optarg);
			break;
		default:
		usage:
			fprintf(stderr, "usage: %s [-l LEN] [-c CNT] [FILE]", av[0]);
			exit(1);
		}

	if (optind >= ac)
		in = stdin;
	else {
		in = fopen(av[optind], "r");
		if (!in)
			goto usage;
	}

	/* 1. learn */
	pw = ws + C1;
	shift(0, ws, &pw);

	while ((w = freadw(in)) != -1) {
		m = chain(pw - Chain);
		for (q=m->f;; q++)
			if (q == &m->f[m->nf]) {
				grow(m->nf, &m->f, sizeof m->f[0]);
				q = &m->f[m->nf++];
				q->w = w;
				q->n = 1;
				break;
			} else if (q->w == w) {
				q->n++;
				break;
			}
		m->all++;
		shift(w, ws, &pw);
	}

	/* 2. generate sentences */
	while (c-- > 0) {
		pw = ws + C1;
		shift(0, ws, &pw);

		printf(">>> ");
		for (i=0; l<0 || i<l; i++) {
			m = chain(pw - Chain);
			o = rand() % m->all;
			for (q=m->f; o>=q->n; q++)
				o -= q->n;
			w = q->w;
			if (w == 0 || i > 10000) {
				printf(". ");
				if (l < 0)
					break;
			} else {
				if (i != 0)
					printf(" ");
				printf("%s", wtoa[w]);
			}
			shift(w, ws, &pw);
		}
		printf("\n");
	}

	exit(0);
}
