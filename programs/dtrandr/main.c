/*
 * CDE - Common Desktop Environment
 *
 * dtrandr: detect the connected monitors and apply the matching saved
 * display setup, like autorandr, with its profiles
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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "backend.h"

#ifndef CDE_CONFIGURATION_TOP
#define CDE_CONFIGURATION_TOP "/etc/dt"
#endif
#ifndef DTRANDR_VERSION
#define DTRANDR_VERSION "2.5"
#endif

/* Wait this long after a monitor came or went for more such events */
#define DEBOUNCE_MS     500
#define DEBOUNCE_MAX_MS 5000

typedef struct {
    DrBackend *be;
    int fake;
    int dry_run, force, system;
    const char *default_name;
    int home_ok;
    char *user_dir;                     /* NULL without a usable HOME */
    char *system_dir;
    char *hook_dir;
    int ndir;
    char **dirs;                        /* the search order */
    DrProfileList profiles;
} Ctx;

enum {
    CMD_NONE, CMD_CHANGE, CMD_LOAD, CMD_SAVE, CMD_REMOVE, CMD_LIST,
    CMD_DETECTED, CMD_CURRENT, CMD_FINGERPRINT, CMD_CONFIG, CMD_DAEMON,
    CMD_HELP, CMD_VERSION
};

static void
usage(FILE *f)
{
    fprintf(f,
"Usage: %s [OPTION]... [PROFILE]\n"
"Detect the connected monitors and apply the matching saved display setup\n"
"(autorandr profiles work too).\n"
"\n"
"  -c, --change          apply the profile that matches the connected\n"
"                        monitors (the default)\n"
"  -l, --load NAME       apply profile NAME (also: PROFILE argument)\n"
"  -s, --save NAME       save the current setup as profile NAME\n"
"  -r, --remove NAME     remove profile NAME\n"
"      --system          --save/--remove: use the system directory\n"
"  -f, --force           overwrite with --save; apply even if current;\n"
"                        allow --load off\n"
"      --default NAME    the profile to use when none matches\n"
"      --list            list the profiles\n"
"      --detected        list the profiles that match, best first\n"
"      --current         list the profiles that are applied now\n"
"      --fingerprint     print the setup lines of the connected monitors\n"
"      --config          print the current setup in profile format\n"
"  -d, --daemon          apply, then follow monitors coming and going\n"
"  -n, --dry-run         only print what would be done\n"
"  -v, --verbose         say more\n"
"      --debug           say everything\n"
"  -h, --help            print this help\n"
"      --version         print the version\n"
"\n"
"Virtual profiles: off, common, clone-largest, horizontal, vertical,\n"
"horizontal-reverse, vertical-reverse, and auto (keep what is on, add\n"
"new monitors on the right).  See dtrandr(1).\n", dr_progname);
}

/* ------------------------------------------------------------------ */
/* directories                                                          */

static void
add_dir(Ctx *ctx, char *dir)
{
    ctx->dirs = xrealloc(ctx->dirs, (ctx->ndir + 1) * sizeof *ctx->dirs);
    ctx->dirs[ctx->ndir++] = dir;
}

static void
setup_dirs(Ctx *ctx)
{
    const char *home = getenv("HOME"), *s;
    struct stat st;

    ctx->home_ok = home && home[0] == '/' && !stat(home, &st)
        && S_ISDIR(st.st_mode);
    if (ctx->home_ok) {
        const char *xdg = getenv("XDG_CONFIG_HOME");
        char *cfg = (xdg && xdg[0] == '/') ? xstrdup(xdg)
            : xasprintf("%s/.config", home);

        ctx->user_dir = xasprintf("%s/dtrandr", cfg);
        add_dir(ctx, xstrdup(ctx->user_dir));
        add_dir(ctx, xasprintf("%s/autorandr", cfg));
        add_dir(ctx, xasprintf("%s/.autorandr", home));
        free(cfg);
    }
    s = getenv("DTRANDR_SYSTEM_DIR");
    ctx->system_dir = (s && *s) ? xstrdup(s)
        : xstrdup(CDE_CONFIGURATION_TOP "/dtrandr");
    add_dir(ctx, xstrdup(ctx->system_dir));
    s = getenv("XDG_CONFIG_DIRS");
    if (!s || !*s)
        s = "/etc/xdg";
    while (*s) {
        size_t n = strcspn(s, ":");

        if (s[0] == '/')
            add_dir(ctx, xasprintf("%.*s/autorandr", (int) n, s));
        s += n;
        if (*s)
            s++;
    }
    ctx->hook_dir = xstrdup(ctx->home_ok ? ctx->user_dir : ctx->system_dir);
}

