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
/*****************************************************************************
 *
 *   File:         WmFP.c
 *
 *   Project:	   CDE
 *
 *   Description:  This file contains functions used between the window manager
 *                 and front panel.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
 *
 ****************************************************************************/


#include "WmGlobal.h"


#include <Dt/DtP.h>
#include <Dt/DbReader.h>
#include <Dt/Control.h>
#include <Dt/ControlP.h>
#include <Dt/IconFile.h>
#include <Dt/Icon.h>
#include <Dt/Wsm.h>
#include <Dt/MacrosP.h>

#include <Xm/Form.h>
#include <Xm/ToggleBG.h>
#include <Xm/MwmUtil.h>
#include <Xm/AtomMgr.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/DialogS.h>

#include <X11/Xatom.h>
#include <X11/keysymdef.h>

#include "PanelSP.h"
#include "DataBaseLoad.h"
#include "UI.h"

#include "WmPanelP.h"
#include "WmManage.h"
#include "WmWinConf.h"
#include "WmWinInfo.h"

#include <pwd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>



extern void SubpanelTornEventHandler (Widget, XtPointer, XEvent *, Boolean *);
extern void WorkspaceModifyCB (Widget, Atom, int, XtPointer);
extern void SessionRestoreData (void);
extern void UnManageWindow (ClientData *pCD);
extern void WorkspaceAdjustPanelPosition (Position x,
                              Position y,
                              Dimension width,
                              Dimension height);

static void PushRecallSetData ();
static void EmbeddedClientSetData ();
static void EmbeddedClientSetGeometry (WmFpEmbeddedClientData *);





/************************************************************************
 *
 *  EmbeddedClientReposition
 *      This function moves client windows within the main panel after
 *      controls have been resized.
 *
 *      Inputs: icon - the icon of the embedded client to be repositioned
 *              x - the position of the resized control
 *              adjust - how much to move the window
 *
 ************************************************************************/


void
EmbeddedClientReposition (Widget    icon,
                          Position  x,
                          Dimension adjust)



{
   WmFpEmbeddedClientList embedded_client_list = 
      (WmFpEmbeddedClientList) panel.embedded_client_list;

   int i;
      

   for (i = 0; i < panel.embedded_client_count; i++)
   {
      if (XtParent (icon) == XtParent (embedded_client_list[i].wControl) &&
          embedded_client_list[i].x > x)
      {
         if (embedded_client_list[i].pCD != NULL)
         {
            embedded_client_list[i].x += adjust;
            XMoveWindow (XtDisplay (panel.shell),
                         embedded_client_list[i].pCD->client,
                         embedded_client_list[i].x, embedded_client_list[i].y);
         }
         else
            embedded_client_list[i].x += adjust;
      }
   }
}




/************************************************************************
 *
 *  EmbeddedClientReparent
 *      This function sets of the data necessary to call the window
 *      manager function which reparents an embedded client to a new
 *      control.
 *
 *      Inputs: client_name - the name of the client to be reparented.
 *              icon - the widget that is to contain the client.
 *              parent_win - the parent win to reparent the client to.
 *
 ************************************************************************/


void
EmbeddedClientReparent (char   * client_name,
                        Widget   icon)


{
   WmFpEmbeddedClientList embedded_client_list = 
      (WmFpEmbeddedClientList) panel.embedded_client_list;

   int i;
      

   for (i = 0; i < panel.embedded_client_count; i++)
   {
      if (strcmp (client_name, embedded_client_list[i].pchResName) == 0)
         break;
   }


   /*  Try to reparent the client to/from the main panel.  If this    */
   /*  fails, it is because of a dtwm restart and the clients have    */
   /*  not yet been grabbed.  So just reset the embedded client data  */

   if (ReparentEmbeddedClient (&embedded_client_list[i], icon,
                               XtWindow (XtParent (icon)),
                               XtX (icon) + 3, XtY (icon) + 3,
                               XtWidth(icon) - 6, XtHeight(icon) - 6) == False)
   {
      embedded_client_list[i].wControl = icon;
      embedded_client_list[i].winParent = XtWindow (XtParent (icon));
      embedded_client_list[i].x = XtX (icon) + 3;
      embedded_client_list[i].y =  XtY (icon) + 3;
      embedded_client_list[i].width =  XtWidth (icon) - 6;
      embedded_client_list[i].height =  XtHeight (icon) - 6;
   }
}




