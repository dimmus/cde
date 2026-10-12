/*
 * CDE - Common Desktop Environment
 *
 * dtrandr: reading and writing profiles (the autorandr format)
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

/*
 * A profile is a directory with two files, as autorandr has them:
 *
 *   setup   the fingerprint, one "OUTPUT EDID" line per connected output
 *   config  "output NAME" lines, each followed by "key [value]" lines
 *
 * The options are kept as the strings of the file and only interpreted
 * when a profile is applied (layout.c), so that a profile reads back as
 * it was written, unknown keys (autorandr's x-prop-*) included.  They
 * are written as autorandr writes them: one block per output, the keys
 * of a block sorted.
 */

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dtrandr.h"

static const char *const virtual_profiles[] = {
    "off", "common", "clone-largest", "horizontal", "vertical",
    "horizontal-reverse", "vertical-reverse", "auto", "none", NULL
};

int
is_virtual_profile(const char *name)
{
    int i;

    for (i = 0; virtual_profiles[i]; i++)
        if (!strcmp(name, virtual_profiles[i]))
            return 1;
    return 0;
}

/* A name that can be a directory name in a search directory */
int
profile_name_ok(const char *name)
{
    return name[0] && name[0] != '.' && !strchr(name, '/')
        && strcmp(name, "settings");
}

DrProfile *
profile_new(const char *name)
{
    DrProfile *p = xcalloc(1, sizeof *p);

    p->name = xstrdup(name);
    p->source = -1;
    return p;
}

static void
opts_free(int nopt, DrOpt *opts)
{
    int i;

    for (i = 0; i < nopt; i++) {
        free(opts[i].key);
        free(opts[i].val);
    }
    free(opts);
}

void
profile_free(DrProfile *p)
{
    int i;

    if (!p)
        return;
    for (i = 0; i < p->nsetup; i++) {
        free(p->setup[i].output);
        free(p->setup[i].edid);
    }
    free(p->setup);
    for (i = 0; i < p->nout; i++) {
        free(p->out[i].name);
        opts_free(p->out[i].nopt, p->out[i].opts);
    }
    free(p->out);
    opts_free(p->nhead, p->head);
    free(p->name);
    free(p->dir);
    free(p);
}

DrOutputConf *
profile_add_output(DrProfile *p, const char *name)
{
    DrOutputConf *o;

    p->out = xrealloc(p->out, (p->nout + 1) * sizeof *p->out);
    o = &p->out[p->nout++];
    o->name = xstrdup(name);
    o->nopt = 0;
    o->opts = NULL;
    return o;
}

void
opt_add(int *nopt, DrOpt **opts, const char *key, const char *val)
{
    *opts = xrealloc(*opts, (*nopt + 1) * sizeof **opts);
    (*opts)[*nopt].key = xstrdup(key);
    (*opts)[*nopt].val = val ? xstrdup(val) : NULL;
    (*nopt)++;
}

/* The last option named key (a later line replaces an earlier one) */
const DrOpt *
opt_find(int nopt, const DrOpt *opts, const char *key)
{
    int i;

    for (i = nopt - 1; i >= 0; i--)
        if (!strcmp(opts[i].key, key))
            return &opts[i];
    return NULL;
}

/*
 * Split a line into its first word and the rest (both without the
 * surrounding white space).  Returns 0 for an empty or comment line.
 */
static int
split_line(char *line, char **key, char **val)
{
    char *s = line, *e;

    while (isspace((unsigned char) *s))
        s++;
    e = s + strlen(s);
    while (e > s && isspace((unsigned char) e[-1]))
        *--e = '\0';
    if (!*s || *s == '#')
        return 0;
    *key = s;
    while (*s && !isspace((unsigned char) *s))
        s++;
    if (*s) {
        *s++ = '\0';
        while (isspace((unsigned char) *s))
            s++;
        *val = s;
    } else
        *val = NULL;
    return 1;
}

static char *
read_line(FILE *f)
{
    char *line = NULL;
    size_t size = 0;

    if (getline(&line, &size, f) < 0) {
        free(line);
        return NULL;
    }
    return line;
}

