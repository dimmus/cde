MAINTAINERCLEANFILES = Makefile.in

# The generator rules below each write several files.  They are grouped
# targets ("&:", GNU make 4.3 or later), so a parallel make runs each
# recipe once.  Older GNU makes have no grouped targets and would run the
# recipe once per output, concurrently: build serially there.
$(if $(filter grouped-target,$(.FEATURES)),,$(eval .NOTPARALLEL:))

.DELETE_ON_ERROR:

appdefaultsdir = $(CDE_INSTALLATION_TOP)/app-defaults/C

ABINCLUDES = -I../include -I../include/ab_private -I../libABil

bin_PROGRAMS = dtbuilder

# Every generated header, so that a parallel make generates them all
# before it compiles anything that includes them.
BUILT_SOURCES = dtbuilder.c dtbuilder.msg dtb_utils.c dtb_utils.h \
		$(PROJFILE_MODULES:=_ui.h)

appdefaults_DATA = Dtbuilder

DTCODEGENCAT = NLSPATH=$(top_builddir)/programs/dtappbuilder/src/abmf/dtcodegen.cat

DTCODEGEN = $(DTCODEGENCAT) ../abmf/dtcodegen

# dtcodegen -merge rewrites the *_stubs.c files and dtbuilder.h, which are
# tracked sources, every time it runs.  Run it in a scratch directory and
# keep only the generated files; the stubs and dtbuilder.h stay as they
# are in the tree (regenerate them on purpose with dtbuilder or
# "make regen-stubs").
DTCODEGEN_SCRATCH = $(SHELL) $(top_srcdir)/build-aux/dtcodegen-scratch \
	"NLSPATH=$(abs_top_builddir)/programs/dtappbuilder/src/abmf/dtcodegen.cat $(abs_top_builddir)/programs/dtappbuilder/src/abmf/dtcodegen"

# dtbuilder.c and dtbuilder.msg are dtbuilder.c.src and dtbuilder.msg.src
# with the generated main code merged in.
DTCODEGEN_MAIN = $(DTCODEGEN_SCRATCH) -i dtbuilder.c.src=dtbuilder.c \
	-i dtbuilder.msg.src=dtbuilder.msg -i dtbuilder.h=dtbuilder.h \
	dtb_utils.c dtb_utils.h dtbuilder.c dtbuilder.msg \
	-- -changed -merge -p dtbuilder.bip -main

PROJFILE_MODULES = about_box appfw attch_ed brws button cgen_env cgen_props \
	cgen_win choice color_chooser combobox conn cpanel custdlg \
	dnd_ed drawp fchooser group help_ed label list mainwin menu \
	menubar message_ed palette panedwin_ed proj revolv scale sep \
	spinbox termp textf textp

# Maintainer target: merge the current .bil files into the tracked stubs.
.PHONY: regen-stubs
regen-stubs:
	$(DTCODEGEN) -changed -merge -p dtbuilder.bip -main
	for m in $(PROJFILE_MODULES); do \
		$(DTCODEGEN) -changed -merge -p dtbuilder.bip $$m.bil || exit 1; \
	done

if BSD
UTILLIB = -lutil
endif

AB_MAKEFILES = \
        Makefile.aix \
        Makefile.sunos \
        Makefile.linux \
        Makefile.freebsd \
        Makefile.netbsd \
        Makefile.openbsd

CLEANFILES = $(AB_MAKEFILES) *.res *_ui.c *_ui.h .dtcodegen.log *.delta dtbuilder.c dtbuilder.msg \
	dtb_utils.c dtb_utils.h Dtbuilder Dtbuilder.tmp

SOURCES.msg = \
                dtbuilder.msg   \
                ../libAButil/libAButil.msg \
                ../libABobjXm/libABobjXm.msg \
                ../libABil/libABil.msg

dtbuilder_LDADD = ../libABil/libABil.a  ../libABobjXm/libABobjXm.a  \
        ../libABobj/libABobj.a ../libAButil/libAButil.a \
        $(LIBWIDGET) $(LIBTERM) $(LIBHELP) $(LIBSVC) \
        $(LIBTT) $(LIBXIN) -lUil $(MRESOURCELIB) $(XTOOLLIB) \
	$(UTILLIB)

dtbuilder_CFLAGS = $(CDE_CFLAGS) $(KEEP_ASSERTS) -DPIXMAP_WORKAROUND $(ABINCLUDES)