/************************************************************************
 *
 *  EmbeddedClientRegister
 *	This function adds a control to the list of embedded client controls
 *	and resets, if necessary, the Window Managers pointers to the list.
 *
 *      Inputs:
 *        control_data - the pointer to the ControlData * structure of
 *	                 the control to be installed or removed.
 *
 *        install - Boolean indicating whether to add or remove the control
 *	            from the push recall list.
 *
 ************************************************************************/

void
EmbeddedClientRegister (ControlData * control_data,
                        Boolean       install)


{
   char * client_name;
   Window client_window;
   int i, j;

   WmFpEmbeddedClientList embedded_client_list = 
      (WmFpEmbeddedClientList) panel.embedded_client_list;

   WmFpEmbeddedClientData * embedded_client = NULL;

   if ((intptr_t) control_data->
                  element_values[CONTROL_TYPE].parsed_value != CONTROL_CLIENT)
      return;


   /*  If this is a control installation, increase the list size, if     */
   /*  needed, and initialize the embedded client structure.  If it is   */
   /*  a removal, find the control in the list and remove it by sliding  */
   /*  each subsequent structure down one element in array.              */

   client_name = (char *)
               control_data->element_values[CONTROL_CLIENT_NAME].parsed_value;

   if (client_name == NULL)
      client_name = (char *)
                    control_data->element_values[CONTROL_LABEL].parsed_value;

   if (install)
   {

      /*  First see if this is an update to an already installed  */
      /*  embedded client.  If so, simply update the icon field   */
      
      for (i = 0; i < panel.embedded_client_count; i++)
      {
	 if (strcmp (client_name, embedded_client_list[i].pchResName) == 0)
         {
	    embedded_client_list[i].wControl = control_data->icon;
            break;
         }
      }

      if (i >= panel.embedded_client_count)
      {
         if (panel.embedded_client_count == panel.max_embedded_client_count)
         {
            panel.max_embedded_client_count += 10;
            embedded_client_list = (WmFpEmbeddedClientList) 
               XtRealloc ((char *) embedded_client_list,
                          sizeof (WmFpEmbeddedClientData) * 
			          panel.max_embedded_client_count);
            panel.embedded_client_list = (XtPointer) embedded_client_list;
         }

         embedded_client = &embedded_client_list[panel.embedded_client_count];
         panel.embedded_client_count++;

         embedded_client->pchResName = XtNewString (client_name);
         embedded_client->wControl = control_data->icon;
         embedded_client->winParent = None;
         embedded_client->pCD = NULL;
      }

      for (i = 0; i < panel.embedded_client_count - 1; i++)
      {
          embedded_client = &embedded_client_list[i];
          if (embedded_client->pCD != NULL)
             embedded_client->pCD->pECD = (void *) embedded_client;
      }
   }
   else
   {
      for (i = 0; i < panel.embedded_client_count; i++)
      {
	 if (embedded_client_list[i].wControl == control_data->icon)
	 {
            Position remove_y;
            Dimension adjust_y = 0;


            /*  This block will reparent the client window, move it  */
            /*  to a new window location, and remap the window.      */

            if (embedded_client_list[i].pCD != NULL)
            {
               client_window = embedded_client_list[i].pCD->client;
               UnManageWindow (embedded_client_list[i].pCD);
               XSync (XtDisplay (panel.shell), False);
               XMoveWindow (XtDisplay (panel.shell), client_window, 0, 0);
               XMapWindow (XtDisplay (panel.shell), client_window);
            }

            remove_y = embedded_client_list[i].y;


            /* deleted control height plus 5 pixels of form offset */

            if(control_data->icon) {
               adjust_y = XtHeight (control_data->icon) + 5;
            }
            
            panel.embedded_client_count--;

            XtFree (embedded_client_list[i].pchResName);

	    for (j = i; j < panel.embedded_client_count; j++)
            {
	       embedded_client_list[j] = embedded_client_list[j + 1];
               embedded_client = &embedded_client_list[j];

               if (embedded_client_list[i].pCD != NULL)
               {
                  embedded_client->pCD->pECD = (void *) embedded_client;

                  if (control_data->icon && XtParent(control_data->icon) == 
                      XtParent(embedded_client->wControl) &&
                      remove_y < embedded_client->y)
                  {
                     embedded_client->y -= adjust_y;
                     XMoveWindow(XtDisplay(panel.shell),
                                 embedded_client->pCD->client,
                                 embedded_client->x, embedded_client->y);
                  }
               }
            }

            break;
	 }
      }
   }


   /*  Set the embedded client list and count into the window manager's  */
   /*  screen global data.                                               */
   
   EmbeddedClientSetData ();
}




