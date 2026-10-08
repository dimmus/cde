/*
 * CDE - Common Desktop Environment
 *
 * Copyright (c) 1993-2012, The Open Group. All rights reserved.
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
/* $XConsortium: msgs.c /main/4 1995/11/01 15:56:14 rswiston $ */
#include "stdio.h"
#include <sys/param.h>
#include <string.h>
#ifndef _SUN_OS
#include <nl_types.h>
#endif
#include "msgs.h"
#include "error.h"

char *_DtGetMessage(char *filename, char *s)
{
    char * loc = getenv("LANG");
    return(ERROR_translate(loc, 0, _CLIENT_CAT_NAME, s));
}

char *
GetSharedMsg(
        int msgId )
{
   char * msg;

   switch (msgId)
   {
      case DT_BAD_FIELD_NAME:
         msg = GETMESSAGE("Cannot find a field named '%s' in the structure '%s'");
         break;

      case DT_CONV_BUF_OVFL:
         msg = GETMESSAGE("DtkshCvtCallbackToString: An internal conversion buffer overflowed");
         break;

      case DT_HASHING_FAILURE:
         msg = GETMESSAGE("Hashing failure for resource '%s' in widget class '%s'");
         break;

      case DT_NO_FUNC_NAME:
         msg = GETMESSAGE("No function name was supplied");
         break;

      case DT_TK_NOT_INIT:
         msg = GETMESSAGE("The command cannot be executed; the toolkit has not been initialized");
         break;

      case DT_WIDGET_CREATE_FAILED:
         msg = GETMESSAGE("The creation of widget '%s' failed");
         break;

      case DT_BAD_DISPLAY:
         msg = GETMESSAGE("The display parameter is invalid: %s");
         break;

      case DT_BAD_ATOM:
         msg = GETMESSAGE("The following property atom is invalid: %s");
         break;

      case DT_UNDEF_RESOURCE:
         msg = GETMESSAGE("The following resource is not defined for widget '%s': %s");
         break;

      case DT_BAD_POSITION:
         msg = GETMESSAGE("The position specified is invalid: %s");
         break;

      case DT_BAD_FONT:
         msg = GETMESSAGE("The specified font is invalid: %s");
         break;

      case DT_BAD_WIDGET_HANDLE:
         msg = GETMESSAGE("The widget handle '%s' does not refer to an existing widget");
         break;

      case DT_CMD_WIDGET:
         msg = GETMESSAGE("The widget must be a 'command' widget");
         break;

      case DT_MAIN_WIN_WIDGET:
         msg = GETMESSAGE("The widget must be a 'mainWindow' widget");
         break;

      case DT_SCALE_WIDGET:
         msg = GETMESSAGE("The widget must be a 'scale' widget");
         break;

      case DT_SCROLLBAR_WIDGET:
         msg = GETMESSAGE("The widget must be a 'scrollBar' widget");
         break;

      case DT_TOGGLE_WIDGET:
         msg = GETMESSAGE("The widget must be a 'toggleButton' widget or gadget");
         break;

      case DT_BAD_WINDOW:
         msg = GETMESSAGE("The window parameter is invalid: %s");
         break;

      case DT_ALLOC_FAILURE:
         msg = GETMESSAGE("Unable to allocate required memory; exiting");
         break;

      case DT_NO_PARENT:
         msg = GETMESSAGE("Unable to find the parent widget");
         break;

      case DT_UNDEF_SYMBOL:
         msg = GETMESSAGE("Unable to locate the symbol '%s'");
         break;

      case DT_UNDEF_TYPE:
         msg = GETMESSAGE("Unable to locate the type '%s'");
         break;

      case DT_BAD_DECL:
         msg = GETMESSAGE("Unable to parse the declaration '%s'; using 'unsigned long'");
         break;

      case DT_UNKNOWN_CHILD_TYPE:
         msg = GETMESSAGE("Unknown child type: %s");
         break;

      case DT_UNKNOWN_OPTION:
         msg = GETMESSAGE("Unrecognized option flag: %s");
         break;

      case DT_USAGE_WIDGET_POS:
         msg = GETMESSAGE("Usage: %s widget position");
         break;

      case DT_USAGE_WIDGET_TIME:
         msg = GETMESSAGE("Usage: %s widget time");
         break;

      case DT_USAGE_WIDGET:
         msg = GETMESSAGE("Usage: %s widget");
         break;

      case DTKSH_ERROR:
         msg = GETMESSAGE("dtksh error");
         break;

      case DT_WARNING:
         msg = GETMESSAGE("dtksh warning");
         break;

      case DT_USAGE_DISPLAY_WINDOW:
         msg = GETMESSAGE("Usage: %s display window");
         break;

      case DT_USAGE_DISPLAY_WINDOW_VAR:
         msg = GETMESSAGE("Usage: %s display window variable");
         break;

      case DT_USAGE_DISPLAY_ROOT_VAR:
         msg = GETMESSAGE("Usage: %s display rootWindow variable");
         break;

      case DT_BAD_TIMEOUT:
         msg = GETMESSAGE("The timeout parameter is invalid: %s");
         break;

      case DT_BAD_MESSAGE:
         msg = GETMESSAGE("The msg parameter is invalid: %s");
         break;

      case DT_BAD_PATTERN:
         msg = GETMESSAGE("The patterns parameter is invalid: %s");
         break;
   }
   return(msg);
}