dtbuilder_SOURCES = dtb_utils.c \
		    appfw_ui.c \
		    about_box_ui.c \
		    brws_ui.c \
		    color_chooser_ui.c \
		    conn_ui.c \
		    dnd_ed_ui.c \
		    help_ed_ui.c \
		    attch_ed_ui.c \
		    proj_ui.c \
		    revolv_ui.c \
		    button_ui.c \
		    choice_ui.c \
		    combobox_ui.c \
		    cpanel_ui.c \
		    custdlg_ui.c \
		    drawp_ui.c \
		    group_ui.c \
		    label_ui.c \
		    list_ui.c \
		    mainwin_ui.c \
		    menu_ui.c \
		    menubar_ui.c \
		    scale_ui.c \
		    sep_ui.c \
		    spinbox_ui.c \
		    termp_ui.c \
		    textf_ui.c \
		    textp_ui.c \
		    cgen_win_ui.c \
		    cgen_props_ui.c \
		    cgen_env_ui.c \
		    fchooser_ui.c \
		    message_ed_ui.c \
		    panedwin_ed_ui.c \
		    palette_ui.c \
		    appfw_stubs.c \
		    brws_stubs.c \
		    color_chooser_stubs.c \
		    conn_stubs.c \
		    dnd_ed_stubs.c \
		    help_ed_stubs.c \
		    attch_ed_stubs.c \
		    proj_stubs.c \
		    revolv_stubs.c \
		    button_stubs.c \
		    choice_stubs.c \
		    combobox_stubs.c \
		    cpanel_stubs.c \
		    custdlg_stubs.c \
		    drawp_stubs.c \
		    group_stubs.c \
		    label_stubs.c \
		    list_stubs.c \
		    mainwin_stubs.c \
		    menu_stubs.c \
		    menubar_stubs.c \
		    scale_stubs.c \
		    sep_stubs.c \
		    spinbox_stubs.c \
		    termp_stubs.c \
		    textf_stubs.c \
		    textp_stubs.c \
		    cgen_win_stubs.c \
		    cgen_props_stubs.c \
		    cgen_env_stubs.c \
		    fchooser_stubs.c \
		    message_ed_stubs.c \
		    palette_stubs.c \
		    about_box_stubs.c \
		    dtbuilder.c \
		    ab_utils.c \
		    ab_bil.c \
		    ab_dnd.c \
		    ab_globals.c \
		    abobj_align.c \
		    abobj_edit.c \
		    abobj_list.c \
		    abobj_events.c \
		    abobj_layers.c \
		    abobj_layout.c \
		    abobj_menu.c \
		    abobj_move.c \
		    abobj_resize.c \
		    abobj_select.c \
		    abobj_set.c \
		    abobj_util.c \
		    brws.c \
		    brws_find.c \
		    brws_mthds.c \
		    brws_utils.c \
		    cgen_utils.c \
		    conn_drag.c \
		    conn_interpret.c \
		    conn_obj.c \
		    pal.c \
		    pal_create.c \
		    pal_button.c \
		    pal_choice.c \
		    pal_combobox.c \
		    pal_cpanel.c \
		    pal_custdlg.c \
		    pal_drawp.c \
		    pal_fchooser.c \
		    pal_group.c \
		    pal_label.c \
		    pal_list.c \
		    pal_mainwin.c \
		    pal_menu.c \
		    pal_menubar.c \
		    pal_panedwin.c \
		    pal_scale.c \
		    pal_sep.c \
		    pal_spinbox.c \
		    pal_termp.c \
		    pal_textf.c \
		    pal_textp.c \
		    proj.c \
		    proj_utils.c \
		    projP_utils.c \
		    prop.c \
		    prop_items.c \
		    tmode.c \
		    tmodeP.c \
		    ui_msg.c \
		    ui_list.c \
		    ui_util.c \
		    vwr.c \
		    x_util.c



Dtbuilder: Dtbuilder.src
	$(GENCPP) -P -DXCOMM=# $(LOCAL_CPP_DEFINES) $< > $@.tmp && mv -f $@.tmp $@

if SOLARIS

dtb_utils.c + dtb_utils.h + dtbuilder.c + dtbuilder.msg: dtbuilder.bip dtbuilder.h dtbuilder.c.src dtbuilder.msg.src
	$(DTCODEGEN_MAIN)

PROJFILE = dtbuilder

about_box_ui.c + about_box_ui.h: $(PROJFILE).bip about_box.bil
	$(DTCODEGEN_SCRATCH) -i about_box_stubs.c=about_box_stubs.c about_box_ui.c about_box_ui.h \
		-- -changed -merge -p $(PROJFILE).bip about_box.bil

appfw_ui.c + appfw_ui.h: $(PROJFILE).bip appfw.bil
	$(DTCODEGEN_SCRATCH) -i appfw_stubs.c=appfw_stubs.c appfw_ui.c appfw_ui.h \
		-- -changed -merge -p $(PROJFILE).bip appfw.bil

attch_ed_ui.c + attch_ed_ui.h: $(PROJFILE).bip attch_ed.bil
	$(DTCODEGEN_SCRATCH) -i attch_ed_stubs.c=attch_ed_stubs.c attch_ed_ui.c attch_ed_ui.h \
		-- -changed -merge -p $(PROJFILE).bip attch_ed.bil

brws_ui.c + brws_ui.h: $(PROJFILE).bip brws.bil
	$(DTCODEGEN_SCRATCH) -i brws_stubs.c=brws_stubs.c brws_ui.c brws_ui.h \
		-- -changed -merge -p $(PROJFILE).bip brws.bil

button_ui.c + button_ui.h: $(PROJFILE).bip button.bil
	$(DTCODEGEN_SCRATCH) -i button_stubs.c=button_stubs.c button_ui.c button_ui.h \
		-- -changed -merge -p $(PROJFILE).bip button.bil

cgen_env_ui.c + cgen_env_ui.h: $(PROJFILE).bip cgen_env.bil
	$(DTCODEGEN_SCRATCH) -i cgen_env_stubs.c=cgen_env_stubs.c cgen_env_ui.c cgen_env_ui.h \
		-- -changed -merge -p $(PROJFILE).bip cgen_env.bil

cgen_props_ui.c + cgen_props_ui.h: $(PROJFILE).bip cgen_props.bil
	$(DTCODEGEN_SCRATCH) -i cgen_props_stubs.c=cgen_props_stubs.c cgen_props_ui.c cgen_props_ui.h \
		-- -changed -merge -p $(PROJFILE).bip cgen_props.bil

cgen_win_ui.c + cgen_win_ui.h: $(PROJFILE).bip cgen_win.bil
	$(DTCODEGEN_SCRATCH) -i cgen_win_stubs.c=cgen_win_stubs.c cgen_win_ui.c cgen_win_ui.h \
		-- -changed -merge -p $(PROJFILE).bip cgen_win.bil

choice_ui.c + choice_ui.h: $(PROJFILE).bip choice.bil
	$(DTCODEGEN_SCRATCH) -i choice_stubs.c=choice_stubs.c choice_ui.c choice_ui.h \
		-- -changed -merge -p $(PROJFILE).bip choice.bil

color_chooser_ui.c + color_chooser_ui.h: $(PROJFILE).bip color_chooser.bil
	$(DTCODEGEN_SCRATCH) -i color_chooser_stubs.c=color_chooser_stubs.c color_chooser_ui.c color_chooser_ui.h \
		-- -changed -merge -p $(PROJFILE).bip color_chooser.bil

combobox_ui.c + combobox_ui.h: $(PROJFILE).bip combobox.bil
	$(DTCODEGEN_SCRATCH) -i combobox_stubs.c=combobox_stubs.c combobox_ui.c combobox_ui.h \
		-- -changed -merge -p $(PROJFILE).bip combobox.bil

conn_ui.c + conn_ui.h: $(PROJFILE).bip conn.bil
	$(DTCODEGEN_SCRATCH) -i conn_stubs.c=conn_stubs.c conn_ui.c conn_ui.h \
		-- -changed -merge -p $(PROJFILE).bip conn.bil

cpanel_ui.c + cpanel_ui.h: $(PROJFILE).bip cpanel.bil
	$(DTCODEGEN_SCRATCH) -i cpanel_stubs.c=cpanel_stubs.c cpanel_ui.c cpanel_ui.h \
		-- -changed -merge -p $(PROJFILE).bip cpanel.bil

custdlg_ui.c + custdlg_ui.h: $(PROJFILE).bip custdlg.bil
	$(DTCODEGEN_SCRATCH) -i custdlg_stubs.c=custdlg_stubs.c custdlg_ui.c custdlg_ui.h \
		-- -changed -merge -p $(PROJFILE).bip custdlg.bil