/************************************************************************
 *
 *   EmbeddedClientSetData
 *	Set the embedded client list and count into the window manager's
 *	screen global data.
 *
 ************************************************************************/

static void
EmbeddedClientSetData (void)
{
   WmScreenData *pSD;
   int i;


   /*  Find the screen which contains the front panel and set it  */
   /*  embedded client list and count.                            */
   
   for (i = 0; i < wmGD.numScreens; i++)
   {
      pSD = &(wmGD.Screens[i]);

      if (pSD->managed)
      {
         if (pSD->wPanelist == panel.form)
         {
            pSD->pECD = (struct _WmFpEmbeddedClientData *) panel.embedded_client_list;
            pSD->numEmbeddedClients = panel.embedded_client_count;
            break;
         }
      }
   }
}




/************************************************************************
 *
 *  EmbeddedClientSetGeometry
 *	Set the geometry and parenting window information for an 
 *	embedded client control.
 *
 *  Inputs: embedded_client_data - a pointer to the structure containing
 *	    the embedded client information for a control.
 *
 *  NOTE: Simplistic setting of position and size information.  May need
 *	  to base on resolution and component parent.
 *
 ************************************************************************/

static void
EmbeddedClientSetGeometry (WmFpEmbeddedClientData * embedded_client_data)


{
   Widget control;
   
   control = embedded_client_data->wControl;

   embedded_client_data->winParent = XtWindow (XtParent (control));
      

   embedded_client_data->x = XtX (control) + 3;
   embedded_client_data->y = XtY (control) + 3;
   embedded_client_data->width = XtWidth (control) - 6;
   embedded_client_data->height = XtHeight (control) - 6;
}




/************************************************************************
 *
 *  PushRecallRegister
 *	This function adds a control to the list of push recall controls
 *	and resets, if necessary, the Window Managers pointers to the list.
 *
 *      Inputs:
 *        control_data - the pointer to the ControlData * structure of
 *	                 the control to be installed or removed.
 *
 *        install - Boolean indicating whether to add or remove the control
 *	            from the push recall list.
 *
 ************************************************************************/

void
PushRecallRegister (ControlData * control_data,
                    Boolean       install)