static void
scan_profiles(Ctx *ctx)
{
    int i;

    profiles_free(&ctx->profiles);
    for (i = 0; i < ctx->ndir; i++)
        profiles_scan(&ctx->profiles, ctx->dirs[i], i);
}

static char *
setting(Ctx *ctx, const char *key)
{
    char *path, *v = NULL;

    if (ctx->user_dir) {
        path = xasprintf("%s/settings", ctx->user_dir);
        v = settings_get(path, key);
        free(path);
    }
    if (!v) {
        path = xasprintf("%s/settings", ctx->system_dir);
        v = settings_get(path, key);
        free(path);
    }
    return v;
}

/* ------------------------------------------------------------------ */
/* hooks                                                                */

static void
run_hook(const char *path)
{
    struct stat st;
    pid_t pid;
    int status;

    if (stat(path, &st) || !S_ISREG(st.st_mode) || access(path, X_OK))
        return;
    dr_info("running %s", path);
    fflush(stdout);
    fflush(stderr);
    if ((pid = fork()) < 0) {
        dr_warn("cannot run %s: %s", path, strerror(errno));
        return;
    }
    if (pid == 0) {
        execl(path, path, (char *) NULL);
        fprintf(stderr, "%s: cannot run %s: %s\n", dr_progname, path,
                strerror(errno));
        _exit(127);
    }
    while (waitpid(pid, &status, 0) < 0)
        if (errno != EINTR)
            return;
    if (!WIFEXITED(status) || WEXITSTATUS(status))
        dr_warn("%s failed", path);
}

static int
cmp_name(const void *a, const void *b)
{
    return strcmp(*(char *const *) a, *(char *const *) b);
}

/* Run dir/stage, then the programs in dir/stage.d in name order */
static void
run_hooks_in(const char *dir, const char *stage)
{
    char *path = xasprintf("%s/%s", dir, stage);
    char *ddir = xasprintf("%s/%s.d", dir, stage);
    char **names = NULL;
    struct dirent *e;
    int n = 0, i;
    DIR *d;

    run_hook(path);
    if ((d = opendir(ddir))) {
        while ((e = readdir(d)))
            if (e->d_name[0] != '.') {
                names = xrealloc(names, (n + 1) * sizeof *names);
                names[n++] = xstrdup(e->d_name);
            }
        closedir(d);
        if (n)
            qsort(names, n, sizeof *names, cmp_name);
        for (i = 0; i < n; i++) {
            char *p = xasprintf("%s/%s", ddir, names[i]);

            run_hook(p);
            free(p);
            free(names[i]);
        }
        free(names);
    }
    free(path);
    free(ddir);
}

/* The global hooks, then the profile's */
static void
run_hooks(Ctx *ctx, const char *stage, const char *profile_dir)
{
    if (ctx->dry_run)
        return;
    run_hooks_in(ctx->hook_dir, stage);
    if (profile_dir)
        run_hooks_in(profile_dir, stage);
}

static void
hook_env(const DrHardware *hw, const char *name, const char *dir)
{
    DrBuf b = { 0 };
    char *mon;
    int i;

    for (i = 0; i < hw->noutput; i++)
        if (hw->outputs[i].connected)
            buf_printf(&b, "%s%s", b.len ? ":" : "", hw->outputs[i].name);
    mon = buf_take(&b);
    setenv("AUTORANDR_CURRENT_PROFILE", name, 1);
    setenv("AUTORANDR_PROFILE_FOLDER", dir ? dir : "", 1);
    setenv("AUTORANDR_MONITORS", mon, 1);
    setenv("DTRANDR_PROFILE", name, 1);
    free(mon);
}