dnd_ed_ui.c + dnd_ed_ui.h: $(PROJFILE).bip dnd_ed.bil
	$(DTCODEGEN_SCRATCH) -i dnd_ed_stubs.c=dnd_ed_stubs.c dnd_ed_ui.c dnd_ed_ui.h \
		-- -changed -merge -p $(PROJFILE).bip dnd_ed.bil

drawp_ui.c + drawp_ui.h: $(PROJFILE).bip drawp.bil
	$(DTCODEGEN_SCRATCH) -i drawp_stubs.c=drawp_stubs.c drawp_ui.c drawp_ui.h \
		-- -changed -merge -p $(PROJFILE).bip drawp.bil

fchooser_ui.c + fchooser_ui.h: $(PROJFILE).bip fchooser.bil
	$(DTCODEGEN_SCRATCH) -i fchooser_stubs.c=fchooser_stubs.c fchooser_ui.c fchooser_ui.h \
		-- -changed -merge -p $(PROJFILE).bip fchooser.bil

group_ui.c + group_ui.h: $(PROJFILE).bip group.bil
	$(DTCODEGEN_SCRATCH) -i group_stubs.c=group_stubs.c group_ui.c group_ui.h \
		-- -changed -merge -p $(PROJFILE).bip group.bil

help_ed_ui.c + help_ed_ui.h: $(PROJFILE).bip help_ed.bil
	$(DTCODEGEN_SCRATCH) -i help_ed_stubs.c=help_ed_stubs.c help_ed_ui.c help_ed_ui.h \
		-- -changed -merge -p $(PROJFILE).bip help_ed.bil

label_ui.c + label_ui.h: $(PROJFILE).bip label.bil
	$(DTCODEGEN_SCRATCH) -i label_stubs.c=label_stubs.c label_ui.c label_ui.h \
		-- -changed -merge -p $(PROJFILE).bip label.bil

list_ui.c + list_ui.h: $(PROJFILE).bip list.bil
	$(DTCODEGEN_SCRATCH) -i list_stubs.c=list_stubs.c list_ui.c list_ui.h \
		-- -changed -merge -p $(PROJFILE).bip list.bil

mainwin_ui.c + mainwin_ui.h: $(PROJFILE).bip mainwin.bil
	$(DTCODEGEN_SCRATCH) -i mainwin_stubs.c=mainwin_stubs.c mainwin_ui.c mainwin_ui.h \
		-- -changed -merge -p $(PROJFILE).bip mainwin.bil

menu_ui.c + menu_ui.h: $(PROJFILE).bip menu.bil
	$(DTCODEGEN_SCRATCH) -i menu_stubs.c=menu_stubs.c menu_ui.c menu_ui.h \
		-- -changed -merge -p $(PROJFILE).bip menu.bil

menubar_ui.c + menubar_ui.h: $(PROJFILE).bip menubar.bil
	$(DTCODEGEN_SCRATCH) -i menubar_stubs.c=menubar_stubs.c menubar_ui.c menubar_ui.h \
		-- -changed -merge -p $(PROJFILE).bip menubar.bil

message_ed_ui.c + message_ed_ui.h: $(PROJFILE).bip message_ed.bil
	$(DTCODEGEN_SCRATCH) -i message_ed_stubs.c=message_ed_stubs.c message_ed_ui.c message_ed_ui.h \
		-- -changed -merge -p $(PROJFILE).bip message_ed.bil

palette_ui.c + palette_ui.h: $(PROJFILE).bip palette.bil
	$(DTCODEGEN_SCRATCH) -i palette_stubs.c=palette_stubs.c palette_ui.c palette_ui.h \
		-- -changed -merge -p $(PROJFILE).bip palette.bil

panedwin_ed_ui.c + panedwin_ed_ui.h: $(PROJFILE).bip panedwin_ed.bil
	$(DTCODEGEN_SCRATCH) -i panedwin_ed_stubs.c=panedwin_ed_stubs.c panedwin_ed_ui.c panedwin_ed_ui.h \
		-- -changed -merge -p $(PROJFILE).bip panedwin_ed.bil

proj_ui.c + proj_ui.h: $(PROJFILE).bip proj.bil
	$(DTCODEGEN_SCRATCH) -i proj_stubs.c=proj_stubs.c proj_ui.c proj_ui.h \
		-- -changed -merge -p $(PROJFILE).bip proj.bil