{
   char * client_name;
   int i, j;

   WmFpPushRecallClientList push_recall_list = 
      (WmFpPushRecallClientList) panel.push_recall_list;

   WmFpPushRecallClientData * push_recall = NULL;


   /*  If this is a control installation, increase the list size, if  */
   /*  needed, and initialize the push recall structure.  If it is a  */
   /*  removal, find the control in the push recall list and remove   */
   /*  it by sliding each subsequent structure down one element in    */
   /*  array.                                                         */

   if (install)
   {
      client_name = (char *) control_data->element_values[CONTROL_CLIENT_NAME].parsed_value;

      if (client_name == NULL)
        client_name = (char *) control_data->element_values[CONTROL_LABEL].parsed_value;


      /*  First see if this is an update to an already installed     */
      /*  push recall client.  If so, simply update the icon field   */
      
      for (i = 0; i < panel.push_recall_count; i++)
      {
	 if (strcmp (client_name, push_recall_list[i].pchResName) == 0)
         {
	    push_recall_list[i].wControl = control_data->icon;
            break;
         }
      }

      if (i >= panel.push_recall_count)
      {
         if (panel.push_recall_count == panel.max_push_recall_count)
         {
            panel.max_push_recall_count += 10;
            push_recall_list = (WmFpPushRecallClientList) 
               XtRealloc ((char *) push_recall_list,
                          sizeof (WmFpPushRecallClientData) * 
			          panel.max_push_recall_count);
            panel.push_recall_list = (XtPointer) push_recall_list;
         }

         push_recall = &push_recall_list[panel.push_recall_count];
         panel.push_recall_count++;

         push_recall->pchResName = XtNewString (client_name);
         push_recall->wControl = control_data->icon;
         push_recall->pCD = NULL;
         push_recall->tvTimeout.tv_sec = 0;
      }

      for (i = 0; i < panel.push_recall_count - 1; i++)
      {
          push_recall = &push_recall_list[i];
          if (push_recall->pCD != NULL)
             push_recall->pCD->pPRCD = (void *) push_recall;
      }
   }
   else
   {
      for (i = 0; i < panel.push_recall_count; i++)
      {
	 if (push_recall_list[i].wControl == control_data->icon)
	 {
            panel.push_recall_count--;

            XtFree (push_recall_list[i].pchResName);

	    for (j = i; j < panel.push_recall_count; j++)
            {
	       push_recall_list[j] = push_recall_list[j + 1];
               push_recall = &push_recall_list[j];

               if (push_recall->pCD != NULL)
                  push_recall->pCD->pPRCD = (void *) push_recall;
            }
	    
	    break;
	 }
      }
   }


   /*  Set the push recall list and count into the window manager's  */
   /*  screen global data.                                           */
   
   PushRecallSetData ();
}




/************************************************************************
 *
 *   PushRecallSetData
 *	Set the push recall list and count into the window manager's
 *	screen global data.
 *
 ************************************************************************/

static void
PushRecallSetData (void)
{
   WmScreenData *pSD;
   int i;


   /*  Find the screen which contains the front panel and set it  */
   /*  push recall list and count.                                */
   
   for (i = 0; i < wmGD.numScreens; i++)
   {
      pSD = &(wmGD.Screens[i]);

      if (pSD->managed)
      {
         if (pSD->wPanelist == panel.form)
         {
            pSD->pPRCD = (struct _WmFpPushRecallClientData *) panel.push_recall_list;
            pSD->numPushRecallClients = panel.push_recall_count;
            break;
         }
      }
   }
}




/************************************************************************
 *
 *  PushRecallGetData
 *	Find the index within the push recall list of the entry which
 *	contains the passed client name.
 *
 *      Inputs:
 *	  client_name - contains the name of the push recall client to
 *        be used for the lookup.
 *
 ************************************************************************/


int
PushRecallGetData (char * client_name)


{
   WmFpPushRecallClientList push_recall_list = 
      (WmFpPushRecallClientList) panel.push_recall_list;
   int i;
   
   for (i = 0; i < panel.push_recall_count; i++)
      if (strcmp (client_name, push_recall_list[i].pchResName) == 0)
         return (i);

   /* Should never get here */
   return 0;
}
   
 


/************************************************************************
 *
 *  WmPanelistWindowToSubpanel
 *	Get a panel widget given its shell window.
 *
 ************************************************************************/

Widget
WmPanelistWindowToSubpanel (Display *dpy, Window win)

{
   Widget   subpanel_shell;
   Widget * child_list;
   int      num_children;

   Arg      al[2];
   int      ac;


   /*  Get the widget for the subpanel (Should be only child of the shell!) */

   subpanel_shell = XtWindowToWidget (dpy, win);

   ac = 0; 
   XtSetArg (al[ac], XmNchildren, &child_list);  ac++;
   XtSetArg (al[ac], XmNnumChildren, &num_children);  ac++;
   XtGetValues (subpanel_shell, al, ac);

   return ((num_children > 0) ? child_list[0] : (Widget)NULL);
}




/************************************************************************
 *
 *  WmPanelistSetBusy
 *	Turn on or off the busy light.
 *
 ************************************************************************/

void
WmFrontPanelSetBusy (Boolean on)

{
   if (panel.busy_light_data != NULL)
      _DtControlSetBusy (panel.busy_light_data->icon, on);
}