/* ------------------------------------------------------------------ */
/* applying                                                             */

static int
query(Ctx *ctx, DrHardware *hw, int probe)
{
    if (ctx->be->query(ctx->be, hw, probe)) {
        hw_free(hw);
        return -1;
    }
    return 0;
}

/* The plan for a profile, virtual profile or "auto"; *dir: its folder */
static int
plan_named(Ctx *ctx, const DrHardware *hw, const char *name, DrPlan *plan,
           const char **dir)
{
    const DrProfile *p;

    *dir = NULL;
    if (!strcmp(name, "auto"))
        return plan_auto(hw, plan);
    if (!strcmp(name, "none")) {
        plan->want = NULL;
        return dr_error("\"none\" is not a profile");
    }
    if (is_virtual_profile(name))
        return plan_virtual(hw, name, plan);
    if (!(p = profiles_find(&ctx->profiles, name))) {
        plan->want = NULL;
        return dr_error("there is no profile %s", name);
    }
    *dir = p->dir;
    return plan_from_profile(hw, p, plan);
}

/* Apply a plan unless it is current; why: how the profile was chosen */
static int
apply_plan(Ctx *ctx, const DrHardware *hw, const DrPlan *plan,
           const char *name, const char *dir, const char *why)
{
    DrConfig target;
    int r = 0;

    if (plan_to_config(hw, plan, &target)) {
        config_free(&target);
        return dr_error("cannot apply profile %s", name);
    }
    if (!ctx->force && config_equal(hw, &hw->cur, &target)) {
        printf("%s: profile %s%s is current\n", dr_progname, name, why);
    } else if (ctx->dry_run) {
        char *cmd = xrandr_command(hw, plan, &target);

        printf("%s: would apply profile %s%s\n%s\n", dr_progname, name, why,
               cmd);
        free(cmd);
    } else {
        hook_env(hw, name, dir);
        run_hooks(ctx, "preswitch", dir);
        if (ctx->be->apply(ctx->be, hw, &target))
            r = dr_error("could not apply profile %s", name);
        else {
            printf("%s: applied profile %s%s\n", dr_progname, name, why);
            fflush(stdout);
            run_hooks(ctx, "postswitch", dir);
        }
    }
    fflush(stdout);
    config_free(&target);
    return r;
}

static char *
fingerprint_string(const DrHardware *hw)
{
    char *s = NULL;
    size_t n = 0;
    FILE *f = open_memstream(&s, &n);

    if (!f)
        return xstrdup("");
    print_fingerprint(hw, f);
    fclose(f);
    return s;
}

/* The profiles that match hw, best first */
static int
matching(Ctx *ctx, const DrHardware *hw, DrProfile ***out)
{
    DrProfile **v = xcalloc(ctx->profiles.nprofile, sizeof *v);
    int *ex = xcalloc(ctx->profiles.nprofile, sizeof *ex);
    double *cl = xcalloc(ctx->profiles.nprofile, sizeof *cl);
    int i, j, n = 0;

    for (i = 0; i < ctx->profiles.nprofile; i++) {
        DrProfile *p = ctx->profiles.profiles[i];
        int e;
        double c;

        if (!profile_match(hw, p, &e, &c))
            continue;
        dr_debug("profile %s matches (%d exact, closeness %.3f)", p->name,
                 e, c);
        /* insertion: better first, else keep the search order */
        for (j = n; j > 0 && (ex[j - 1] < e
                              || (ex[j - 1] == e && cl[j - 1] < c - 1e-9));
             j--) {
            v[j] = v[j - 1];
            ex[j] = ex[j - 1];
            cl[j] = cl[j - 1];
        }
        v[j] = p;
        ex[j] = e;
        cl[j] = c;
        n++;
    }
    free(ex);
    free(cl);
    *out = v;
    return n;
}

static int
profile_is_current(Ctx *ctx, const DrHardware *hw, const DrProfile *p)
{
    DrPlan plan;
    DrConfig target;
    int r = 0;

    dr_quiet++;
    if (!plan_from_profile(hw, p, &plan)) {
        if (!plan_to_config(hw, &plan, &target))
            r = config_equal(hw, &hw->cur, &target);
        config_free(&target);
        plan_free(&plan);
    }
    dr_quiet--;
    (void) ctx;
    return r;
}

