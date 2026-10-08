/*
 * CDE - Common Desktop Environment
 *
 * (c) Copyright 1993-2012 The Open Group
 * (c) Copyright 2012-2022 CDE Project contributors, see
 * CONTRIBUTORS for details
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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define IFACE_FAL_SIZE 7
#define APP_FAL_SIZE 12

struct xlfd_t {
  char *fndry;
  char *fmly;
  char *wght;
  char *slant;
  char *swdth;
  char *adstyl;
  char *pxlsz;
  char *ptsz;
  char *resx;
  char *resy;
  char *spc;
  char *avgwdth;
  char *rgstry;
  char *encdng;
};

struct font_alias_t {
  struct xlfd_t *font, *alias;
};

struct xlfd_t iface_font_xxs;
struct xlfd_t iface_font_xs;
struct xlfd_t iface_font_s;
struct xlfd_t iface_font_m;
struct xlfd_t iface_font_l;
struct xlfd_t iface_font_xl;
struct xlfd_t iface_font_xxl;

struct xlfd_t iface_alias_xxs;
struct xlfd_t iface_alias_xs;
struct xlfd_t iface_alias_s;
struct xlfd_t iface_alias_m;
struct xlfd_t iface_alias_l;
struct xlfd_t iface_alias_xl;
struct xlfd_t iface_alias_xxl;

struct font_alias_t iface_fal[IFACE_FAL_SIZE] = {
  {&iface_font_xxs, &iface_alias_xxs},
  {&iface_font_xs,  &iface_alias_xs},
  {&iface_font_s,   &iface_alias_s},
  {&iface_font_m,   &iface_alias_m},
  {&iface_font_l,   &iface_alias_l},
  {&iface_font_xl,  &iface_alias_xl},
  {&iface_font_xxl, &iface_alias_xxl}
};

enum app_adstyl_spc_t {SANS_P, SERIF_M, SERIF_P};
struct xlfd_t app_font[APP_FAL_SIZE], app_alias[APP_FAL_SIZE];
struct font_alias_t app_fal[APP_FAL_SIZE];

static void print_xlfd(struct xlfd_t *pxlfd) {
  fprintf(stdout, "\"-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s-%s\"",
      pxlfd->fndry,
      pxlfd->fmly,
      pxlfd->wght,
      pxlfd->slant,
      pxlfd->swdth,
      pxlfd->adstyl,
      pxlfd->pxlsz,
      pxlfd->ptsz,
      pxlfd->resx,
      pxlfd->resy,
      pxlfd->spc,
      pxlfd->avgwdth,
      pxlfd->rgstry,
      pxlfd->encdng);
}

static void print_font_alias(struct font_alias_t *pfa) {
  print_xlfd(pfa->alias); fprintf(stdout, " ");
  print_xlfd(pfa->font);  fprintf(stdout, "\n");
}

static void iface_iso8859(bool is_user, bool is_bold) {
  for (int i = 0; i < IFACE_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &iface_fal[i];

    memset(pfa->font, 0, sizeof(struct xlfd_t));
    memset(pfa->alias, 0, sizeof(struct xlfd_t));

    pfa->font->fndry  = "misc";
    pfa->font->fmly   = "fixed";
    pfa->font->wght   = "medium";
    pfa->font->slant  = "r";
    pfa->font->swdth  = "normal";
    pfa->font->adstyl = "";
    pfa->font->resx   =
    pfa->font->resy   = "75";
    pfa->font->spc    = "c";
    pfa->font->rgstry = "iso8859";

    pfa->alias->fndry = "dt";
    pfa->alias->fmly  = is_user ? "interface user" : "interface system";
    pfa->alias->wght  = is_bold ? "bold" : "medium";
    pfa->alias->spc   = "m";
  }

  if (is_bold) {
    iface_font_s.wght  =
    iface_font_m.wght  =
    iface_font_l.wght  =
    iface_font_xl.wght = "bold";
  }

  iface_alias_xxs.adstyl = "xxs sans";
  iface_alias_xs.adstyl  = "xs sans";
  iface_alias_s.adstyl   = "s sans";
  iface_alias_m.adstyl   = "m sans";
  iface_alias_l.adstyl   = "l sans";
  iface_alias_xl.adstyl  = "xl sans";
  iface_alias_xxl.adstyl = "xxl sans";

  iface_font_xxs.pxlsz   = "9";
  iface_font_xxs.ptsz    = "90";
  iface_font_xxs.avgwdth = "60";

  iface_font_xs.pxlsz   = "10";
  iface_font_xs.ptsz    = "100";
  iface_font_xs.avgwdth = "60";

  iface_font_s.pxlsz   = "13";
  iface_font_s.ptsz    = "120";
  iface_font_s.avgwdth = "70";

  iface_font_m.pxlsz   = "14";
  iface_font_m.ptsz    = "130";
  iface_font_m.avgwdth = "70";

  iface_font_l.pxlsz   = "15";
  iface_font_l.ptsz    = "140";
  iface_font_l.avgwdth = "90";

  iface_font_xl.pxlsz   = "18";
  iface_font_xl.ptsz    = "120";
  iface_font_xl.resx    = "100";
  iface_font_xl.resy    = "100";
  iface_font_xl.avgwdth = "90";

  iface_font_xxl.pxlsz   = "20";
  iface_font_xxl.ptsz    = "200";
  iface_font_xxl.avgwdth = "100";

  for (int i = 0; i < IFACE_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &iface_fal[i];

    pfa->alias->slant   = pfa->font->slant;
    pfa->alias->swdth   = pfa->font->swdth;
    pfa->alias->pxlsz   = pfa->font->pxlsz;
    pfa->alias->ptsz    = pfa->font->ptsz;
    pfa->alias->resx    = pfa->font->resx;
    pfa->alias->resy    = pfa->font->resy;
    pfa->alias->avgwdth = pfa->font->avgwdth;
    pfa->alias->rgstry  = pfa->font->rgstry;
  }
}

static void iface_iso8859_11(bool is_user, bool is_bold) {
  iface_iso8859(is_user, is_bold);

  iface_font_xxs.pxlsz =
  iface_font_xs.pxlsz  = iface_font_s.pxlsz;

  iface_font_xxs.ptsz =
  iface_font_xs.ptsz  = iface_font_s.ptsz;

  iface_font_xxs.avgwdth =
  iface_font_xs.avgwdth  = iface_font_s.avgwdth;

  iface_font_xl.wght = "medium";
}

static void iface_koi8_r(bool is_user, bool is_bold) {
  iface_iso8859(is_user, is_bold);

  for (int i = 0; i < IFACE_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &iface_fal[i];

    pfa->font->wght   = "medium";
    pfa->font->rgstry = "koi8";
    pfa->font->encdng = "r";

    pfa->alias->rgstry = pfa->font->rgstry;
    pfa->alias->encdng = pfa->font->encdng;
  }
}

static void iface_jisx0201_1976_0(bool is_user, bool is_bold) {
  iface_iso8859(is_user, is_bold);

  for (int i = 0; i < IFACE_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &iface_fal[i];

    pfa->font->wght    = "medium";
    pfa->font->pxlsz   = "14";
    pfa->font->ptsz    = "130";
    pfa->font->resx    =
    pfa->font->resy    = "75";
    pfa->font->avgwdth = "70";
    pfa->font->rgstry  = "jisx0201.1976";
    pfa->font->encdng  = "0";

    pfa->alias->pxlsz   = pfa->font->pxlsz;
    pfa->alias->ptsz    = pfa->font->ptsz;
    pfa->alias->resx    = pfa->font->resx;
    pfa->alias->resy    = pfa->font->resy;
    pfa->alias->avgwdth = pfa->font->avgwdth;
    pfa->alias->rgstry  = pfa->font->rgstry;
    pfa->alias->encdng  = pfa->font->encdng;
  }
}

static void iface_jisx0208_1983_0(bool is_user, bool is_bold) {
  iface_jisx0201_1976_0(is_user, is_bold);

  for (int i = 0; i < IFACE_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &iface_fal[i];

    pfa->font->fndry   = "jis";
    pfa->font->pxlsz   = "16";
    pfa->font->ptsz    = "150";
    pfa->font->avgwdth = "160";
    pfa->font->rgstry  = "jisx0208.1983";

    pfa->alias->rgstry = pfa->font->rgstry;
  }

  iface_font_xxs.fndry =
  iface_font_xs.fndry  =
  iface_font_s.fndry   = "misc";

  iface_font_xxs.pxlsz =
  iface_font_xs.pxlsz  =
  iface_font_s.pxlsz   = "14";

  iface_font_xxs.ptsz =
  iface_font_xs.ptsz  =
  iface_font_s.ptsz   = "130";

  iface_font_xxs.avgwdth =
  iface_font_xs.avgwdth  =
  iface_font_s.avgwdth   = "140";

  iface_font_xxl.pxlsz   = "24";
  iface_font_xxl.ptsz    = "230";
  iface_font_xxl.avgwdth = "240";
}

static void iface_ksc5601_1987_0(bool is_user, bool is_bold) {
  iface_iso8859(is_user, is_bold);

  for (int i = 0; i < IFACE_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &iface_fal[i];

    pfa->font->fndry   = "daewoo";
    pfa->font->fmly    = is_bold ? "mincho" : "gothic";
    pfa->font->wght    = "medium";
    pfa->font->pxlsz   = "16";
    pfa->font->ptsz    = "120";
    pfa->font->resx    =
    pfa->font->resy    = "100";
    pfa->font->avgwdth = "160";
    pfa->font->rgstry  = "ksc5601.1987";
    pfa->font->encdng  = "0";

    pfa->alias->rgstry = pfa->font->rgstry;
    pfa->alias->encdng = pfa->font->encdng;
  }

  iface_font_xxl.fmly    = "mincho";
  iface_font_xxl.pxlsz   = "24";
  iface_font_xxl.ptsz    = "170";
  iface_font_xxl.avgwdth = "240";
}

static void iface_gb2312_1980_0(bool is_user, bool is_bold) {
  iface_iso8859(is_user, is_bold);

  for (int i = 0; i < IFACE_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &iface_fal[i];

    pfa->font->fndry   = "isas";
    pfa->font->fmly    = is_bold ? "song ti" : "fangsong ti";
    pfa->font->wght    = "medium";
    pfa->font->pxlsz   = "16";
    pfa->font->ptsz    = "160";
    pfa->font->resx    =
    pfa->font->resy    = "72";
    pfa->font->avgwdth = "160";
    pfa->font->rgstry  = "gb2312.1980";
    pfa->font->encdng  = "0";

    pfa->alias->rgstry = pfa->font->rgstry;
    pfa->alias->encdng = pfa->font->encdng;
  }

  iface_font_xxl.fmly    = "song ti";
  iface_font_xxl.pxlsz   = "24";
  iface_font_xxl.ptsz    = "240";
  iface_font_xxl.avgwdth = "240";
}

static void print_iface(void (*fiface)(bool, bool), char *encdng) {
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 2; ++j) {
      fiface(i, j);

      for (int k = 0; k < IFACE_FAL_SIZE; ++k) {
        struct font_alias_t *pfa = &iface_fal[k];
        pfa->alias->encdng = pfa->font->encdng = encdng;
        print_font_alias(pfa);
      }
    }
  }
}

static void print_iface_iso8859(char *encdng) {
  print_iface(iface_iso8859, encdng);
}

static void print_iface_iso8859_11(void) {
  print_iface(iface_iso8859_11, "11");
}

static void print_iface_koi8_r(void) {
  print_iface(iface_koi8_r, "r");
}

static void print_iface_jisx0201_1976_0(void) {
  print_iface(iface_jisx0201_1976_0, "0");
}

static void print_iface_jisx0208_1983_0(void) {
  print_iface(iface_jisx0208_1983_0, "0");
}

static void print_iface_ksc5601_1987_0(void) {
  print_iface(iface_ksc5601_1987_0, "0");
}

static void print_iface_gb2312_1980_0(void) {
  print_iface(iface_gb2312_1980_0, "0");
}

static void app_iso8859_full(enum app_adstyl_spc_t adstyl_spc, bool is_bold,
    bool is_italic)
{
  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    memset(pfa->font, 0, sizeof(struct xlfd_t));
    memset(pfa->alias, 0, sizeof(struct xlfd_t));

    pfa->font->swdth  = "normal";
    pfa->font->adstyl = "";
    pfa->font->resx   =
    pfa->font->resy   = i % 2 ? "100" : "75";
    pfa->font->rgstry = "iso8859";

    pfa->alias->fndry = "dt";
    pfa->alias->fmly  = "application";
  }

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->fndry = "adobe";
    pfa->font->fmly  = "courier";
    pfa->font->wght  = is_bold ? "bold" : "medium";
    pfa->font->slant = is_italic ? "o" : "r";
    pfa->font->spc   = "m";

    pfa->alias->slant  = is_italic ? "i" : "r";
    pfa->alias->adstyl = "serif";

    switch (i) {
      case 0:
        pfa->font->pxlsz   = "8";
        pfa->font->ptsz    = "80";
        pfa->font->avgwdth = "50";
        break;

      case 2:
        pfa->font->pxlsz   = "10";
        pfa->font->ptsz    = "100";
        pfa->font->avgwdth = "60";
        break;

      case 4:
        pfa->font->pxlsz   = "12";
        pfa->font->ptsz    = "120";
        pfa->font->avgwdth = "70";
        break;

      case 6:
        pfa->font->pxlsz   = "14";
        pfa->font->ptsz    = "140";
        pfa->font->avgwdth = "90";
        break;

      case 8:
        pfa->font->pxlsz   = "18";
        pfa->font->ptsz    = "180";
        pfa->font->avgwdth = "110";
        break;

      case 10:
        pfa->font->pxlsz   = "24";
        pfa->font->ptsz    = "240";
        pfa->font->avgwdth = "150";
        break;

      case 1:
        pfa->font->pxlsz   = "11";
        pfa->font->ptsz    = "80";
        pfa->font->avgwdth = "60";
        break;

      case 3:
        pfa->font->pxlsz   = "14";
        pfa->font->ptsz    = "100";
        pfa->font->avgwdth = "90";
        break;

      case 5:
        pfa->font->pxlsz   = "17";
        pfa->font->ptsz    = "120";
        pfa->font->avgwdth = "100";
        break;

      case 7:
        pfa->font->pxlsz   = "20";
        pfa->font->ptsz    = "140";
        pfa->font->avgwdth = "110";
        break;

      case 9:
        pfa->font->pxlsz   = "25";
        pfa->font->ptsz    = "180";
        pfa->font->avgwdth = "150";
        break;

      case 11:
        pfa->font->pxlsz   = "34";
        pfa->font->ptsz    = "240";
        pfa->font->avgwdth = "200";
        break;
    }
  }

  if (adstyl_spc == SERIF_M) goto done;

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->fndry = "adobe";
    pfa->font->fmly  = "helvetica";
    pfa->font->wght  = "medium";
    pfa->font->slant = "r";
    pfa->font->spc   = "p";

    pfa->alias->slant  = pfa->font->slant;
    pfa->alias->adstyl = "sans";

    switch (i) {
      case 0:
        pfa->font->pxlsz   = "8";
        pfa->font->ptsz    = "80";
        pfa->font->avgwdth = "46";
        break;

      case 2:
        pfa->font->pxlsz   = "10";
        pfa->font->ptsz    = "100";
        pfa->font->avgwdth = "56";
        break;

      case 4:
        pfa->font->pxlsz   = "12";
        pfa->font->ptsz    = "120";
        pfa->font->avgwdth = "67";
        break;

      case 6:
        pfa->font->pxlsz   = "14";
        pfa->font->ptsz    = "140";
        pfa->font->avgwdth = "77";
        break;

      case 8:
        pfa->font->pxlsz   = "18";
        pfa->font->ptsz    = "180";
        pfa->font->avgwdth = "98";
        break;

      case 10:
        pfa->font->pxlsz   = "24";
        pfa->font->ptsz    = "240";
        pfa->font->avgwdth = "130";
        break;

      case 1:
        pfa->font->pxlsz   = "11";
        pfa->font->ptsz    = "80";
        pfa->font->avgwdth = "56";
        break;

      case 3:
        pfa->font->pxlsz   = "14";
        pfa->font->ptsz    = "100";
        pfa->font->avgwdth = "76";
        break;

      case 5:
        pfa->font->pxlsz   = "17";
        pfa->font->ptsz    = "120";
        pfa->font->avgwdth = "88";
        break;

      case 7:
        pfa->font->pxlsz   = "20";
        pfa->font->ptsz    = "140";
        pfa->font->avgwdth = "100";
        break;

      case 9:
        pfa->font->pxlsz   = "25";
        pfa->font->ptsz    = "180";
        pfa->font->avgwdth = "130";
        break;

      case 11:
        pfa->font->pxlsz   = "34";
        pfa->font->ptsz    = "240";
        pfa->font->avgwdth = "176";
        break;
    }
  }

  if (adstyl_spc == SANS_P && !is_bold && !is_italic) goto done;

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->slant = "o";

    pfa->alias->slant = "i";

    switch (i) {
      case  0: pfa->font->avgwdth = "47";  break;
      case  2: pfa->font->avgwdth = "57";  break;
      case  4: pfa->font->avgwdth = "67";  break;
      case  6: pfa->font->avgwdth = "78";  break;
      case  8: pfa->font->avgwdth = "98";  break;
      case 10: pfa->font->avgwdth = "130"; break;

      case  1: pfa->font->avgwdth = "57";  break;
      case  3: pfa->font->avgwdth = "78";  break;
      case  5: pfa->font->avgwdth = "88";  break;
      case  7: pfa->font->avgwdth = "98";  break;
      case  9: pfa->font->avgwdth = "130"; break;
      case 11: pfa->font->avgwdth = "176"; break;
    }
  }

  if (adstyl_spc == SANS_P && !is_bold && is_italic) goto done;

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->wght  = "bold";
    pfa->font->slant = "r";

    pfa->alias->slant = pfa->font->slant;

    switch (i) {
      case  0: pfa->font->avgwdth = "50";  break;
      case  2: pfa->font->avgwdth = "60";  break;
      case  4: pfa->font->avgwdth = "70";  break;
      case  6: pfa->font->avgwdth = "82";  break;
      case  8: pfa->font->avgwdth = "103"; break;
      case 10: pfa->font->avgwdth = "138"; break;

      case  1: pfa->font->avgwdth = "60";  break;
      case  3: pfa->font->avgwdth = "82";  break;
      case  5: pfa->font->avgwdth = "92";  break;
      case  7: pfa->font->avgwdth = "105"; break;
      case  9: pfa->font->avgwdth = "138"; break;
      case 11: pfa->font->avgwdth = "182"; break;
    }
  }

  if (adstyl_spc == SANS_P && is_bold && !is_italic) goto done;

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->slant = "o";

    pfa->alias->slant = "i";

    switch (i) {
      case  0: pfa->font->avgwdth = "50";  break;
      case  2: pfa->font->avgwdth = "60";  break;
      case  4: pfa->font->avgwdth = "69";  break;
      case  6: pfa->font->avgwdth = "82";  break;
      case  8: pfa->font->avgwdth = "104"; break;
      case 10: pfa->font->avgwdth = "138"; break;

      case  1: pfa->font->avgwdth = "60";  break;
      case  3: pfa->font->avgwdth = "82";  break;
      case  5: pfa->font->avgwdth = "92";  break;
      case  7: pfa->font->avgwdth = "103"; break;
      case  9: pfa->font->avgwdth = "138"; break;
      case 11: pfa->font->avgwdth = "182"; break;
    }
  }

  if (adstyl_spc == SANS_P && is_bold && is_italic) goto done;

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->fndry = "adobe";
    pfa->font->fmly  = "times";
    pfa->font->wght  = "medium";
    pfa->font->slant = "r";
    pfa->font->spc   = "p";

    pfa->alias->slant  = pfa->font->slant;
    pfa->alias->adstyl = "serif";

    switch (i) {
      case 0:
        pfa->font->pxlsz   = "8";
        pfa->font->ptsz    = "80";
        pfa->font->avgwdth = "44";
        break;

      case 2:
        pfa->font->pxlsz   = "10";
        pfa->font->ptsz    = "100";
        pfa->font->avgwdth = "54";
        break;

      case 4:
        pfa->font->pxlsz   = "12";
        pfa->font->ptsz    = "120";
        pfa->font->avgwdth = "64";
        break;

      case 6:
        pfa->font->pxlsz   = "14";
        pfa->font->ptsz    = "140";
        pfa->font->avgwdth = "74";
        break;

      case 8:
        pfa->font->pxlsz   = "18";
        pfa->font->ptsz    = "180";
        pfa->font->avgwdth = "94";
        break;

      case 10:
        pfa->font->pxlsz   = "24";
        pfa->font->ptsz    = "240";
        pfa->font->avgwdth = "124";
        break;

      case 1:
        pfa->font->pxlsz   = "11";
        pfa->font->ptsz    = "80";
        pfa->font->avgwdth = "54";
        break;

      case 3:
        pfa->font->pxlsz   = "14";
        pfa->font->ptsz    = "100";
        pfa->font->avgwdth = "74";
        break;

      case 5:
        pfa->font->pxlsz   = "17";
        pfa->font->ptsz    = "120";
        pfa->font->avgwdth = "84";
        break;

      case 7:
        pfa->font->pxlsz   = "20";
        pfa->font->ptsz    = "140";
        pfa->font->avgwdth = "96";
        break;

      case 9:
        pfa->font->pxlsz   = "25";
        pfa->font->ptsz    = "180";
        pfa->font->avgwdth = "125";
        break;

      case 11:
        pfa->font->pxlsz   = "34";
        pfa->font->ptsz    = "240";
        pfa->font->avgwdth = "170";
        break;
    }
  }

  if (adstyl_spc == SERIF_P && !is_bold && !is_italic) goto done;

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->slant = "i";

    pfa->alias->slant = pfa->font->slant;

    switch (i) {
      case  0: pfa->font->avgwdth = "42";  break;
      case  2: pfa->font->avgwdth = "52";  break;
      case  4: pfa->font->avgwdth = "63";  break;
      case  6: pfa->font->avgwdth = "73";  break;
      case  8: pfa->font->avgwdth = "94";  break;
      case 10: pfa->font->avgwdth = "125"; break;

      case  1: pfa->font->avgwdth = "52";  break;
      case  3: pfa->font->avgwdth = "73";  break;
      case  5: pfa->font->avgwdth = "84";  break;
      case  7: pfa->font->avgwdth = "94";  break;
      case  9: pfa->font->avgwdth = "125"; break;
      case 11: pfa->font->avgwdth = "168"; break;
    }
  }

  if (adstyl_spc == SERIF_P && !is_bold && is_italic) goto done;

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->wght  = "bold";
    pfa->font->slant = "r";

    pfa->alias->slant = pfa->font->slant;

    switch (i) {
      case  0: pfa->font->avgwdth = "47";  break;
      case  2: pfa->font->avgwdth = "57";  break;
      case  4: pfa->font->avgwdth = "67";  break;
      case  6: pfa->font->avgwdth = "77";  break;
      case  8: pfa->font->avgwdth = "99";  break;
      case 10: pfa->font->avgwdth = "132"; break;

      case  1: pfa->font->avgwdth = "57";  break;
      case  3: pfa->font->avgwdth = "76";  break;
      case  5: pfa->font->avgwdth = "88";  break;
      case  7: pfa->font->avgwdth = "100"; break;
      case  9: pfa->font->avgwdth = "132"; break;
      case 11: pfa->font->avgwdth = "177"; break;
    }
  }

  if (adstyl_spc == SERIF_P && is_bold && !is_italic) goto done;

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->slant = "i";

    pfa->alias->slant = pfa->font->slant;

    switch (i) {
      case  0: pfa->font->avgwdth = "47";  break;
      case  2: pfa->font->avgwdth = "57";  break;
      case  4: pfa->font->avgwdth = "68";  break;
      case  6: pfa->font->avgwdth = "77";  break;
      case  8: pfa->font->avgwdth = "98";  break;
      case 10: pfa->font->avgwdth = "128"; break;

      case  1: pfa->font->avgwdth = "57";  break;
      case  3: pfa->font->avgwdth = "77";  break;
      case  5: pfa->font->avgwdth = "86";  break;
      case  7: pfa->font->avgwdth = "98";  break;
      case  9: pfa->font->avgwdth = "128"; break;
      case 11: pfa->font->avgwdth = "170"; break;
    }
  }

  if (adstyl_spc == SERIF_P && is_bold && is_italic) goto done;

done:
  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->alias->wght    = pfa->font->wght;
    pfa->alias->swdth   = pfa->font->swdth;
    pfa->alias->pxlsz   = pfa->font->pxlsz;
    pfa->alias->ptsz    = pfa->font->ptsz;
    pfa->alias->resx    = pfa->font->resx;
    pfa->alias->resy    = pfa->font->resy;
    pfa->alias->spc     = pfa->font->spc;
    pfa->alias->avgwdth = pfa->font->avgwdth;
    pfa->alias->rgstry  = pfa->font->rgstry;
    pfa->alias->encdng  = pfa->font->encdng;
  }
}

static void app_iso8859_partial(enum app_adstyl_spc_t adstyl_spc, bool is_bold,
    bool is_italic)
{
  app_iso8859_full(adstyl_spc, is_bold, is_italic);

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->fndry = "misc";
    pfa->font->fmly  = "fixed";
    pfa->font->wght  = "medium";
    pfa->font->slant = "r";
    pfa->font->resx  =
    pfa->font->resy  = "75";
    pfa->font->spc   = "c";

    switch (i) {
      case 0:
        pfa->font->pxlsz   = "8";
        pfa->font->ptsz    = "80";
        pfa->font->avgwdth = "50";
        break;

      case 1:
      case 2:
        pfa->font->pxlsz   = "10";
        pfa->font->ptsz    = "100";
        pfa->font->avgwdth = "60";
        break;

      case 3:
      case 6:
        pfa->font->wght    = is_bold ? "bold" : "medium";
        pfa->font->pxlsz   = "14";
        pfa->font->ptsz    = "130";
        pfa->font->avgwdth = "70";
        break;

      case 4:
        pfa->font->wght    = is_bold ? "bold" : "medium";
        pfa->font->pxlsz   = "13";
        pfa->font->ptsz    = "120";
        pfa->font->avgwdth = "70";
        break;

      case 5:
      case 8:
        pfa->font->wght    = is_bold ? "bold" : "medium";
        pfa->font->pxlsz   = "18";
        pfa->font->ptsz    = "120";
        pfa->font->resx    = "100";
        pfa->font->resy    = pfa->font->resx;
        pfa->font->avgwdth = "90";
        break;

      case 7:
      case 9:
      case 10:
      case 11:
        pfa->font->pxlsz   = "20";
        pfa->font->ptsz    = "200";
        pfa->font->avgwdth = "100";
        break;
    }
  }
}

static void app_iso8859_11(enum app_adstyl_spc_t adstyl_spc, bool is_bold,
    bool is_italic)
{
  app_iso8859_partial(adstyl_spc, is_bold, is_italic);

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];
    struct font_alias_t *p;

    pfa->font->encdng = "11";

    pfa->alias->encdng = pfa->font->encdng;

    switch (i) {
      case 0:
      case 1:
      case 2:
        p = &app_fal[4];

        pfa->font->wght    = p->font->wght;
        pfa->font->pxlsz   = p->font->pxlsz;
        pfa->font->ptsz    = p->font->ptsz;
        pfa->font->avgwdth = p->font->avgwdth;

        break;

      case 5:
      case 8:
        pfa->font->wght = "medium";
        break;
    }
  }
}

static void app_koi8_r(enum app_adstyl_spc_t adstyl_spc, bool is_bold,
    bool is_italic)
{
  app_iso8859_partial(adstyl_spc, is_bold, is_italic);

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->wght   = "medium";
    pfa->font->rgstry = "koi8";
    pfa->font->encdng = "r";

    pfa->alias->rgstry = pfa->font->rgstry;
    pfa->alias->encdng = pfa->font->encdng;
  }
}

static void app_jisx0201_1976_0(enum app_adstyl_spc_t adstyl_spc, bool is_bold,
    bool is_italic)
{
  app_iso8859_partial(adstyl_spc, is_bold, is_italic);

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->wght    = "medium";
    pfa->font->pxlsz   = "14";
    pfa->font->ptsz    = "130";
    pfa->font->resx    =
    pfa->font->resy    = "75";
    pfa->font->avgwdth = "70";
    pfa->font->rgstry  = "jisx0201.1976";
    pfa->font->encdng  = "0";

    pfa->alias->rgstry = pfa->font->rgstry;
    pfa->alias->encdng = pfa->font->encdng;
  }
}

static void app_jisx0208_1983_0(enum app_adstyl_spc_t adstyl_spc, bool is_bold,
    bool is_italic)
{
  app_jisx0201_1976_0(adstyl_spc, is_bold, is_italic);

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->fndry  = "jis";
    pfa->font->rgstry = "jisx0208.1983";

    pfa->alias->rgstry = pfa->font->rgstry;

    switch (i) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 6:
        pfa->font->fndry   = "misc";
        pfa->font->avgwdth = "140";
        break;

      case 5:
      case 7:
      case 8:
        pfa->font->pxlsz   = "16";
        pfa->font->ptsz    = "150";
        pfa->font->avgwdth = "160";
        break;

      case 9:
      case 10:
      case 11:
        pfa->font->pxlsz   = "24";
        pfa->font->ptsz    = "230";
        pfa->font->avgwdth = "240";
        break;
    }
  }
}

static void app_ksc5601_1987_0(enum app_adstyl_spc_t adstyl_spc, bool is_bold,
    bool is_italic)
{
  app_iso8859_partial(adstyl_spc, is_bold, is_italic);

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->fndry  = "daewoo";
    pfa->font->fmly   = "mincho";
    pfa->font->wght   = "medium";
    pfa->font->resx   =
    pfa->font->resy   = "100";
    pfa->font->rgstry = "ksc5601.1987";
    pfa->font->encdng = "0";

    pfa->alias->rgstry = pfa->font->rgstry;
    pfa->alias->encdng = pfa->font->encdng;

    switch (i) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
        pfa->font->fmly    = is_bold ? "mincho" : "gothic";
        pfa->font->pxlsz   = "16";
        pfa->font->ptsz    = "120";
        pfa->font->avgwdth = "160";
        break;

      case 9:
      case 10:
      case 11:
        pfa->font->pxlsz   = "24";
        pfa->font->ptsz    = "170";
        pfa->font->avgwdth = "240";
        break;
    }
  }
}

static void app_gb2312_1980_0(enum app_adstyl_spc_t adstyl_spc, bool is_bold,
    bool is_italic)
{
  app_iso8859_partial(adstyl_spc, is_bold, is_italic);

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->fndry  = "isas";
    pfa->font->fmly   = "song ti";
    pfa->font->wght   = "medium";
    pfa->font->resx   =
    pfa->font->resy   = "72";
    pfa->font->rgstry = "gb2312.1980";
    pfa->font->encdng = "0";

    pfa->alias->rgstry = pfa->font->rgstry;
    pfa->alias->encdng = pfa->font->encdng;

    switch (i) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
        pfa->font->fmly    = is_bold ? "song ti" : "fangsong ti";
        pfa->font->pxlsz   = "16";
        pfa->font->ptsz    = "160";
        pfa->font->avgwdth = "160";
        break;

      case 9:
      case 10:
      case 11:
        pfa->font->pxlsz   = "24";
        pfa->font->ptsz    = "240";
        pfa->font->avgwdth = "240";
        break;
    }
  }
}

static void app_dtsymbol_1(void) {
  app_iso8859_full(SANS_P, false, false);

  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];

    pfa->font->fndry  = "adobe";
    pfa->font->fmly   = "symbol";
    pfa->font->adstyl = "";
    pfa->font->rgstry = "adobe";
    pfa->font->encdng = "fontspecific";

    pfa->alias->adstyl = pfa->font->adstyl;
    pfa->alias->rgstry = "dtsymbol";
    pfa->alias->encdng = "1";

    switch (i) {
      case  0: pfa->font->avgwdth = "51";  break;
      case  2: pfa->font->avgwdth = "61";  break;
      case  4: pfa->font->avgwdth = "74";  break;
      case  6: pfa->font->avgwdth = "85";  break;
      case  8: pfa->font->avgwdth = "107"; break;
      case 10: pfa->font->avgwdth = "142"; break;

      case  1: pfa->font->avgwdth = "61";  break;
      case  3: pfa->font->avgwdth = "85";  break;
      case  5: pfa->font->avgwdth = "95";  break;
      case  7: pfa->font->avgwdth = "107"; break;
      case  9: pfa->font->avgwdth = "142"; break;
      case 11: pfa->font->avgwdth = "191"; break;
    }
  }
}

static void print_app(void (*fapp)(enum app_adstyl_spc_t, bool, bool),
    char *encdng)
{
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 2; ++j) {
      for (int k = 0; k < 2; ++k) {
        fapp(i, j, k);

        for (int l = 0; l < APP_FAL_SIZE; ++l) {
          struct font_alias_t *pfa = &app_fal[l];
          pfa->alias->encdng = pfa->font->encdng = encdng;
          print_font_alias(pfa);
        }
      }
    }
  }
}

static void print_app_iso8859_full(char *encdng) {
  print_app(app_iso8859_full, encdng);
}

static void print_app_iso8859_partial(char *encdng) {
  print_app(app_iso8859_partial, encdng);
}

static void print_app_iso8859_11(void) {
  print_app(app_iso8859_11, "11");
}

static void print_app_koi8_r(void) {
  print_app(app_koi8_r, "r");
}

static void print_app_jisx0201_1976_0(void) {
  print_app(app_jisx0201_1976_0, "0");
}

static void print_app_jisx0208_1983_0(void) {
  print_app(app_jisx0208_1983_0, "0");
}

static void print_app_ksc5601_1987_0(void) {
  print_app(app_ksc5601_1987_0, "0");
}

static void print_app_gb2312_1980_0(void) {
  print_app(app_gb2312_1980_0, "0");
}

static void print_app_dtsymbol_1(void) {
  app_dtsymbol_1();
  for (int i = 0; i < APP_FAL_SIZE; ++i) print_font_alias(&app_fal[i]);
}

static void mixed_iface_iso8859(bool is_user, bool is_bold) {
  iface_iso8859(is_user, is_bold);

  if (!is_user && is_bold) return;

  for (int i = 0; i < IFACE_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &iface_fal[i];

    pfa->font->fndry  = "b&h";
    pfa->font->fmly   = is_user ? "lucidatypewriter" : "lucidabright";
    pfa->font->wght   = is_bold ? "bold" : "medium";
    pfa->font->adstyl = is_user ? "sans" : "";
    pfa->font->spc    = is_user ? "m" : "p";
  }

  if (!is_user) {
    iface_alias_xxs.adstyl = "xxs serif";
    iface_alias_xs.adstyl  = "xs serif";
    iface_alias_s.adstyl   = "s serif";
    iface_alias_m.adstyl   = "m serif";
    iface_alias_l.adstyl   = "l serif";
    iface_alias_xl.adstyl  = "xl serif";
    iface_alias_xxl.adstyl = "xxl serif";
  }

  iface_font_xxs.pxlsz   = "10";
  iface_font_xxs.ptsz    = "100";
  iface_font_xxs.resx    = "75";
  iface_font_xxs.resy    = "75";
  iface_font_xxs.avgwdth = is_user ? "60" : "56";

  iface_font_xs.pxlsz   = "11";
  iface_font_xs.ptsz    = "80";
  iface_font_xs.resx    = "100";
  iface_font_xs.resy    = "100";
  iface_font_xs.avgwdth = is_user ? "70" : "63";

  iface_font_s.pxlsz   = "14";
  iface_font_s.ptsz    = "140";
  iface_font_s.resx    = "75";
  iface_font_s.resy    = "75";
  iface_font_s.avgwdth = is_user ? "90" : "80";

  iface_font_m.pxlsz   = "17";
  iface_font_m.ptsz    = "120";
  iface_font_m.resx    = "100";
  iface_font_m.resy    = "100";
  iface_font_m.avgwdth = is_user ? "100" : "96";

  iface_font_l.pxlsz   = "19";
  iface_font_l.ptsz    = "190";
  iface_font_l.resx    = "75";
  iface_font_l.resy    = "75";
  iface_font_l.avgwdth = is_user ? "110" : "109";

  iface_font_xl.pxlsz   = "20";
  iface_font_xl.ptsz    = "140";
  iface_font_xl.resx    = "100";
  iface_font_xl.resy    = "100";
  iface_font_xl.avgwdth = is_user ? "120" : "114";

  iface_font_xxl.pxlsz   = "24";
  iface_font_xxl.ptsz    = "240";
  iface_font_xxl.resx    = "75";
  iface_font_xxl.resy    = "75";
  iface_font_xxl.avgwdth = is_user ? "140" : "137";

  for (int i = 0; i < IFACE_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &iface_fal[i];

    pfa->alias->pxlsz   = pfa->font->pxlsz;
    pfa->alias->ptsz    = pfa->font->ptsz;
    pfa->alias->resx    = pfa->font->resx;
    pfa->alias->resy    = pfa->font->resy;
    pfa->alias->spc     = pfa->font->spc;
    pfa->alias->avgwdth = pfa->font->avgwdth;
  }
}

static void print_mixed_iface_iso8859(char *encdng) {
  print_iface(mixed_iface_iso8859, encdng);
}

int main(int argc, char *argv[]) {
  for (int i = 0; i < APP_FAL_SIZE; ++i) {
    struct font_alias_t *pfa = &app_fal[i];
    pfa->font  = &app_font[i];
    pfa->alias = &app_alias[i];
  }

  char *iface_encdng[] = {
    "1",
    "2",
    "3",
    "4",
    "5",
    "7",
    "8",
    "9",
    "10",
    "13",
    "14",
    "15",
    "16"
  };

  for (int i = 0; i < sizeof(iface_encdng) / sizeof(char *); ++i) {
    char *encdng = iface_encdng[i];

    if (argc > 1) {
      if (strcmp(encdng, "5") &&
          strcmp(encdng, "7") &&
          strcmp(encdng, "8") &&
          strcmp(encdng, "16"))
        print_mixed_iface_iso8859(encdng);
    }
    else print_iface_iso8859(encdng);
  }

  print_iface_iso8859_11();
  print_iface_koi8_r();
  print_iface_jisx0201_1976_0();
  print_iface_jisx0208_1983_0();
  print_iface_ksc5601_1987_0();
  print_iface_gb2312_1980_0();

  char *app_full_encdng[] = {"1", "2", "3", "4", "9", "10", "13", "14", "15"};

  for (int i = 0; i < sizeof(app_full_encdng) / sizeof(char *); ++i)
    print_app_iso8859_full(app_full_encdng[i]);

  char *app_partial_encdng[] = {"5", "7", "8", "16"};

  for (int i = 0; i < sizeof(app_partial_encdng) / sizeof(char *); ++i)
    print_app_iso8859_partial(app_partial_encdng[i]);

  print_app_iso8859_11();
  print_app_koi8_r();
  print_app_jisx0201_1976_0();
  print_app_jisx0208_1983_0();
  print_app_ksc5601_1987_0();
  print_app_gb2312_1980_0();
  print_app_dtsymbol_1();
}
