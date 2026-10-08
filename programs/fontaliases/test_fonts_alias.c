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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>

#define BUF_SIZE 1024

int main(void) {
  int ret = 0;
  int line_num = 0;
  int npaths;
  char cwd[BUF_SIZE];
  char *font_path_list[BUF_SIZE];
  char **font_path_list_orig = NULL;
  char *line = NULL;
  size_t len = 0;
  ssize_t line_len;

  if (!getcwd(cwd, BUF_SIZE)) {
    fprintf(stderr, "Cannot get current working directory.\n");
    ret = -1;
    goto done;
  }

  font_path_list[0] = cwd;

  Display *display = XOpenDisplay(NULL);

  if (display == NULL) {
    fprintf(stderr, "Cannot open display.\n");
    ret = -1;
    goto done;
  }

  font_path_list_orig = XGetFontPath(display, &npaths);

  for (int i = 0; i < npaths; ++i)
    font_path_list[i + 1] = font_path_list_orig[i];

  XSetFontPath(display, font_path_list, ++npaths);

  while ((line_len = getline(&line, &len, stdin)) != -1) {
    int i;
    char *font_name = NULL;
    size_t font_name_len = 0;

    ++line_num;

    for (i = 0; i < line_len; ++i) {
      ++font_name_len;

      if (!font_name && line[i] == '"') {
        font_name = &line[i];
        font_name_len = 0;
      }
      else if (font_name && line[i] == '"') break;
    }

    if (i >= line_len || line_len < 3) {
      fprintf(stderr, "Corrupted file.\n");
      ret = -1;
      goto done;
    }

    *(++font_name + --font_name_len) = '\0';

    XFontStruct *font_struct = XLoadQueryFont(display, font_name);

    if (!font_struct) {
      fprintf(stderr, "Bad alias: %d: %s\n", line_num, font_name);
      ret = -1;
    }

    if (font_struct) XFreeFont(display, font_struct);
  }

done:
  if (font_path_list_orig) {
    XSetFontPath(display, font_path_list_orig, --npaths);
    XFreeFontPath(font_path_list_orig);
  }

  if (line) free(line);
  if (display) XCloseDisplay(display);

  return ret;
}