/* --change; *fp: the fingerprint it went by */
static int
do_change(Ctx *ctx, int probe, char **fp)
{
    DrHardware hw;
    DrProfile **m;
    DrPlan plan;
    const char *dir, *def;
    char *setdef = NULL, *why;
    int n, r;

    unsetenv("AUTORANDR_CURRENT_PROFILE");
    unsetenv("AUTORANDR_PROFILE_FOLDER");
    unsetenv("AUTORANDR_MONITORS");
    unsetenv("DTRANDR_PROFILE");
    run_hooks(ctx, "predetect", NULL);
    if (query(ctx, &hw, probe))
        return 1;
    if (fp)
        *fp = fingerprint_string(&hw);
    scan_profiles(ctx);
    n = matching(ctx, &hw, &m);
    if (n > 0) {
        dr_info("profile %s matches the connected monitors", m[0]->name);
        if (!plan_from_profile(&hw, m[0], &plan)) {
            r = apply_plan(ctx, &hw, &plan, m[0]->name, m[0]->dir,
                           " (detected)") ? 1 : 0;
            plan_free(&plan);
            free(m);
            hw_free(&hw);
            return r;
        }
        dr_warn("cannot use profile %s, using the default", m[0]->name);
    }
    free(m);
    def = ctx->default_name;
    if (!def && (setdef = setting(ctx, "default")) && *setdef)
        def = setdef;
    why = xasprintf(" (%s)", ctx->default_name ? "--default"
                    : def ? "default setting" : "no profile matches");
    if (!def)
        def = "auto";
    if (!strcmp(def, "none")) {
        dr_info("no profile matches, and the default is none");
        r = 0;
    } else if (plan_named(ctx, &hw, def, &plan, &dir))
        r = 1;
    else {
        r = apply_plan(ctx, &hw, &plan, def, dir, why) ? 1 : 0;
        plan_free(&plan);
    }
    free(why);
    free(setdef);
    hw_free(&hw);
    return r;
}

static int
do_load(Ctx *ctx, const char *name)
{
    DrHardware hw;
    DrPlan plan;
    const char *dir;
    int r;

    if (!strcmp(name, "off") && !ctx->force)
        return dr_error("\"off\" turns every output off; use --force "
                        "if that is what you want"), 1;
    run_hooks(ctx, "predetect", NULL);
    if (query(ctx, &hw, 1))
        return 1;
    scan_profiles(ctx);
    if (plan_named(ctx, &hw, name, &plan, &dir)) {
        hw_free(&hw);
        return 1;
    }
    r = apply_plan(ctx, &hw, &plan, name, dir, "") ? 1 : 0;
    plan_free(&plan);
    hw_free(&hw);
    return r;
}

static int
do_save(Ctx *ctx, const char *name)
{
    DrHardware hw;
    DrProfile *p;
    const char *dir = ctx->system ? ctx->system_dir : ctx->user_dir;
    int r;

    if (is_virtual_profile(name))
        return dr_error("%s is the name of a virtual profile", name), 1;
    if (!profile_name_ok(name))
        return dr_error("%s cannot be a profile name", name), 1;
    if (!dir)
        return dr_error("there is no home directory; use --system"), 1;
    if (query(ctx, &hw, 1))
        return 1;
    p = profile_from_config(&hw, &hw.cur, name);
    if (ctx->dry_run) {
        printf("%s: would save profile %s in %s\n", dr_progname, name, dir);
        profile_write_config(p, stdout);
        r = 0;
    } else if (profile_save(p, dir, ctx->force))
        r = 1;
    else {
        printf("%s: saved profile %s in %s\n", dr_progname, name, dir);
        r = 0;
    }
    profile_free(p);
    hw_free(&hw);
    return r;
}

