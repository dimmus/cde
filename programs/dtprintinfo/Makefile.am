MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = dtprintinfo

AM_CPPFLAGS = -I./UI -I./libUI -I./util -I./libUI/MotifUI -I./objects	\
	-I./objects/PrintObj -DKORNSHELL=\"$(KSH)\"

dtprintinfo_LDADD = $(DTCLIENTLIBS) $(XTOOLLIB)

UTIL_SRC = util/Invoke.C util/Invoke.h util/Process.C util/Process.h

LIBUI_SRC = libUI/BaseUI.C libUI/BaseUI.h libUI/MotifUI/Application.C	\
	libUI/MotifUI/LabelObj.C libUI/MotifUI/Application.h		\
	libUI/MotifUI/LabelObj.h libUI/MotifUI/Button.C			\
	libUI/MotifUI/MainWindow.C libUI/MotifUI/Button.h		\
	libUI/MotifUI/MainWindow.h libUI/MotifUI/ComboBoxObj.C		\
	libUI/MotifUI/MenuBar.C libUI/MotifUI/ComboBoxObj.h		\
	libUI/MotifUI/MenuBar.h libUI/MotifUI/Container.C		\
	libUI/MotifUI/Menu.C libUI/MotifUI/Container.h			\
	libUI/MotifUI/Menu.h libUI/MotifUI/Dialog.C			\
	libUI/MotifUI/MotifThread.C libUI/MotifUI/Dialog.h		\
	libUI/MotifUI/MotifThread.h libUI/MotifUI/DtDND.C		\
	libUI/MotifUI/MotifUI.C libUI/MotifUI/DtDND.h			\
	libUI/MotifUI/MotifUI.h libUI/MotifUI/Group.C			\
	libUI/MotifUI/Prompt.C libUI/MotifUI/Group.h			\
	libUI/MotifUI/Prompt.h libUI/MotifUI/HelpSystem.C		\
	libUI/MotifUI/ScaleObj.C libUI/MotifUI/HelpSystem.h		\
	libUI/MotifUI/ScaleObj.h libUI/MotifUI/Icon.h			\
	libUI/MotifUI/Sep.C libUI/MotifUI/IconObj.C			\
	libUI/MotifUI/Sep.h libUI/MotifUI/IconObj.h			\
	libUI/MotifUI/WorkArea.h libUI/MotifUI/IconP.h			\
	libUI/MotifUI/WorkAreaP.h libUI/MotifUI/Debug.c			\
	libUI/MotifUI/Icon.c libUI/MotifUI/WorkArea.c

OBJ_SRC = objects/BaseObj.C objects/BaseObj.h				\
	objects/PrintObj/ParseJobs.C objects/PrintObj/PrintSubSys.C	\
	objects/PrintObj/ParseJobs.h objects/PrintObj/PrintSubSys.h	\
	objects/PrintObj/PrintJob.C objects/PrintObj/Queue.C		\
	objects/PrintObj/PrintJob.h objects/PrintObj/Queue.h

UI_SRC = UI/DtActions.C UI/DtFindD.h UI/DtProps.C UI/DtSetModList.h	\
	UI/DtActions.h UI/DtFindSet.C UI/DtProps.h UI/DtSetPref.C	\
	UI/DtApp.C UI/DtFindSet.h UI/DtPrtJobIcon.C UI/DtSetPref.h	\
	UI/DtApp.h UI/DtMainW.C UI/DtPrtJobIcon.h UI/DtWorkArea.C	\
	UI/DtDetailsLabel.C UI/DtMainW.h UI/DtPrtProps.C		\
	UI/DtWorkArea.h UI/DtDetailsLabel.h UI/DtPrinterIcon.C		\
	UI/DtPrtProps.h UI/DtFindD.C UI/DtPrinterIcon.h			\
	UI/DtSetModList.C


dtprintinfo_SOURCES = $(UTIL_SRC) $(LIBUI_SRC) $(OBJ_SRC) $(UI_SRC)	\
	DtPrintinfo.C dtprintinfomsg.h

# special processing for message files
SYM2NUM_CMD = CPP=$(GENCPP) $(KSH) sym2num

BUILT_SOURCES = dtprintinfo_cat.h dtprintinfo_msg.h dtprintinfo.msg
CLEANFILES = $(BUILT_SOURCES)

# we set this up as an empty target to please make.  The commands for
# the dtprintinfo.msg target will generate it.  We do it this way so
# that this module can be built in parallel without causing corruption
# in the dtprintinfo_msg.h file.  Otherwise an attempt to call
# SYM2NUM_CMD twice will occur during multi-core builds if it is
# listed as the second target depending on dtprintinfo.msg.src along
# with dtprintinfo.msg.
dtprintinfo_msg.h:

dtprintinfo.msg: dtprintinfo.msg.src
	$(RM) dtprintinfo_msg.h dtprintinfo.msg
	$(SYM2NUM_CMD) dtprintinfo dtprintinfo.msg.src > dtprintinfo.msg

dtprintinfo_cat.h: dtprintinfo.msg.src
	$(RM) dtprintinfo_cat.h
	$(AWK) -f msg.awk dtprintinfo.msg.src > dtprintinfo_cat.h

