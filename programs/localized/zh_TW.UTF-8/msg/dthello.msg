$ "$XConsortium: dthello.msg /main/3 1995/11/08 14:06:18 rswiston $";

$ *************************************<+>*************************************
$ *****************************************************************************
$ **
$ **  File:        dthello.msg
$ **
$ **  Project:     HP DT login transition program (dthello) 
$ **
$ **  Description:
$ **  -----------
$ **  This file is the source for the message catalog for dthello
$ **
$ **
$ *****************************************************************************
$ **
$ **     (c) Copyright 1990, 1991 Hewlett-Packard Company
$ **     All Rights reserved
$ **
$ **
$ *****************************************************************************
$ *****************************************************************************
$ *************************************<+>*************************************

$ *************************************<L>*************************************
$ **  -----------------GENERAL LOCALIZATION NOTES SECTION---------------------
$ * Comment lines begin with a $ except $set which indicates the start of a 
$ * new set.
$ *
$ * Do not delete any comments; you may add comments for your use.
$ *
$ **  ----------------GENERAL LOCALIZATION NOTES SECTION END------------------
$ *************************************<L>*************************************

$set 2
$    THIS COMMENT FOR DT TEAM ONLY -- dthello.c --
$    Do not translate. Blank message, please ignore
$quote "
1 ""
$quote

$    Message #2 is no longer used. It has been replaced with messag #3
2 啟動\n\
COSE\n\
使用者環境\n\n

$    Default startup message (written before the contents
$    of the default file, /etc/copyright).
3 啟動\n\
一般桌上管理系統環境\n\n


$set 4
$    THIS COMMENT FOR DT TEAM ONLY -- dthello.c --
$    This entire set is for error messages
$    DO NOT TRANSLATE THESE MESSAGES
1 %s: 無法開啟顯示\n
2 %s: 過時必須為正\n
3 %1$s: 允許最大檔案數為 %2$d ， 略過 %3$s\n

$ Messages 4 and 5 are both used when there is a syntax error on
$ the command line invoking dthello
$ 
4 用法： %s [-display <顯示>] [-fg <色彩>] [-bg <色彩>]\\\n
5 [-font <字形>] [-string <訊息>] [-file <名稱>] [-timeout <秒>]\n

$ _DtMessage 6 occurs when an font can't be loaded
$  the likely causes: (1) the font was incorrectly specified
$                     (2) it is not available on this server
6 %1$s: 顯示 %2$s 不知道字形 %3$s\n

7 %1$s：無法開啟檔案 %2$s\n
8 %1$s：無法將背景設定為 %2$s，使用 %3$s。\n
9 %1$s：無法將前景設定為 %2$s」使用 %3$s。\n
10 %s：(分隔的本文行)\n

$set 99
$    DO NOT TRANSLATE THESE MESSAGES
$    DO NOT CHANGE THESE MESSAGES
$    DO NOT LOCALIZE THESE MESSAGES
$    These messages are used for the version information.
$quote "
1 "@(#)version_goes_here"
2 "\n@(#)_DtMessage 目錄來源 $XConsortium: dthello.msg /main/3 1995/11/08 14:06:18 rswiston $:

