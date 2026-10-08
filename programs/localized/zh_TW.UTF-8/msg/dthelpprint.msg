$ $XConsortium: dthelpprint.msg /main/3 1995/11/08 14:06:41 rswiston $
$ *************************************<+>*************************************
$ *****************************************************************************
$ **
$ **  File:        dthelpprint.msg
$ **
$ **  Project:     Cde1 Help
$ **
$ **  Description:
$ **  -----------
$ **  This file is the source for the message catalog for dthelpprint
$ **
$ **
$ *****************************************************************************
$ **
$ ** (c) Copyright 1993, 1994 Hewlett-Packard Company
$ ** (c) Copyright 1993, 1994 International Business Machines Corp.
$ ** (c) Copyright 1993, 1994 Sun Microsystems, Inc.
$ ** (c) Copyright 1993, 1994 Unix System Labs, Inc.,a subsidiary of Novell,Inc.
$ **         All Rights reserved
$ **
$ **
$ **     There are two types of messages in this file:
$ **
$ **     1) Messages that appear in printed output
$ **        These messages are the default and they should all be localized.
$ **        These messages are marked with LOCALIZE THESE MESSAGES.
$ **
$ **     2) Messages that should not be localized.
$ **        These messages are marked with DO NOT LOCALIZE THESE MESSAGES.
$ **
$ *****************************************************************************
$ **
$ **
$ **  ------------------------- MODIFICATION RECORD --------------------------
$ *   Major Mods, 7/11/94 for dthelpprint
$ *
$ **  ----------------------- MODIFICATION RECORD END ------------------------
$ *****************************************************************************
$ *************************************<+>*************************************

$ Use the double quote char around all messages
$quote "

$set 2
$ *****  Module: PrintUtil.c *****
$ 

$set 3
$ *****  Module: PrintTopics.c *****
$ 
$ ** LOCALIZE THESE MESSAGES **
1 "%s 錯誤：輔助說明類別是主題，但未指定輔助說明容體\n"
2 "%s 錯誤：無法定位輔助說明容體 %s\n"
3 "%s 錯誤：問題處理輔助說明容體 %s\n"
4 "%s 錯誤：記憶體配置失敗\n"
5 "%s 錯誤：無法取得主題資訊：\n容體 %s，Id 位置 %s\n"
6 "%s 錯誤：無法開啟暫時性檔案 %s\n"
$ String used to replace $SECTNUM when printing the index section
10 "索引"
$ String used to replace $SECTNUM when printing the table of contents section
11 "內容表格"
$ ** DO NOT LOCALIZE THESE MESSAGES **
$ index entry, page number
20 "%s, %d\n"
$ index subentry, page number
21 "     %s, %d\n"
$ location ID associated with the index in the TOC maintained by dthelpprint
30 "__GENERATED-INDEX"
$ string used by can't get a topic title associated with an index subentry
31 ""

$set 4
$ *****  Module: PrintManStrFile.c *****
$ 
$ ** LOCALIZE THESE MESSAGES **
1 "%s 錯誤：輔助說明類別是字串，但未指定字串資料。\n"
2 "%s 錯誤：輔助說明類別是動態字串，但未指定字串資料。\n"
3 "%s 錯誤：輔助說明類別是線上援助頁，但未指定線上援助頁。\n"
4 "%s 錯誤：輔助說明是檔案，但未指定輔助說明檔案。\n"
5 "%s 錯誤：記憶體配置失敗。\n"

$set 5
$ *****  Module: Initialize.c *****
$ 
$ ** LOCALIZE THESE MESSAGES **
$ Messages 1 to 29: dthelpprint usage message
$ **DO NOT LOCALIZE** the command line option names (e.g. -copies).
$ **DO LOCALIZE** the option argument (e.g. number) and description.
1 "dthelpprint － 輔助說明列印程式\n\n"
2 "使用方法： dthelpprint [options]\n"
3 "控制如何列印的選項：\n"
4 "\t-printer printername  欲使用之印表機\n"
5 "\t-copies number        欲列印之檔案數目\n"
6 "\t-outputFile filename  將輸出寫入本檔案\n"
7 "\t-paperSize size       針對分頁器大小之格式內容\n"
$ DO NOT LOCALIZE THE PAPER SIZE NAMES help_papersize_xxx
8 "\t\tsize = { help_papersize_letter|help_papersize_legal|\n"
9 "\t\t         help_papersize_executive|help_papersize_a4|help_papersize_b5}\n"
10 "\t-display displayname  顯示字何處取得資源\n"
11 "\t-name                 取得資源時使用的程式名稱\n"
12 "\t-class                取得資源時使用的等級名稱\n"
13 "\t-xrm resourcestring   額外的資源\n"
14 "控制欲列印之物的選項：\n"
15 "\t-helpType type        輔助說明資料的類別\n"
16 "\t\ttype = 0 (help volume), 1 (string), 2 (man page), 3 (help file)\n"
17 "\t-helpVolume volume    輔助說明容體檔案的完整路徑\n"
18 "\t-locationId location  容體中輔助說明主題的名稱\n"
19 "\t-all                  列印輔助說明容體中所有主題，目錄，和索引\n"
20 "\t-sub                  列印主題 Id 位置與全部子題\n"
21 "\t-one                  列印主題 Id 位置\n"
22 "\t-toc                  列印輔助說明容體目錄\n"
23 "\t-index                列印輔助說明容體索引\n"
24 "\t-frontMatter          列印輔助說明容體前方事項\n"
25 "\t-manPage manpagename  線上支援頁的名稱\n"
26 "\t-stringData string    欲列印的輔助說明本文\n"
27 "\t-helpFile filename    包含輔助說明本文的檔案\n"
28 "\t-jobTitle title       列印工作之標題字串\n"
29 "\t-topicTitle title     輔助說明本文之標題字串\n"
$ Warning and error messages
$ Do NOT localize the default size name help_papersize_letter
40 "%s 警告：不合法的紙張大小 '%s'。使用 help_papersize_letter。\n"
41 "%s 警告：缺少紙張大小、高度或寬度值。使用 help_papersize_letter。\n"
42 "%s 警告：無法開啟顯示 %s\n"
45 "錯誤：無法為暫時性檔案配置記憶體。\n"
$ ** DO NOT LOCALIZE THESE MESSAGES **
$ arg order: directory prefix processid filecnt suffix
50 "%1$s/%2$s%3$d_%4$d%5$s"


$set 6
$ *****  Module: Main.c *****
$ 
$ ** LOCALIZE THESE MESSAGES **
1 "%s 錯誤：輔助說明類別 %d 不對.\n"