/*
 *  How the front panel is placed, for when the screen changes size
 *  (WmPanelistScreenResized): by the geometry it was given (resources,
 *  session or panel definition), or, without one, centered along the
 *  bottom of the screen; once the user has moved it, where it is.
 */

static String  panel_geometry = NULL;	/* NULL: centered along the bottom */
static Boolean panel_moved = False;

/*  How far from the default place a geometry may put the panel and   */
/*  still count as the default place (the position saved for a        */
/*  session is the frame's, a few pixels off).                        */

#define PANEL_PLACE_SLACK 8



/************************************************************************
 *
 *  GeometryPosition
 *	The position that a geometry gives a window of the given size on
 *	a screen of the given size.  *x and *y are left alone where the
 *	geometry gives none.  Returns XParseGeometry's flags.
 *
 ************************************************************************/

static int
GeometryPosition (String geometry,
                  int    screen_width,
                  int    screen_height,
                  int    width,
                  int    height,
                  int  * x,
                  int  * y)

{
   int x_return, y_return;
   unsigned int width_return, height_return;
   int flags;

   flags = XParseGeometry (geometry, &x_return, &y_return,
                           &width_return, &height_return);

   if (flags & XValue)
      *x = (flags & XNegative) ? screen_width - width + x_return : x_return;

   if (flags & YValue)
      *y = (flags & YNegative) ? screen_height - height + y_return : y_return;

   return (flags);
}



/************************************************************************
 *
 *  SetGeometry
 *	set x and y based on given geometry
 *
 ************************************************************************/

static void 
SetGeometry (Widget w, String geometry, Position * x, Position * y)
			
{
   int new_x = *x;
   int new_y = *y;

   if (GeometryPosition (geometry, WidthOfScreen (XtScreen (w)),
                         HeightOfScreen (XtScreen (w)), XtWidth (w),
                         XtHeight (w), &new_x, &new_y) & (XValue | YValue))
   {
      *x = (Position) new_x;
      *y = (Position) new_y;
      XtMoveWidget (w, *x, *y);
   }
}



/************************************************************************
 *
 *  CenteredAlongBottom
 *	Whether a geometry puts the panel about where the default one
 *	does, centered along the bottom of the screen.  The position
 *	saved for the session of a panel that was there does: such a
 *	panel is placed by the default rule when the screen changes size.
 *
 ************************************************************************/

static Boolean
CenteredAlongBottom (Widget w, String geometry)

{
   int screen_width = WidthOfScreen (XtScreen (w));
   int screen_height = HeightOfScreen (XtScreen (w));
   int width = XtWidth (w);
   int height = XtHeight (w);
   int center_x = (screen_width > width) ? (screen_width - width) / 2 : 0;
   int x = 0, y = 0;
   int flags;

   flags = GeometryPosition (geometry, screen_width, screen_height,
                             width, height, &x, &y);

   return ((flags & XValue) && (flags & YValue) &&
           abs (x - center_x) <= PANEL_PLACE_SLACK &&
           abs (screen_height - height - y) <= PANEL_PLACE_SLACK);
}



/************************************************************************
 *
 *   WmPanelistShow
 *
 ************************************************************************/

void
WmPanelistShow (Widget w)

