/*
 * CDE - Common Desktop Environment
 *
 * dtrandr: memory, string and message helpers
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * These libraries and programs are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with these libraries and programs; if not, write
 * to the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301 USA
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dtrandr.h"

int dr_verbose = 0;
int dr_quiet = 0;
const char *dr_progname = "dtrandr";

static void
oom(void)
{
    fprintf(stderr, "%s: out of memory\n", dr_progname);
    exit(1);
}

void *
xmalloc(size_t n)
{
    void *p = malloc(n ? n : 1);

    if (!p)
        oom();
    return p;
}

void *
xcalloc(size_t n, size_t size)
{
    void *p = calloc(n ? n : 1, size ? size : 1);

    if (!p)
        oom();
    return p;
}

void *
xrealloc(void *p, size_t n)
{
    p = realloc(p, n ? n : 1);
    if (!p)
        oom();
    return p;
}

char *
xstrdup(const char *s)
{
    char *d = strdup(s ? s : "");

    if (!d)
        oom();
    return d;
}

char *
xasprintf(const char *fmt, ...)
{
    va_list ap;
    char *s;
    int n;

    va_start(ap, fmt);
    n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0)
        oom();
    s = xmalloc((size_t) n + 1);
    va_start(ap, fmt);
    vsnprintf(s, (size_t) n + 1, fmt, ap);
    va_end(ap);
    return s;
}

static void
vmessage(const char *prefix, const char *fmt, va_list ap)
{
    fprintf(stderr, "%s: %s", dr_progname, prefix);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
}

/* Report an error (unless dr_quiet) and return -1 */
int
dr_error(const char *fmt, ...)
{
    va_list ap;

    if (dr_quiet > 0)
        return -1;
    va_start(ap, fmt);
    vmessage("", fmt, ap);
    va_end(ap);
    return -1;
}

void
dr_warn(const char *fmt, ...)
{
    va_list ap;

    if (dr_quiet > 0)
        return;
    va_start(ap, fmt);
    vmessage("warning: ", fmt, ap);
    va_end(ap);
}

void
dr_info(const char *fmt, ...)
{
    va_list ap;

    if (dr_verbose < 1 || dr_quiet > 0)
        return;
    va_start(ap, fmt);
    vmessage("", fmt, ap);
    va_end(ap);
}

void
dr_debug(const char *fmt, ...)
{
    va_list ap;

    if (dr_verbose < 2)
        return;
    va_start(ap, fmt);
    vmessage("debug: ", fmt, ap);
    va_end(ap);
}

void
buf_add(DrBuf *b, const char *s)
{
    size_t n = strlen(s);

    if (b->len + n + 1 > b->size) {
        b->size = (b->len + n + 1) * 2;
        b->s = xrealloc(b->s, b->size);
    }
    memcpy(b->s + b->len, s, n + 1);
    b->len += n;
}

void
buf_printf(DrBuf *b, const char *fmt, ...)
{
    va_list ap;
    char *s;
    int n;

    va_start(ap, fmt);
    n = vsnprintf(NULL, 0, fmt, ap);
    va_end(ap);
    if (n < 0)
        return;
    s = xmalloc((size_t) n + 1);
    va_start(ap, fmt);
    vsnprintf(s, (size_t) n + 1, fmt, ap);
    va_end(ap);
    buf_add(b, s);
    free(s);
}

/* Return the string built so far (never NULL) and reset the buffer */
char *
buf_take(DrBuf *b)
{
    char *s = b->s ? b->s : xstrdup("");

    b->s = NULL;
    b->len = b->size = 0;
    return s;
}

/*
 * A double as Python's str() writes it (the shortest form that reads
 * back the same, with ".0" for whole numbers), which is how autorandr
 * writes the gamma values.
 */
char *
dr_format_double(double d)
{
    char tmp[64];
    int prec;

    for (prec = 1; prec <= 17; prec++) {
        snprintf(tmp, sizeof tmp, "%.*g", prec, d);
        if (strtod(tmp, NULL) == d)
            break;
    }
    if (!strpbrk(tmp, ".einn"))
        strcat(tmp, ".0");
    return xstrdup(tmp);
}