int
profile_parse_config(DrProfile *p, FILE *f, const char *path)
{
    DrOutputConf *cur = NULL;
    char *line, *key, *val;
    int lineno = 0;

    while ((line = read_line(f))) {
        lineno++;
        if (!split_line(line, &key, &val)) {
            free(line);
            continue;
        }
        if (!strcmp(key, "output")) {
            if (!val || strchr(val, ' ') || strchr(val, '\t')) {
                dr_warn("%s:%d: bad output line", path, lineno);
                cur = NULL;
            } else
                cur = profile_add_output(p, val);
        } else if (cur)
            opt_add(&cur->nopt, &cur->opts, key, val);
        else
            opt_add(&p->nhead, &p->head, key, val);
        free(line);
    }
    return 0;
}

int
profile_parse_setup(DrProfile *p, FILE *f, const char *path)
{
    char *line, *key, *val;
    int lineno = 0;

    while ((line = read_line(f))) {
        lineno++;
        if (split_line(line, &key, &val)) {
            if (!val || strpbrk(val, " \t"))
                dr_warn("%s:%d: expected \"OUTPUT EDID\"", path, lineno);
            else {
                p->setup = xrealloc(p->setup,
                                    (p->nsetup + 1) * sizeof *p->setup);
                p->setup[p->nsetup].output = xstrdup(key);
                p->setup[p->nsetup].edid = xstrdup(val);
                p->nsetup++;
            }
        }
        free(line);
    }
    return 0;
}

static int
cmp_opt(const void *a, const void *b)
{
    const DrOpt *const *x = a, *const *y = b;
    int r = strcmp((*x)->key, (*y)->key);

    /* stable: keep the file order of equal keys */
    return r ? r : (*x < *y ? -1 : *x > *y);
}

/* Write options sorted by key, a later duplicate replacing an earlier */
static void
write_opts(FILE *f, int nopt, const DrOpt *opts)
{
    const DrOpt **v = xcalloc(nopt, sizeof *v);
    int i, n = 0;

    for (i = 0; i < nopt; i++)
        if (opt_find(nopt, opts, opts[i].key) == &opts[i])
            v[n++] = &opts[i];
    qsort(v, n, sizeof *v, cmp_opt);
    for (i = 0; i < n; i++) {
        if (v[i]->val && *v[i]->val)
            fprintf(f, "%s %s\n", v[i]->key, v[i]->val);
        else
            fprintf(f, "%s\n", v[i]->key);
    }
    free(v);
}

void
profile_write_config(const DrProfile *p, FILE *f)
{
    int i;

    write_opts(f, p->nhead, p->head);
    for (i = 0; i < p->nout; i++) {
        fprintf(f, "output %s\n", p->out[i].name);
        write_opts(f, p->out[i].nopt, p->out[i].opts);
    }
}

static int
cmp_setup(const void *a, const void *b)
{
    const DrSetupLine *x = a, *y = b;

    return strcmp(x->output, y->output);
}

void
profile_write_setup(const DrProfile *p, FILE *f)
{
    DrSetupLine *v = xcalloc(p->nsetup, sizeof *v);
    int i;

    if (p->nsetup)
        memcpy(v, p->setup, p->nsetup * sizeof *v);
    qsort(v, p->nsetup, sizeof *v, cmp_setup);
    for (i = 0; i < p->nsetup; i++)
        fprintf(f, "%s %s\n", v[i].output, v[i].edid);
    free(v);
}

/* Read the profile dir/name; NULL (silently) if it has no config file */
DrProfile *
profile_load(const char *dir, const char *name, int source)
{
    char *pdir = xasprintf("%s/%s", dir, name);
    char *cpath = xasprintf("%s/config", pdir);
    char *spath = xasprintf("%s/setup", pdir);
    DrProfile *p = NULL;
    struct stat st;
    FILE *f;

    if (stat(cpath, &st) || !S_ISREG(st.st_mode))
        goto out;
    if (!(f = fopen(cpath, "r"))) {
        dr_warn("cannot read %s: %s", cpath, strerror(errno));
        goto out;
    }
    p = profile_new(name);
    p->dir = pdir;
    pdir = NULL;
    p->source = source;
    profile_parse_config(p, f, cpath);
    fclose(f);
    if ((f = fopen(spath, "r"))) {
        profile_parse_setup(p, f, spath);
        fclose(f);
    }
  out:
    free(pdir);
    free(cpath);
    free(spath);
    return p;
}

static int
mkdirs(const char *path)
{
    char *p = xstrdup(path), *s;
    int r = 0;

    for (s = p + 1; ; s++) {
        if (*s == '/' || !*s) {
            char c = *s;

            *s = '\0';
            if (mkdir(p, 0755) && errno != EEXIST) {
                r = dr_error("cannot create %s: %s", p, strerror(errno));
                break;
            }
            *s = c;
            if (!c)
                break;
        }
    }
    free(p);
    return r;
}

