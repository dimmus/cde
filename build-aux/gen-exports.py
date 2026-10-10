#!/usr/bin/env python3
#
# gen-exports.py - regenerate the export lists of the CDE libraries
#
# Each CDE shared library exports only the symbols listed in
# build-aux/symbols/lib<name>.sym (libtool -export-symbols, which GNU ld
# turns into an unversioned version script, so the symbols that stay
# exported keep their ABI).  Hiding the rest takes their symbolic
# relocations and PLT calls out of every process start and lets the
# linker bind the library's internal calls directly.
#
# A symbol is listed when the library defines it and
#   - it is named anywhere in an installed header (include/Makefile.am,
#     nobase_include_HEADERS): the public API;
#   - an ELF file of the built tree other than the library itself
#     (programs, the other libraries, tools/bench) references it, defines
#     it too (the dynamic linker unifies the two copies), or copies it
#     (R_*_COPY);
#   - it is a C symbol named in a tracked source file outside the
#     library's own directory, which covers code that this build does not
#     compile (other systems, other configure options);
#   - it is a widget class record (dtksh's DtLoadWidget looks those up by
#     name);
#   - for the C++ libraries (libtt, libDtMmdb), it is a member, vtable or
#     typeinfo of a class one of whose members is listed for the reasons
#     above, so that code this build does not compile can use the rest of
#     the class.
#
# Usage, from the top of a fully built tree configured with
# --disable-export-maps (otherwise the libraries already export only the
# old lists, and a symbol can only ever be dropped):
#
#     ./configure --disable-export-maps ... && make
#     python3 build-aux/gen-exports.py
#     git diff build-aux/symbols
#
# A new in-tree use of a symbol that the list hides fails to link
# ("undefined reference"); add the symbol to the list by hand, or
# regenerate the lists as above.

import glob
import os
import re
import subprocess
import sys

TOP = os.path.realpath(os.path.join(os.path.dirname(sys.argv[0]), '..')) + '/'
OUT = TOP + 'build-aux/symbols/'

# name: (directory of the library, C++?)
LIBS = {
    'tt': ('lib/tt/lib/', True),
    'DtXinerama': ('lib/DtXinerama/', False),
    'DtSvc': ('lib/DtSvc/', False),
    'DtSearch': ('lib/DtSearch/', False),
    'DtWidget': ('lib/DtWidget/', False),
    'DtHelp': ('lib/DtHelp/', False),
    'DtPrint': ('lib/DtPrint/', False),
    'DtTerm': ('lib/DtTerm/', False),
    'DtMrm': ('lib/DtMrm/', False),
    'csa': ('lib/csa/', False),
    'DtMmdb': ('lib/DtMmdb/', True),
}

IDENT = re.compile(r'[A-Za-z_][A-Za-z_0-9]*')
WIDGET_CLASS = re.compile(r'(WidgetClass|GadgetClass|ClassRec)$')


def run(cmd, **kw):
    return subprocess.run(cmd, capture_output=True, text=True, check=False,
                          **kw).stdout


def dynsyms(path):
    """(defined, undefined) dynamic symbols of an ELF file"""
    defined, undefined = set(), set()
    for line in run(['nm', '-D', path]).splitlines():
        f = line.split()
        if len(f) == 3 and f[1] in 'TDBRVWuiG':
            defined.add(f[2].split('@')[0])
        elif len(f) == 2 and f[0] in 'Uwv':
            undefined.add(f[1].split('@')[0])
        elif len(f) == 3 and f[1] in 'wv':
            undefined.add(f[2].split('@')[0])
    for line in run(['readelf', '-rW', path]).splitlines():
        if '_COPY ' in line:
            undefined.add(line.split()[4].split('@')[0])
    return defined, undefined


def elf_files():
    for root, dirs, files in os.walk(TOP):
        dirs[:] = [d for d in dirs if d not in ('.git', 'ksh93')]
        for name in files:
            path = os.path.join(root, name)
            if os.path.islink(path) or name.endswith('T'):
                continue
            try:
                with open(path, 'rb') as fh:
                    head = fh.read(18)
            except OSError:
                continue
            # ET_EXEC or ET_DYN
            if head[:4] == b'\x7fELF' and len(head) == 18 and head[16] in (2, 3):
                yield os.path.realpath(path)


def class_of(demangled):
    """the class a demangled C++ symbol belongs to, or None"""
    special = ('typeinfo name for ', 'typeinfo for ', 'vtable for ',
               'VTT for ', 'construction vtable for ')
    name = demangled
    is_class_data = False
    for prefix in special + ('guard variable for ', 'non-virtual thunk to ',
                             'virtual thunk to '):
        if name.startswith(prefix):
            name = name[len(prefix):]
            is_class_data = prefix in special
            break
    depth, base = 0, ''
    for ch in name:
        if ch in '<(':
            depth += 1
        elif ch in '>)':
            depth -= 1
        elif depth == 0:
            base += ch
    base = base.strip()
    if is_class_data:
        return base
    return base.rsplit('::', 1)[0] if '::' in base else None


def main():
    headers = re.findall(
        r'[A-Za-z]+/[A-Za-z_0-9]+\.h',
        run(['sed', '-n', '/nobase_include_HEADERS/,/^$/p',
             TOP + 'include/Makefile.am']))
    public = set()
    for h in headers:
        with open(TOP + 'include/' + h, errors='replace') as fh:
            public |= set(IDENT.findall(fh.read()))

    consumers = {path: d | u for path in set(elf_files())
                 for d, u in [dynsyms(path)]}

    sources = run(['git', 'ls-files', '--', '*.c', '*.C', '*.cc', '*.cxx',
                   '*.h', '*.hh', '*.y', '*.l', ':!:*/ksh93/*'],
                  cwd=TOP).split()
    tokens = {}
    for src in sources:
        try:
            with open(TOP + src, errors='replace') as fh:
                tokens[src] = set(IDENT.findall(fh.read()))
        except OSError:
            pass

    os.makedirs(OUT, exist_ok=True)
    for name, (libdir, cxx) in LIBS.items():
        sonames = [p for p in glob.glob(TOP + libdir + '.libs/lib%s.so.*.*' % name)
                   if not p.endswith('T')]
        if len(sonames) != 1:
            sys.exit('gen-exports: %s: build the tree first' % name)
        exported = dynsyms(sonames[0])[0]

        used = set()
        for path, syms in consumers.items():
            if not path.startswith(TOP + libdir):
                used |= syms
        named = set()
        for src, toks in tokens.items():
            if not src.startswith(libdir) and not src.startswith('include/'):
                named |= toks

        keep = exported & (public | used | named)
        keep |= {s for s in exported if WIDGET_CLASS.search(s)}

        if cxx:
            names = sorted(exported)
            demangled = dict(zip(names, run(['c++filt'],
                                            input='\n'.join(names)).split('\n')))
            classes = {class_of(demangled[s]) for s in keep} - {None}
            keep |= {s for s in exported if class_of(demangled[s]) in classes}

        with open(OUT + 'lib%s.sym' % name, 'w') as fh:
            for sym in sorted(keep):
                fh.write(sym + '\n')
        print('lib%-11s %5d of %5d symbols exported' % (name, len(keep),
                                                         len(exported)))


if __name__ == '__main__':
    main()