static int
do_remove(Ctx *ctx, const char *name)
{
    const char *dir = ctx->system ? ctx->system_dir : ctx->user_dir;
    char *pdir, *cpath, *spath;
    DrProfile *p;
    int r = 1;

    if (!profile_name_ok(name))
        return dr_error("%s cannot be a profile name", name), 1;
    if (!dir)
        return dr_error("there is no home directory; use --system"), 1;
    pdir = xasprintf("%s/%s", dir, name);
    cpath = xasprintf("%s/config", pdir);
    spath = xasprintf("%s/setup", pdir);
    if (access(cpath, F_OK)) {
        scan_profiles(ctx);
        if ((p = profiles_find(&ctx->profiles, name)))
            dr_error("profile %s is in %s, not in %s", name, p->dir, dir);
        else
            dr_error("there is no profile %s", name);
        goto out;
    }
    if (ctx->dry_run) {
        printf("%s: would remove %s\n", dr_progname, pdir);
        r = 0;
        goto out;
    }
    if (unlink(cpath) || (unlink(spath) && errno != ENOENT)) {
        dr_error("cannot remove profile %s: %s", name, strerror(errno));
        goto out;
    }
    if (rmdir(pdir))
        dr_warn("left %s: %s", pdir, strerror(errno));
    printf("%s: removed profile %s from %s\n", dr_progname, name, dir);
    r = 0;
  out:
    free(pdir);
    free(cpath);
    free(spath);
    return r;
}

static int
do_list(Ctx *ctx, int cmd)
{
    DrHardware hw;
    DrProfile **m = NULL;
    int have_hw, n = 0, i, j;

    scan_profiles(ctx);
    have_hw = ctx->be && !query(ctx, &hw, 1);
    if (have_hw)
        n = matching(ctx, &hw, &m);
    if (cmd == CMD_DETECTED) {
        for (i = 0; i < n; i++)
            printf("%s\n", m[i]->name);
    } else
        for (i = 0; i < ctx->profiles.nprofile; i++) {
            DrProfile *p = ctx->profiles.profiles[i];
            int det = 0, cur = have_hw && profile_is_current(ctx, &hw, p);

            for (j = 0; j < n; j++)
                if (m[j] == p)
                    det = 1;
            if (cmd == CMD_CURRENT) {
                if (cur)
                    printf("%s\n", p->name);
                continue;
            }
            printf("%-20s %s%s%s\n", p->name, ctx->dirs[p->source],
                   det ? " (detected)" : "", cur ? " (current)" : "");
        }
    free(m);
    if (have_hw)
        hw_free(&hw);
    return (cmd != CMD_LIST && !have_hw) ? 1 : 0;
}

static int
do_show(Ctx *ctx, int cmd)
{
    DrHardware hw;

    if (query(ctx, &hw, 1))
        return 1;
    if (cmd == CMD_FINGERPRINT)
        print_fingerprint(&hw, stdout);
    else {
        DrProfile *p = profile_from_config(&hw, &hw.cur, "current");

        profile_write_config(p, stdout);
        profile_free(p);
    }
    hw_free(&hw);
    return 0;
}

/* ------------------------------------------------------------------ */
/* the daemon                                                           */

static volatile sig_atomic_t got_signal;
static int sig_pipe[2] = { -1, -1 };

static void
on_signal(int sig)
{
    int saved = errno;

    got_signal = sig;
    if (sig_pipe[1] >= 0 && write(sig_pipe[1], "", 1) < 0) {
        /* the pipe is full: a wakeup is pending anyway */
    }
    errno = saved;
}

static long
ms_since(const struct timespec *t)
{
    struct timespec now;

    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec - t->tv_sec) * 1000L
        + (now.tv_nsec - t->tv_nsec) / 1000000L;
}

/* Wait for more X events (or a signal); timeout in ms, -1 = forever.
   Returns -1 on a signal. */
static int
wait_events(Ctx *ctx, int timeout)
{
    struct pollfd fds[2];

    fds[0].fd = x_backend_fd(ctx->be);
    fds[0].events = POLLIN;
    fds[1].fd = sig_pipe[0];
    fds[1].events = POLLIN;
    if (got_signal)
        return -1;
    if (poll(fds, 2, timeout) < 0 && errno != EINTR)
        dr_warn("poll: %s", strerror(errno));
    return got_signal ? -1 : 0;
}