static int
write_file(const char *path, const DrProfile *p,
           void (*writer)(const DrProfile *, FILE *))
{
    char *tmp = xasprintf("%s.new", path);
    FILE *f = fopen(tmp, "w");
    int r = 0;

    if (!f) {
        r = dr_error("cannot write %s: %s", tmp, strerror(errno));
        free(tmp);
        return r;
    }
    writer(p, f);
    if (ferror(f) | fclose(f))
        r = dr_error("cannot write %s", tmp);
    else if (rename(tmp, path))
        r = dr_error("cannot rename %s: %s", tmp, strerror(errno));
    if (r)
        unlink(tmp);
    free(tmp);
    return r;
}

/* Save p as dir/NAME/{config,setup} */
int
profile_save(const DrProfile *p, const char *dir, int force)
{
    char *pdir = xasprintf("%s/%s", dir, p->name);
    char *cpath = xasprintf("%s/config", pdir);
    char *spath = xasprintf("%s/setup", pdir);
    int r = -1;

    if (!force && (!access(cpath, F_OK) || !access(spath, F_OK))) {
        dr_error("profile \"%s\" exists in %s, not overwriting it "
                 "without --force", p->name, dir);
        goto out;
    }
    if (mkdirs(pdir))
        goto out;
    if (write_file(cpath, p, profile_write_config)
        || write_file(spath, p, profile_write_setup))
        goto out;
    r = 0;
  out:
    free(pdir);
    free(cpath);
    free(spath);
    return r;
}

static int
cmp_str(const void *a, const void *b)
{
    return strcmp(*(char *const *) a, *(char *const *) b);
}

/* Add the profiles of dir (in name order) that l does not have yet */
void
profiles_scan(DrProfileList *l, const char *dir, int source)
{
    DIR *d = opendir(dir);
    struct dirent *e;
    char **names = NULL;
    int n = 0, i;

    if (!d) {
        dr_debug("no profile directory %s", dir);
        return;
    }
    while ((e = readdir(d))) {
        if (!profile_name_ok(e->d_name))
            continue;
        names = xrealloc(names, (n + 1) * sizeof *names);
        names[n++] = xstrdup(e->d_name);
    }
    closedir(d);
    if (n)
        qsort(names, n, sizeof *names, cmp_str);
    for (i = 0; i < n; i++) {
        DrProfile *p;

        if (profiles_find(l, names[i])) {
            dr_debug("%s/%s is hidden by an earlier profile", dir, names[i]);
        } else if ((p = profile_load(dir, names[i], source))) {
            dr_debug("profile %s from %s", names[i], dir);
            l->profiles = xrealloc(l->profiles,
                                   (l->nprofile + 1) * sizeof *l->profiles);
            l->profiles[l->nprofile++] = p;
        }
        free(names[i]);
    }
    free(names);
}

DrProfile *
profiles_find(const DrProfileList *l, const char *name)
{
    int i;

    for (i = 0; i < l->nprofile; i++)
        if (!strcmp(l->profiles[i]->name, name))
            return l->profiles[i];
    return NULL;
}

void
profiles_free(DrProfileList *l)
{
    int i;

    for (i = 0; i < l->nprofile; i++)
        profile_free(l->profiles[i]);
    free(l->profiles);
    l->profiles = NULL;
    l->nprofile = 0;
}

/* The value of key in a "key=value" settings file, NULL if not set */
char *
settings_get(const char *path, const char *key)
{
    FILE *f = fopen(path, "r");
    char *line, *value = NULL;

    if (!f)
        return NULL;
    while ((line = read_line(f))) {
        char *s = line, *eq, *k, *v, *e;

        while (isspace((unsigned char) *s))
            s++;
        if (*s && *s != '#' && (eq = strchr(s, '='))) {
            *eq = '\0';
            k = s;
            v = eq + 1;
            for (e = eq; e > k && isspace((unsigned char) e[-1]); )
                *--e = '\0';
            while (isspace((unsigned char) *v))
                v++;
            for (e = v + strlen(v); e > v && isspace((unsigned char) e[-1]); )
                *--e = '\0';
            if (!strcmp(k, key)) {
                free(value);
                value = xstrdup(v);
            }
        }
        free(line);
    }
    fclose(f);
    return value;
}