{
   SwitchData * switch_data;

   Dimension width = XtWidth(panel.shell);
   Position x = XtX(panel.shell);
   Position y = XtY(panel.shell);
   Dimension screen_width;

   Display * display;

   CompositeWidget  cw;
   Widget         * widget_list;
   DtWmHints        vHints;

   String    shell_geometry = NULL;
   char      geometry_buffer[32];
   Boolean   geometry_given = False;

   XSizeHints   hints;
   long	        supplied;

   int i;
   Arg al[20];
   int ac;

   /*  Find the switch data for later processing  */
   
   switch_data = NULL;
   
   for (i = 0; i < panel.box_data_count; i++)
   {
      if (panel.box_data[i]->switch_data != NULL)
      {
         switch_data = panel.box_data[i]->switch_data;
         break;
      }
   }


   /*  Realize the shell so that it is sized properly for later  */
   /*  positioning and child repositioning.                      */

   XtRealizeWidget (panel.shell);


   /*  See if a geometry has been set.  */
   
   ac = 0;
   XtSetArg (al[ac], XmNgeometry, &shell_geometry);  ac++;
   XtGetValues (panel.shell, al, ac);


   /*  If the shell has no default geometry, construct a default    */
   /*  which will center the panel along the bottom of the display  */

   width = XtWidth (panel.shell);
   screen_width = WidthOfScreen (XtScreen (panel.shell));
   display = XtDisplay (panel.shell);

   if (shell_geometry != NULL)
      geometry_given = True;
   else
   {
      Position  x;

      if (panel.element_values[PANEL_GEOMETRY].string_value != NULL)
      {
        shell_geometry = panel.element_values[PANEL_GEOMETRY].parsed_value;
        geometry_given = True;
      }
      else
      {
         x = (screen_width > width) ? (Position)(screen_width - width) / 2 : 0;
         snprintf (geometry_buffer, 32 - 1, "+%d-0", x);
         shell_geometry = geometry_buffer;
      }
      
      XtSetArg (al[0], XmNgeometry, shell_geometry);
      XtSetValues (panel.shell, al, 1);
   }


   /*  Adjust the positions of the buttons within the switch  */
   /*  so that they are spaced nicely.                        */

   if (switch_data != NULL)
   {
      UpdateSwitchGeometry (switch_data->box_data);


      /*  Reposition or adjust the front panel if it is either off  */
      /*  the right edge of the screen or larger than the screen    */

      if (width > screen_width)
      {
         Arg al[1];

         while (width > screen_width && 
                panel.switch_row_count < switch_data->switch_count)
         {	    
            panel.switch_row_count++;

            XtSetArg (al[0], XmNnumColumns, panel.switch_row_count);
            XtSetValues (switch_data->rc, al, 1);

            width = XtWidth (panel.shell);
         }
      }
   }

   SetGeometry (panel.shell, shell_geometry, &x, &y);
   WorkspaceAdjustPanelPosition (x, y, XtWidth (panel.shell),
                                 XtHeight (panel.shell));


   /*  Remember the geometry for when the screen changes size, unless  */
   /*  it is the default place.                                         */

   XtFree (panel_geometry);
   panel_geometry = NULL;
   if (geometry_given && !CenteredAlongBottom (panel.shell, shell_geometry))
      panel_geometry = XtNewString (shell_geometry);


   /* Set hints to avoid interactive placement */

   if (XGetWMNormalHints(display, XtWindow(panel.shell),
                         &hints, &supplied) != 0)
   {
      hints.flags |= USPosition|USSize;
      XSetWMNormalHints(display, XtWindow(panel.shell), &hints);
   }


   /*  Set the shells icon and title for when it is minimized  */
   
   if (switch_data != NULL)
   {
      int current_workspace = switch_data->active_switch;

      XtSetArg (al[0], XmNiconName, switch_data->switch_names[current_workspace]);
      XtSetArg (al[1], XmNtitle, switch_data->switch_names[current_workspace]);
      XtSetValues (panel.shell, al, 2);
   }


   /*  Set panel's window manager hints.  */

   vHints.flags = DtWM_HINTS_BEHAVIORS;
   vHints.behaviors = DtWM_BEHAVIOR_PANEL;
   _DtWsmSetDtWmHints (XtDisplay(panel.shell), XtWindow (panel.shell), &vHints);


   /*  Set the subpanel's transientShell to update transientFor and  */
   /*  Set the subpanel hints.                                       */

   vHints.behaviors |= DtWM_BEHAVIOR_SUBPANEL;
   vHints.flags |= DtWM_HINTS_ATTACH_WINDOW;
   vHints.attachWindow = XtWindow (panel.shell);

   for (i = 0, widget_list = M_PopupList (panel.shell); 
        i < M_NumPopups (panel.shell); i++)
   {
      cw = (CompositeWidget) widget_list[i];

      ac = 0;
      XtSetArg (al[ac], XmNtransientFor, NULL);  ac++;
      XtSetValues ((Widget) cw, al, ac);

      ac = 0;
      XtSetArg (al[ac], XmNtransientFor, panel.shell);  ac++;
      XtSetValues ((Widget) cw, al, ac);

      if (M_NumChildren (cw) > 0)
      {
         XtRealizeWidget ((M_Children (cw))[0]);
         _DtWsmSetDtWmHints (XtDisplay (panel.shell), 
                             XtWindow (widget_list[i]), &vHints);
      }
   }


   /*  Set the push recall list and count into the window manager's  */
   /*  screen global data.                                           */
   
   PushRecallSetData ();


   /*  Set up the callback ot the workspace management API for  */
   /*  catching changes in workspace configuration.             */
      
   DtWsmAddWorkspaceModifiedCallback(panel.shell,
                                     WorkspaceModifyCB, (XtPointer)switch_data);


   /*  Get the front panel displayed  */

   XtSetMappedWhenManaged (panel.shell, True);
   XtPopup (panel.shell, XtGrabNone);


   /*  Restore the session information  */

   SessionRestoreData ();


   /*  Set up the window and geometry information for the embedded clients  */

   for (i = 0; i < panel.embedded_client_count; i++)
      EmbeddedClientSetGeometry (&(((WmFpEmbeddedClientList) panel.embedded_client_list)[i]));


   /*  Set the embedded client list and count into the window manager's  */
   /*  screen global data.                                               */

   EmbeddedClientSetData ();
}