static int
do_daemon(Ctx *ctx)
{
    struct sigaction sa;
    char *fp = NULL;
    int r, i;

    if (ctx->fake)
        return dr_error("--daemon needs an X display"), 1;
    if ((r = x_backend_lock(ctx->be))) {
        if (r > 0) {
            dr_info("a dtrandr daemon runs on this display already");
            return 0;
        }
        return 1;
    }
    if (pipe(sig_pipe))
        return dr_error("pipe: %s", strerror(errno)), 1;
    for (i = 0; i < 2; i++) {
        fcntl(sig_pipe[i], F_SETFD, FD_CLOEXEC);
        fcntl(sig_pipe[i], F_SETFL, O_NONBLOCK);
    }
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);

    /* watch before the first change, not to miss what happens meanwhile */
    x_backend_watch(ctx->be);
    do_change(ctx, 1, &fp);
    dr_info("waiting for monitors to come and go");
    for (;;) {
        int n = x_backend_events(ctx->be);
        DrHardware hw;
        char *now;

        if (n == -2)
            break;
        if (n > 0 && !query(ctx, &hw, 0)) {
            now = fingerprint_string(&hw);
            hw_free(&hw);
            if (!fp || strcmp(now, fp)) {
                struct timespec first, last;

                dr_info("the connected monitors changed");
                /* debounce: wait until it is quiet for a moment */
                clock_gettime(CLOCK_MONOTONIC, &first);
                last = first;
                for (;;) {
                    long quiet = DEBOUNCE_MS - ms_since(&last);

                    if (quiet <= 0 || ms_since(&first) >= DEBOUNCE_MAX_MS)
                        break;
                    if (wait_events(ctx, (int) quiet))
                        break;
                    if ((n = x_backend_events(ctx->be)) == -2)
                        break;
                    if (n > 0)
                        clock_gettime(CLOCK_MONOTONIC, &last);
                }
                if (got_signal || n == -2) {
                    free(now);
                    break;
                }
                free(fp);
                fp = NULL;
                do_change(ctx, 0, &fp);
            }
            free(now);
        }
        if (wait_events(ctx, -1))
            break;
    }
    if (got_signal)
        dr_info("exiting on signal %d", (int) got_signal);
    free(fp);
    return 0;
}

/* ------------------------------------------------------------------ */

static void
ctx_free(Ctx *ctx)
{
    int i;

    if (ctx->be)
        ctx->be->close(ctx->be);
    profiles_free(&ctx->profiles);
    for (i = 0; i < ctx->ndir; i++)
        free(ctx->dirs[i]);
    free(ctx->dirs);
    free(ctx->user_dir);
    free(ctx->system_dir);
    free(ctx->hook_dir);
}

