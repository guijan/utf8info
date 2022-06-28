/*
 * Copyright (c) 2022 Guilherme Janczak <guilherme.janczak@yandex.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

static int grapheme;

static void setloc(void);
static void utf8info(const void *, size_t);
static int u8clen(int *, int *, const void *, size_t);
static size_t min(size_t, size_t);
static void printhex(unsigned char);

/* utf8info: print information about UTF-8 sequences */
int
main(void)
{
	ssize_t linelen;
	char *lineptr;
	size_t n;

	setloc();

	lineptr = NULL;
	n = 0;
	puts("Type text to get its Unicode and UTF-8 properties:");
	for (;;) {
		fputs("> ", stdout);
		fflush(stdout);

		linelen = getline(&lineptr, &n, stdin);
		if (linelen == -1)
			break;
		lineptr[--linelen] = '\0';
		if (linelen == 0)
			continue;
		if (linelen == 1 && lineptr[0] == '!') {
			grapheme = !grapheme;
			printf("%s mode.\n",
			    grapheme ? "Grapheme" : "Codepoint");
			continue;
		}

		utf8info(lineptr, linelen);
	}
	free(lineptr);
	putchar('\n'); /* Terminate the program's output with a newline. */

	if (ferror(stdin))
		err(1, "getline");
	return 0;
}

/* setloc: set utf-8 locale */
static void
setloc(void)
{
	char *p;
	const char *utf8 = ".UTF-8";
	/* Strictness is good. A locale in the style "en_US.UTF-8" is 11
	 * characters (12 with '\0').
	 */
	static char locale[12];
        const char *reg =
            "^([[:lower:]]{2}_[[:upper:]]{2}|POSIX|C)(\\.[[:graph:]]+){0,1}$";
        size_t len;

	setlocale(LC_ALL, "");

	p = setlocale(LC_CTYPE, NULL);
	if (!strcasestr(p, utf8)) {
		/*
		 * If truncation occurs, either the encoding part of the locale
		 * (the chars after and including the dot) is too long which is
		 * harmless to truncate because we strip that anyway, or we must
		 * be running in a system that uses a different scheme in which
		 * case validating the locale will throw an error later.
		 */
                strlcpy(locale, p, sizeof(locale));

                /* We don't care about the dot or anything after it and we strip
		 * that part if any as that specifies an encoding according to
		 * X/Open, and we're merely switching to UTF-8 if we're not
		 * already UTF-8.
                 */
                len = strcspn(locale, ".");
                locale[len] = '\0';

		/*
                 * The code seems dumb, but the alternative of creating a DSL on
		 * the spot turned out to be more complicated and I don't want
                 * to use the C regex interfaces.
		 *
		 * The locale must match the regex `reg`:
		 */
		if ((!islower((unsigned char)locale[0])
		    || !islower((unsigned char)locale[1])
		    || locale[2] != '_'
		    || !isupper((unsigned char)locale[3])
		    || !isupper((unsigned char)locale[4])
		    || locale[5] != '\0')
                    && strcmp(locale, "POSIX")
                    && strcmp(locale, "C")) {
			errx(1, "LC_CTYPE '%s' doesn't match regex '%s'", p,
			    reg);
		}

                strlcat(locale, utf8, sizeof(locale));

		if (setlocale(LC_CTYPE, locale) == NULL)
                        errx(1, "setlocale() failure"); /* Doesn't set errno. */
	}
}

/* utf8info: print information about u8str.
 *
 * _u8str isn't null terminated.
 * len is the length of _u8str.
 *
 * The global variable grapheme controls whether we're on grapheme mode or not
 * based on whether it's true.
 */
static void
utf8info(const void *_u8str, size_t len)
{
	size_t i;
	int bytes;
	const char *mbc;
	const char *u8str = _u8str;
	int width;
	int j;
	int codepoints;

	mbtowc(NULL, NULL, MB_CUR_MAX);

	for (i = 0; i < len; i += bytes) {
		mbc = u8str + i;
		bytes = u8clen(&width, &codepoints, mbc, len-i);

		if (width == -1 || width == 0) {
			/* The first argument would be a goddamn trigraph if it
			 * was just one string literal. Anyway, don't print
			 * something if its width is -1 because that's a
			 * nonprintable character, and don't print it if its
			 * width is 0 because that's a combining mark OR it may
			 * be L'\0'.
			 */
			fputs("'?" "?'", stdout);
		} else {
			putchar('\'');
			fwrite(mbc, bytes, 1, stdout);
			if (width == 1)
				putchar(' ');
			putchar('\'');
		}

		printf(" width: %2d, bytes: %2d", width, bytes);

		if (grapheme)
			printf(", codepoints: %2d", codepoints);

		fputs(", hex: ", stdout);
		for (j = 0; j < bytes; j++)
			printhex(mbc[j]);
		putchar('\n');
        }
}

/* u8clen: get the byte length of a grapheme cluster or codepoint.
 *
 * If width is not NULL, write the wcwidth of the character to it.
 * If codepoints is not NULL, write how many codepoints were found to it.
 * _u8c is the unicode grapheme cluster or codepoint.
 * len is the length of the _u8c buffer.
 *
 * The global variable "grapheme" gets the grapheme's byte length if true, or
 * the codepoint's byte length otherwise.
 */
static int
u8clen(int *width, int *codepoints, const void *_u8c, size_t len)
{
	size_t i, j;
	const char *mbc;
	const char *u8c = _u8c;
	int bytes;
	wchar_t wc;
	int w;
	int joined;
	const char zwj[] = {0xE2, 0x80, 0x8D}; /* zero-width joiner */
	const char zwnj[] = {0xE2, 0x80, 0x8C}; /* zero-width nonjoiner */

	if (len == 0)
		return 0;

	for (i = j = 0; i < len;) {
		mbc = u8c + i;
		bytes = mbtowc(&wc, mbc, min(len-i, MB_CUR_MAX));
		if (bytes == 0) /* mbtowc says a '\0' is 0 bytes. */
			bytes = 1;

		w = wcwidth(wc);

		if (j == 0 && width != NULL)
			*width = w;

		/* End if the character's width isn't 0, this isn't the first
		 * character, and we're not after a zero width joiner.
		 */
		if (w != 0 && j > 0 && !joined)
			break;

		i += bytes;
		j++;

		/* End on nonprintable character. */
		if (w == -1)
			break;

		/* End if we're dealing with code points and not
		 * graphemes.
		 */
		if (!grapheme)
			break;

		joined = bytes == sizeof(zwj) && (!memcmp(mbc, zwj, sizeof(zwj))
		    || !memcmp(mbc, zwnj, sizeof(zwnj)));
	}
	if (codepoints)
		*codepoints = j;

	return i;
}

static size_t
min(size_t a, size_t b)
{
	return (a < b ? a : b);
}

static void
printhex(unsigned char byte)
{
	static const char alphabet[16] = "0123456789ABCDEF";

	putchar(alphabet[byte & 0x0F]);
	putchar(alphabet[byte >> 4]);
}
