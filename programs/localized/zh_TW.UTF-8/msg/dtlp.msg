
$ $ "$XConsortium: dtlp.msg /main/3 1995/11/08 14:07:31 rswiston $";

$ *************************************<+>*************************************
$ *****************************************************************************
$ **
$ **  File:        dtlp.msg
$ **
$ **  Project:     DT printer utility (dtlp)
$ **
$ **  Description:
$ **  -----------
$ **  This file is the source for the message catalog for dtlp
$ **
$ **
$ *****************************************************************************
$ **
$ ** (c) Copyright 1990, 1992, 1993, 1994 Hewlett-Packard Company
$ ** (c) Copyright 1994 International Business Machines Corp.
$ ** (c) Copyright 1994 Sun Microsystems, Inc.
$ ** (c) Copyright 1994 Novell, Inc.
$ **
$ **     All Rights reserved
$ **
$ *****************************************************************************
$ *************************************<+>*************************************

$ *************************************<L>*************************************
$ **  -----------------GENERAL LOCALIZATION NOTES SECTION---------------------
$ * Comment lines begin with a $ except $set which indicates the start of a 
$ * new set.
$ *
$ * Do not delete any comments; you may add comments for your use.
$ *
$ * You don't need to localize the following sets:
$ *
$ * Please localize the following sets:
$ *  sets 1
$ **  ----------------GENERAL LOCALIZATION NOTES SECTION END------------------
$ *************************************<L>*************************************

$quote "

$set 1
$ 
$  -- The first group of messages all appear on the Print dialog --
$ 


$  This is the message that appears in the "Printer:" field when no
$  printer name has been specified.
10 "內定印表機"

$  This is the label that appears in the title bar of the print dialog.
20 "列印"

$  This is the label for the field indicating the file that will be printed.
30 "檔案："

$  This is the label for the field indicating the name of the printer.
40 "印表機："

$  This is the message that will appear in the "Printer:" field if no printer
$  has been specified.
50 "內定印表機"

$  This is the label for the field indicating the number of copies to print.
60 "份數："

$  This is the label for the field indicating the title for the banner page.
70 "表頭標題頁標題："

$  This is the label for the field indicating if page numbers are to be added.
80 "列印頁碼"

$  This is thef label for the field indicating additional options for the
$  "lp" command.
90 "列印命令選項："

$  This is the label on the left button. Pushing this button causes the
$  dialog to disappear and the print operation to start.
100 "列印"

$  This is the label on the center button. Pushing this button causes the
$  dialog to disappear and cancels the print operation.
110 "取消"

$  This is the label on the right button. Pushing this button causes a
$  help dialog to be posted.
120 "輔助說明"

$ 
$  -- The following are error messages. --
$ 

$  This message is posted when there is a problem with the file specified. 
$  The file name will be appended to the message. One additional message
$  from the list below will also be included in the dialog to provide
$  more details.
130 "抱歉--無法列印："

$  This message is added to the error dialog when the file is not printable 
$  or does not exist.
132 "這是一個無法列印的檔案，或此檔案不存在。"

$  This message is added to the error dialog when the file is not readable 
$  by the user invoking dtlp.
134 "您沒有讀取該檔案的許可權。"

$  This message is added to the error dialog when there is a general failure 
$  in setting up to print.
136 "請檢查這個檔案是否存在。"

$  This is the label that appears in the title bar of the error dialog.
160 "列印資訊"



$set 99
$    DO NOT TRANSLATE THESE MESSAGES
$    DO NOT CHANGE THESE MESSAGES
$    DO NOT LOCALIZE THESE MESSAGES
$    These messages are used for the version information.
1 "@(#)version_goes_here"
2 "\n@(#)_DtMessage catalog source $XConsortium: dtlp.msg /main/3 1995/11/08 14:07:31 rswiston $"