revolv_ui.c + revolv_ui.h: $(PROJFILE).bip revolv.bil
	$(DTCODEGEN_SCRATCH) -i revolv_stubs.c=revolv_stubs.c revolv_ui.c revolv_ui.h \
		-- -changed -merge -p $(PROJFILE).bip revolv.bil

scale_ui.c + scale_ui.h: $(PROJFILE).bip scale.bil
	$(DTCODEGEN_SCRATCH) -i scale_stubs.c=scale_stubs.c scale_ui.c scale_ui.h \
		-- -changed -merge -p $(PROJFILE).bip scale.bil

sep_ui.c + sep_ui.h: $(PROJFILE).bip sep.bil
	$(DTCODEGEN_SCRATCH) -i sep_stubs.c=sep_stubs.c sep_ui.c sep_ui.h \
		-- -changed -merge -p $(PROJFILE).bip sep.bil

spinbox_ui.c + spinbox_ui.h: $(PROJFILE).bip spinbox.bil
	$(DTCODEGEN_SCRATCH) -i spinbox_stubs.c=spinbox_stubs.c spinbox_ui.c spinbox_ui.h \
		-- -changed -merge -p $(PROJFILE).bip spinbox.bil

termp_ui.c + termp_ui.h: $(PROJFILE).bip termp.bil
	$(DTCODEGEN_SCRATCH) -i termp_stubs.c=termp_stubs.c termp_ui.c termp_ui.h \
		-- -changed -merge -p $(PROJFILE).bip termp.bil

textf_ui.c + textf_ui.h: $(PROJFILE).bip textf.bil
	$(DTCODEGEN_SCRATCH) -i textf_stubs.c=textf_stubs.c textf_ui.c textf_ui.h \
		-- -changed -merge -p $(PROJFILE).bip textf.bil

textp_ui.c + textp_ui.h: $(PROJFILE).bip textp.bil
	$(DTCODEGEN_SCRATCH) -i textp_stubs.c=textp_stubs.c textp_ui.c textp_ui.h \
		-- -changed -merge -p $(PROJFILE).bip textp.bil

else

dtb_utils.c dtb_utils.h dtbuilder.c dtbuilder.msg &: dtbuilder.bip dtbuilder.h dtbuilder.c.src dtbuilder.msg.src
	$(DTCODEGEN_MAIN)

PROJFILE = dtbuilder

about_box_ui.c about_box_ui.h &: $(PROJFILE).bip about_box.bil
	$(DTCODEGEN_SCRATCH) -i about_box_stubs.c=about_box_stubs.c about_box_ui.c about_box_ui.h \
		-- -changed -merge -p $(PROJFILE).bip about_box.bil

appfw_ui.c appfw_ui.h &: $(PROJFILE).bip appfw.bil
	$(DTCODEGEN_SCRATCH) -i appfw_stubs.c=appfw_stubs.c appfw_ui.c appfw_ui.h \
		-- -changed -merge -p $(PROJFILE).bip appfw.bil

attch_ed_ui.c attch_ed_ui.h &: $(PROJFILE).bip attch_ed.bil
	$(DTCODEGEN_SCRATCH) -i attch_ed_stubs.c=attch_ed_stubs.c attch_ed_ui.c attch_ed_ui.h \
		-- -changed -merge -p $(PROJFILE).bip attch_ed.bil

brws_ui.c brws_ui.h &: $(PROJFILE).bip brws.bil
	$(DTCODEGEN_SCRATCH) -i brws_stubs.c=brws_stubs.c brws_ui.c brws_ui.h \
		-- -changed -merge -p $(PROJFILE).bip brws.bil

button_ui.c button_ui.h &: $(PROJFILE).bip button.bil
	$(DTCODEGEN_SCRATCH) -i button_stubs.c=button_stubs.c button_ui.c button_ui.h \
		-- -changed -merge -p $(PROJFILE).bip button.bil

cgen_env_ui.c cgen_env_ui.h &: $(PROJFILE).bip cgen_env.bil
	$(DTCODEGEN_SCRATCH) -i cgen_env_stubs.c=cgen_env_stubs.c cgen_env_ui.c cgen_env_ui.h \
		-- -changed -merge -p $(PROJFILE).bip cgen_env.bil

cgen_props_ui.c cgen_props_ui.h &: $(PROJFILE).bip cgen_props.bil
	$(DTCODEGEN_SCRATCH) -i cgen_props_stubs.c=cgen_props_stubs.c cgen_props_ui.c cgen_props_ui.h \
		-- -changed -merge -p $(PROJFILE).bip cgen_props.bil

