/*
 * CDE benchmarks, help: the counters of libhelpbench_preload.so.
 *
 * Licensed under the LGPL 2.1 license.
 */
#ifndef HELPBENCH_H
#define HELPBENCH_H

struct helpbench_xcounts {
	unsigned long alloc_color;		/* XAllocColor */
	unsigned long alloc_named_color;	/* XAllocNamedColor */
	unsigned long lookup_color;		/* XLookupColor */
	unsigned long parse_color;		/* XParseColor */
	unsigned long load_query_font;		/* XLoadQueryFont */
	unsigned long load_font;		/* XLoadFont */
	unsigned long query_font;		/* XQueryFont */
	unsigned long create_font_set;		/* XCreateFontSet */
	unsigned long list_fonts;		/* XListFonts */
	unsigned long put_image;		/* XPutImage */
	unsigned long create_pixmap;		/* XCreatePixmap */
	unsigned long create_gc;		/* XCreateGC */
};

#define HELPBENCH_NCOUNTS \
	(sizeof(struct helpbench_xcounts) / sizeof(unsigned long))

#endif /* HELPBENCH_H */