/************************************************************************
 *
 *  WmPanelistMoved
 *	Called when the user has moved the front panel (f.move, with the
 *	pointer or the keyboard): from then on, the panel stays where the
 *	user put it when the screen changes size.
 *
 ************************************************************************/

void
WmPanelistMoved (void)

{
   panel_moved = True;
}




/************************************************************************
 *
 *  WmPanelistPlacedByDefault
 *	Whether the front panel is where the default rule puts it,
 *	centered along the bottom of the screen: neither a geometry nor
 *	the user put it elsewhere.  Its position is not saved for the
 *	session then, so that it is centered again on the next screen.
 *
 ************************************************************************/

Boolean
WmPanelistPlacedByDefault (void)

{
   return (!panel_moved && panel_geometry == NULL);
}




/************************************************************************
 *
 *  WmPanelistScreenResized
 *	The screen changed size: place the front panel again.  Placed by
 *	the default rule, it is centered along the bottom again; placed
 *	by a geometry, the geometry is applied again (an edge given by a
 *	negative offset follows the new size); moved by the user, it
 *	stays.  In each case it is kept all on the screen.  The subpanels
 *	that are posted follow it.
 *
 ************************************************************************/

void
WmPanelistScreenResized (WmScreenData * pSD)

{
   ClientData * pCD;
   ClientData * pcdSub;
   String       geometry = panel_geometry;
   char         geometry_buffer[32];
   int          screen_width = DisplayWidth (DISPLAY, pSD->screen);
   int          screen_height = DisplayHeight (DISPLAY, pSD->screen);
   int          frame_x, frame_y, x = 0, y = 0, x_off = 0, y_off = 0;
   int          dx, dy, flags;

   if (!pSD->wPanelist || pSD->wPanelist != panel.form || !panel.shell ||
       !XtIsRealized (panel.shell) ||
       XFindContext (DISPLAY, XtWindow (panel.shell),
                     wmGD.windowContextType, (XPointer *) &pCD))
      return;

   frame_x = pCD->frameInfo.x;
   frame_y = pCD->frameInfo.y;

   if (!panel_moved)
   {
      /*  As WmPanelistShow and the window manager placed it at first:  */
      /*  the window by the geometry, its frame by the window gravity.  */

      if (geometry == NULL)
      {
         x = (screen_width > (int) pCD->clientWidth) ?
             (screen_width - (int) pCD->clientWidth) / 2 : 0;
         snprintf (geometry_buffer, sizeof (geometry_buffer), "+%d-0", x);
         geometry = geometry_buffer;
      }

      flags = GeometryPosition (geometry, screen_width, screen_height,
                                pCD->clientWidth, pCD->clientHeight, &x, &y);

      if (wmGD.positionIsFrame)
         CalculateGravityOffset (pCD, &x_off, &y_off);

      if (flags & XValue)
         frame_x = x + x_off - (pCD->clientX - pCD->frameInfo.x);
      if (flags & YValue)
         frame_y = y + y_off - (pCD->clientY - pCD->frameInfo.y);
   }

   /*  All of it on the screen  */

   if (frame_x + (int) pCD->frameInfo.width > screen_width)
      frame_x = screen_width - (int) pCD->frameInfo.width;
   if (frame_x < 0)
      frame_x = 0;
   if (frame_y + (int) pCD->frameInfo.height > screen_height)
      frame_y = screen_height - (int) pCD->frameInfo.height;
   if (frame_y < 0)
      frame_y = 0;

   dx = frame_x - pCD->frameInfo.x;
   dy = frame_y - pCD->frameInfo.y;
   if (dx == 0 && dy == 0)
      return;

   MoveClientFrame (pCD, frame_x, frame_y);


   /*  The posted subpanels (attached to the panel, not torn off) move  */
   /*  with it, and stay attached.                                      */

   for (pcdSub = pCD->transientChildren; pcdSub;
        pcdSub = pcdSub->transientSiblings)
   {
      if (!(pcdSub->dtwmBehaviors & DtWM_BEHAVIOR_SUBPANEL))
         continue;

      frame_x = pcdSub->frameInfo.x + dx;
      frame_y = pcdSub->frameInfo.y + dy;
      if (frame_x + (int) pcdSub->frameInfo.width > screen_width)
         frame_x = screen_width - (int) pcdSub->frameInfo.width;
      if (frame_x < 0)
         frame_x = 0;
      if (frame_y + (int) pcdSub->frameInfo.height > screen_height)
         frame_y = screen_height - (int) pcdSub->frameInfo.height;
      if (frame_y < 0)
         frame_y = 0;

      MoveClientFrame (pcdSub, frame_x, frame_y);
      pcdSub->dtwmBehaviors |= DtWM_BEHAVIOR_SUBPANEL;
   }
}