cgen_win_ui.c cgen_win_ui.h &: $(PROJFILE).bip cgen_win.bil
	$(DTCODEGEN_SCRATCH) -i cgen_win_stubs.c=cgen_win_stubs.c cgen_win_ui.c cgen_win_ui.h \
		-- -changed -merge -p $(PROJFILE).bip cgen_win.bil

choice_ui.c choice_ui.h &: $(PROJFILE).bip choice.bil
	$(DTCODEGEN_SCRATCH) -i choice_stubs.c=choice_stubs.c choice_ui.c choice_ui.h \
		-- -changed -merge -p $(PROJFILE).bip choice.bil

color_chooser_ui.c color_chooser_ui.h &: $(PROJFILE).bip color_chooser.bil
	$(DTCODEGEN_SCRATCH) -i color_chooser_stubs.c=color_chooser_stubs.c color_chooser_ui.c color_chooser_ui.h \
		-- -changed -merge -p $(PROJFILE).bip color_chooser.bil

combobox_ui.c combobox_ui.h &: $(PROJFILE).bip combobox.bil
	$(DTCODEGEN_SCRATCH) -i combobox_stubs.c=combobox_stubs.c combobox_ui.c combobox_ui.h \
		-- -changed -merge -p $(PROJFILE).bip combobox.bil

conn_ui.c conn_ui.h &: $(PROJFILE).bip conn.bil
	$(DTCODEGEN_SCRATCH) -i conn_stubs.c=conn_stubs.c conn_ui.c conn_ui.h \
		-- -changed -merge -p $(PROJFILE).bip conn.bil

cpanel_ui.c cpanel_ui.h &: $(PROJFILE).bip cpanel.bil
	$(DTCODEGEN_SCRATCH) -i cpanel_stubs.c=cpanel_stubs.c cpanel_ui.c cpanel_ui.h \
		-- -changed -merge -p $(PROJFILE).bip cpanel.bil

custdlg_ui.c custdlg_ui.h &: $(PROJFILE).bip custdlg.bil
	$(DTCODEGEN_SCRATCH) -i custdlg_stubs.c=custdlg_stubs.c custdlg_ui.c custdlg_ui.h \
		-- -changed -merge -p $(PROJFILE).bip custdlg.bil

dnd_ed_ui.c dnd_ed_ui.h &: $(PROJFILE).bip dnd_ed.bil
	$(DTCODEGEN_SCRATCH) -i dnd_ed_stubs.c=dnd_ed_stubs.c dnd_ed_ui.c dnd_ed_ui.h \
		-- -changed -merge -p $(PROJFILE).bip dnd_ed.bil

drawp_ui.c drawp_ui.h &: $(PROJFILE).bip drawp.bil
	$(DTCODEGEN_SCRATCH) -i drawp_stubs.c=drawp_stubs.c drawp_ui.c drawp_ui.h \
		-- -changed -merge -p $(PROJFILE).bip drawp.bil

fchooser_ui.c fchooser_ui.h &: $(PROJFILE).bip fchooser.bil
	$(DTCODEGEN_SCRATCH) -i fchooser_stubs.c=fchooser_stubs.c fchooser_ui.c fchooser_ui.h \
		-- -changed -merge -p $(PROJFILE).bip fchooser.bil

group_ui.c group_ui.h &: $(PROJFILE).bip group.bil
	$(DTCODEGEN_SCRATCH) -i group_stubs.c=group_stubs.c group_ui.c group_ui.h \
		-- -changed -merge -p $(PROJFILE).bip group.bil

help_ed_ui.c help_ed_ui.h &: $(PROJFILE).bip help_ed.bil
	$(DTCODEGEN_SCRATCH) -i help_ed_stubs.c=help_ed_stubs.c help_ed_ui.c help_ed_ui.h \
		-- -changed -merge -p $(PROJFILE).bip help_ed.bil

label_ui.c label_ui.h &: $(PROJFILE).bip label.bil
	$(DTCODEGEN_SCRATCH) -i label_stubs.c=label_stubs.c label_ui.c label_ui.h \
		-- -changed -merge -p $(PROJFILE).bip label.bil

list_ui.c list_ui.h &: $(PROJFILE).bip list.bil
	$(DTCODEGEN_SCRATCH) -i list_stubs.c=list_stubs.c list_ui.c list_ui.h \
		-- -changed -merge -p $(PROJFILE).bip list.bil