int
main(int argc, char **argv)
{
    enum { OPT_SYSTEM = 256, OPT_DEFAULT, OPT_LIST, OPT_DETECTED,
           OPT_CURRENT, OPT_FINGERPRINT, OPT_CONFIG, OPT_DEBUG,
           OPT_VERSION };
    static const struct option longopts[] = {
        { "change", no_argument, NULL, 'c' },
        { "load", required_argument, NULL, 'l' },
        { "save", required_argument, NULL, 's' },
        { "remove", required_argument, NULL, 'r' },
        { "system", no_argument, NULL, OPT_SYSTEM },
        { "force", no_argument, NULL, 'f' },
        { "default", required_argument, NULL, OPT_DEFAULT },
        { "list", no_argument, NULL, OPT_LIST },
        { "detected", no_argument, NULL, OPT_DETECTED },
        { "current", no_argument, NULL, OPT_CURRENT },
        { "fingerprint", no_argument, NULL, OPT_FINGERPRINT },
        { "config", no_argument, NULL, OPT_CONFIG },
        { "daemon", no_argument, NULL, 'd' },
        { "dry-run", no_argument, NULL, 'n' },
        { "verbose", no_argument, NULL, 'v' },
        { "debug", no_argument, NULL, OPT_DEBUG },
        { "help", no_argument, NULL, 'h' },
        { "version", no_argument, NULL, OPT_VERSION },
        { NULL, 0, NULL, 0 }
    };
    Ctx ctx;
    int c, cmd = CMD_NONE, ncmd = 0, r = 0;
    const char *arg = NULL, *fake;

    memset(&ctx, 0, sizeof ctx);
    while ((c = getopt_long(argc, argv, "cl:s:r:fdnvh", longopts,
                            NULL)) != -1) {
        int newcmd = CMD_NONE;

        switch (c) {
        case 'c': newcmd = CMD_CHANGE; break;
        case 'l': newcmd = CMD_LOAD; arg = optarg; break;
        case 's': newcmd = CMD_SAVE; arg = optarg; break;
        case 'r': newcmd = CMD_REMOVE; arg = optarg; break;
        case OPT_LIST: newcmd = CMD_LIST; break;
        case OPT_DETECTED: newcmd = CMD_DETECTED; break;
        case OPT_CURRENT: newcmd = CMD_CURRENT; break;
        case OPT_FINGERPRINT: newcmd = CMD_FINGERPRINT; break;
        case OPT_CONFIG: newcmd = CMD_CONFIG; break;
        case 'd': newcmd = CMD_DAEMON; break;
        case 'h': newcmd = CMD_HELP; break;
        case OPT_VERSION: newcmd = CMD_VERSION; break;
        case OPT_SYSTEM: ctx.system = 1; break;
        case 'f': ctx.force = 1; break;
        case OPT_DEFAULT: ctx.default_name = optarg; break;
        case 'n': ctx.dry_run = 1; break;
        case 'v': if (dr_verbose < 1) dr_verbose = 1; break;
        case OPT_DEBUG: dr_verbose = 2; break;
        default:
            usage(stderr);
            return 2;
        }
        if (newcmd != CMD_NONE && newcmd != cmd) {
            cmd = newcmd;
            ncmd++;
        }
    }
    if (optind < argc) {
        if (optind + 1 < argc || (cmd != CMD_NONE && cmd != CMD_LOAD)
            || arg) {
            usage(stderr);
            return 2;
        }
        cmd = CMD_LOAD;
        arg = argv[optind];
        ncmd++;
    }
    if (ncmd > 1) {
        fprintf(stderr, "%s: give one command at a time\n", dr_progname);
        return 2;
    }
    if (cmd == CMD_NONE)
        cmd = CMD_CHANGE;
    if (ctx.system && cmd != CMD_SAVE && cmd != CMD_REMOVE) {
        fprintf(stderr, "%s: --system goes with --save or --remove\n",
                dr_progname);
        return 2;
    }
    if (ctx.dry_run && cmd == CMD_DAEMON) {
        fprintf(stderr, "%s: --dry-run does not go with --daemon\n",
                dr_progname);
        return 2;
    }
    if (cmd == CMD_HELP) {
        usage(stdout);
        return 0;
    }
    if (cmd == CMD_VERSION) {
        printf("dtrandr (CDE) %s\n", DTRANDR_VERSION);
        return 0;
    }

    setup_dirs(&ctx);
    if (cmd != CMD_REMOVE) {
        fake = getenv("DTRANDR_FAKE");
        if (fake && *fake) {
            ctx.fake = 1;
            ctx.be = fake_backend_open(fake);
        } else
            ctx.be = x_backend_open(NULL);
        if (!ctx.be && cmd != CMD_LIST) {
            ctx_free(&ctx);
            return 1;
        }
    }

    switch (cmd) {
    case CMD_CHANGE:
        r = do_change(&ctx, 1, NULL);
        break;
    case CMD_LOAD:
        r = do_load(&ctx, arg);
        break;
    case CMD_SAVE:
        r = do_save(&ctx, arg);
        break;
    case CMD_REMOVE:
        r = do_remove(&ctx, arg);
        break;
    case CMD_LIST:
    case CMD_DETECTED:
    case CMD_CURRENT:
        r = do_list(&ctx, cmd);
        break;
    case CMD_FINGERPRINT:
    case CMD_CONFIG:
        r = do_show(&ctx, cmd);
        break;
    case CMD_DAEMON:
        r = do_daemon(&ctx);
        break;
    }
    ctx_free(&ctx);
    if (fflush(stdout) || ferror(stdout))
        r = 1;
    return r;
}