/************************************************************************
 *
 *  WmPanelistAllocate
 *
 ************************************************************************/

Widget
WmPanelistAllocate (Widget    w,
                    XtPointer global_data,
                    XtPointer screen_data)

{
   Boolean create_fp;

   panel.app_name = wmGD.mwmName;
   create_fp = FrontPanelReadDatabases ();
      
   if (create_fp)
   {
#ifdef DT_PERFORMANCE
_DtPerfChkpntMsgSend("Begin creating front panel");
#endif

      FrontPanelCreate (w);

#ifdef DT_PERFORMANCE
_DtPerfChkpntMsgSend("End   creating front panel");
#endif

      panel.global_data = global_data;
      panel.screen_data = screen_data;

      return (panel.form);
   }
   else
      return(NULL);
}




/************************************************************************
 *
 *  WmSubpanelPosted
 *	Add an event handler to catch when the subpanel is torn off.  The 
 *	event handler will then change the subpanel's behavior to remain 
 *	displayed after a control is selected.
 *
 ************************************************************************/

void
WmSubpanelPosted (Display * display,
                  Window  shell_window)

{
   BoxData      * box_data;
   ControlData  * main_control_data;
   SubpanelData * subpanel_data;

   int i, j;


   /*  Loop through the main controls to find the subpanel whose   */
   /*  window matches the parameter shell window and then add the  */
   /*  event handler for tear off behavoir.                        */

   for (i = 0; i < panel.box_data_count; i++)
   {
      box_data = panel.box_data[i];
      
      for (j = 0; j < box_data->control_data_count; j++)
      {
         main_control_data = box_data->control_data[j];

         if (main_control_data->subpanel_data != NULL &&
	     XtWindow (main_control_data->subpanel_data->shell) == shell_window)
	 {
            subpanel_data = main_control_data->subpanel_data;

            subpanel_data->posted_x = XtX (subpanel_data->shell);

            XtAddEventHandler (subpanel_data->shell, StructureNotifyMask, False,
                               (XtEventHandler) SubpanelTornEventHandler,
                               (XtPointer) main_control_data);
            break;
	 }
      }
   }
}