mainwin_ui.c mainwin_ui.h &: $(PROJFILE).bip mainwin.bil
	$(DTCODEGEN_SCRATCH) -i mainwin_stubs.c=mainwin_stubs.c mainwin_ui.c mainwin_ui.h \
		-- -changed -merge -p $(PROJFILE).bip mainwin.bil

menu_ui.c menu_ui.h &: $(PROJFILE).bip menu.bil
	$(DTCODEGEN_SCRATCH) -i menu_stubs.c=menu_stubs.c menu_ui.c menu_ui.h \
		-- -changed -merge -p $(PROJFILE).bip menu.bil

menubar_ui.c menubar_ui.h &: $(PROJFILE).bip menubar.bil
	$(DTCODEGEN_SCRATCH) -i menubar_stubs.c=menubar_stubs.c menubar_ui.c menubar_ui.h \
		-- -changed -merge -p $(PROJFILE).bip menubar.bil

message_ed_ui.c message_ed_ui.h &: $(PROJFILE).bip message_ed.bil
	$(DTCODEGEN_SCRATCH) -i message_ed_stubs.c=message_ed_stubs.c message_ed_ui.c message_ed_ui.h \
		-- -changed -merge -p $(PROJFILE).bip message_ed.bil

palette_ui.c palette_ui.h &: $(PROJFILE).bip palette.bil
	$(DTCODEGEN_SCRATCH) -i palette_stubs.c=palette_stubs.c palette_ui.c palette_ui.h \
		-- -changed -merge -p $(PROJFILE).bip palette.bil

panedwin_ed_ui.c panedwin_ed_ui.h &: $(PROJFILE).bip panedwin_ed.bil
	$(DTCODEGEN_SCRATCH) -i panedwin_ed_stubs.c=panedwin_ed_stubs.c panedwin_ed_ui.c panedwin_ed_ui.h \
		-- -changed -merge -p $(PROJFILE).bip panedwin_ed.bil

proj_ui.c proj_ui.h &: $(PROJFILE).bip proj.bil
	$(DTCODEGEN_SCRATCH) -i proj_stubs.c=proj_stubs.c proj_ui.c proj_ui.h \
		-- -changed -merge -p $(PROJFILE).bip proj.bil

revolv_ui.c revolv_ui.h &: $(PROJFILE).bip revolv.bil
	$(DTCODEGEN_SCRATCH) -i revolv_stubs.c=revolv_stubs.c revolv_ui.c revolv_ui.h \
		-- -changed -merge -p $(PROJFILE).bip revolv.bil

scale_ui.c scale_ui.h &: $(PROJFILE).bip scale.bil
	$(DTCODEGEN_SCRATCH) -i scale_stubs.c=scale_stubs.c scale_ui.c scale_ui.h \
		-- -changed -merge -p $(PROJFILE).bip scale.bil

sep_ui.c sep_ui.h &: $(PROJFILE).bip sep.bil
	$(DTCODEGEN_SCRATCH) -i sep_stubs.c=sep_stubs.c sep_ui.c sep_ui.h \
		-- -changed -merge -p $(PROJFILE).bip sep.bil

spinbox_ui.c spinbox_ui.h &: $(PROJFILE).bip spinbox.bil
	$(DTCODEGEN_SCRATCH) -i spinbox_stubs.c=spinbox_stubs.c spinbox_ui.c spinbox_ui.h \
		-- -changed -merge -p $(PROJFILE).bip spinbox.bil

termp_ui.c termp_ui.h &: $(PROJFILE).bip termp.bil
	$(DTCODEGEN_SCRATCH) -i termp_stubs.c=termp_stubs.c termp_ui.c termp_ui.h \
		-- -changed -merge -p $(PROJFILE).bip termp.bil

textf_ui.c textf_ui.h &: $(PROJFILE).bip textf.bil
	$(DTCODEGEN_SCRATCH) -i textf_stubs.c=textf_stubs.c textf_ui.c textf_ui.h \
		-- -changed -merge -p $(PROJFILE).bip textf.bil

textp_ui.c textp_ui.h &: $(PROJFILE).bip textp.bil
	$(DTCODEGEN_SCRATCH) -i textp_stubs.c=textp_stubs.c textp_ui.c textp_ui.h \
		-- -changed -merge -p $(PROJFILE).bip textp.bil
endif
