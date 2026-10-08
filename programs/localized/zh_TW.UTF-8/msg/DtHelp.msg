$ *************************************<+>*************************************
$ *****************************************************************************
$ **
$ **  File:        dthelp.msg
$ **
$ **  Project:     CDE 1.0 Help
$ **
$ **  Description:
$ **  -----------
$ **  This file is the source for the message catalog for DtHelp 
$ **
$ **
$ *****************************************************************************
$ **
$ **     (c) Copyright 1990 Hewlett-Packard Company
$ **         All Rights reserved
$ **
$ **
$ **     There are two types of messages in this file:
$ **
$ **     1) Messages that appear in dialogs or are displayed to the user.
$ **        These messages are the default and they should ALL BE LOCALIZED.
$ **        Note that these messages do NOT have any identification.
$ **
$ **     2) Messages that should not be localized.
$ **        These messages are marked with DO NOT TRANSLATE THESE MESSAGES.
$ **
$ *****************************************************************************
$ **
$ **
$ **  ------------------------- MODIFICATION RECORD --------------------------
$ *   Major Mods, 3/26/92 in preperation for the NLS package release on the 
$ *               27th.  (Mike Wilson).
$ *
$ *   Major Mods, 6/10/94 in prep for Cde Message Catalog Freeze
$ *
$ **  ----------------------- MODIFICATION RECORD END ------------------------
$ *****************************************************************************
$ *************************************<+>*************************************

$set 2
$ *****  Module: HelpDialog.c  and a few from HelpQuickD.c *****
$ 
$ These Messages appear in the pulldown menus for Cde Help General Help
$ dialog widget.
$ 
$ They come in pairs where the first is the menu name (e.g. 1 Print...) and the
$ second is the mnemonic (e.g. 2 P).  
$ 
$ Messages 1 through 6 are the pulldown menus for the File menu.
1 列印(P)...
2 P
3 關閉(C)
4 C
5 開新視窗(W)...
6 W
$ Messages 7 through 8 are the pulldown menus for the Edit menu.
7 複製(C)
8 C 
$ Messages 9 through 10 are the pulldown menus for the Search menu.
9 索引(I)...
10 I
$ Messages 11 through 16 are the pulldown menus for the Navigate menu.
11 起始主題(T)
12 T
13 反向搜尋(B)
14 B
15 記錄(H)...
16 H
$ Messages 17 through 28 are the pulldown menus for the Help menu.
17 概觀(v) - 使用輔助說明
18 v
19 目錄(C)
20 C
21 工作(T)
22 T
23 參照(R)
24 R
25 滑鼠和鍵盤(M)
26 M
27 關於輔助說明(A)
28 A
$ 
$ Messages 29 through 36 are the menu labels and mnemonics for for the 
$ Help Dialog.
29 檔案(F)
30 F
31 搜尋(S)
32 S
33 尋航(N)
34 N
35 輔助說明(H)
36 H
37 編輯(E)
38 E
$ 
$ 
$ >>>>> Skip #39 through #49
$ 
$ Messages 50 through 55 are used to display formatting errors within the
$ helps dialog and quick help dialog widgets.
$ 
50 字串資料無法格式化。
51 動態自串資料無法格式化。
52 線上援助頁無法格式化。您所需要的線上援助頁可能不存在，或是\
已損毀。
53 文字檔案資料無法格式化。您所需要的線上援助頁可能不存在，或\
是已損毀。
54 輔助說明主題無法格式化。
55 右列 ID 位置不存在：
$ 
$ Messages 56 and 67 are displayed when a hypertext link type is selected but
$ not supported within the current application.
$ 
56 這個應用程式不支援您所選取的超文字連結。
57 這個應用程式不支援連結線上援助頁功能。
$ 
$ Messages 58 through 62  are used when bad or null help volumes or 
$ locationId's are used.
$ 
58 您需要的線上輔助說明可能沒有安裝，或未位於適當的搜尋路徑上。有關安裝線上輔助說明的資訊，請參考產品的說明文件。\n
59 未指定 ID 位置。
60 未指定輔助說明容體。
61 輔助說明容體：
62 ID 位置：
$ 
$ >>>>> Skip #63 through #69
$ 
$ 
$ Message 70 is used when a null value for DtNtopicTitle is used.
$ 
70 此主題標題不存在。
$ 
$ $ New messages added after early msg cat freeze; not in order
80 指定的輔助說明類型無效。



$set 3
$ ****  Module: History.c ****
$ These messages are used as labels for the History Dialog.
1 輔助說明 - 記錄瀏覽程式
2 瀏覽過的輔助說明容體：
3 關閉
4 輔助說明
5 輔助說明檔案
6 線上援助頁
7 輔助說明訊息
8 瀏覽過的輔助說明主題：
9 瀏覽過的輔助說明檔案：
10 瀏覽過的線上援助頁：
11 瀏覽過的輔助說明訊息主題：


$set 4
$ *****  Module: Print.c  *****
$ These messages are used as labels for the Print Dialog.
$ 1: dialog title
1 輔助說明 - 列印
$ 10-12: standard printer dialog labels
10 印表機：
11 份數：
12 表頭標題頁標題：
$ 20-25: paper size label and sizes
20 紙張大小：
21 Letter 8.5 x 11 in
22 Legal 8.5 x 14 in
23 Executive 7.25 x 10.5 in
24 A4 210 x 297 mm
25 B5 182 x 257 mm
$ 30-33: help volume-specific printing options
30 要列印的主題
31 現行主題
32 現行主題與次層主題
33 目錄與索引
34 容體中的所有主題
$ 40-42: buttons at bottom of dialog
40 列印
41 取消
42 輔助說明
$ These messages are used to describe the kind of content to be printed
$ 50 "Help Volume: "
50 輔助說明容體：
51 輔助說明訊息：
52 輔助說明檔案
$ 53 "Manual Page: "
53 線上援助頁：
54 錯誤訊息
$ Don't translate message 60 unless it is not an empty string
$ This is the default value that goes in the Printer field
$ 60 " "
$quote "
60 ""
$quote

$ Don't translate message 61
$ This is the default value that goes in the Copies field
61 1


$set 5 
$ *****  Module: GlobSearch.c  *****
$ Message 1 is the label used to label the dialog
1 輔助說明 - 索引搜尋
2 搜尋
3 現行容體
$ DO NOT TRANSLATE MESSAGE 4; it is no longer used
$ " (%s)"
4  (%s)
5 所有容體
6 選取的容體
7 選取容體...
8 顯示
9 完整索引
10 輸入資料：
11 前往
12 開始搜尋
13 繼續搜尋
14 停止搜尋
15 關閉
16 輔助說明
17 掃描容體... 找到 %d 個
18 搜尋中... 剩餘容體：%d 個
19 找不到索引項目
20 完整索引
21 輸入資料 "%s"
22 未選取容體
23 從上方選取搜尋選項
$ Do NOT translate messages 25-31
$quote "
$ " "
25 " "
26 "+"
27 "-"
$ "  +"
28 "  +"
$ "  -"
29 "  -"
$ 30 "       "
30 "       "
$ 31 "         "
31 "         "
$quote
$ DO translate messages 32 and above
$ kind of topic messages
32 Help Message
33 輔助說明檔案
34 援助說明頁
35 不明的格式
$ specialized title of Search Volume Selection dialog
40 輔助說明 - 搜尋容體選擇
$ additional messages (late addition caused this ordering)
50 找不到索引項目。請嘗試搜尋所有容體。
51 找不到索引項目。請嘗試使用其他關鍵字。
$ additional messages (later addition caused this ordering)
$ These messages are the braces that go around the current volume name
$ in the index search dialog.  They are kept in the message catalog
$ in case they need to be localized.
60 (
61 )


$set 6
$ *****  Module: HelpAccess.c  *****
$ These messages are used for the Help on Help dialog in Cde Help widgets.
1 有關輔助說明的輔助說明 
2 列印 ...
3 關閉
4 這個應用程式不支援「有關輔助說明的輔助說明」功能!
5 反向搜尋
6 輔助說明 ...


$set 7
$ *****  Module: Main.c & Util.c & (dthelpview CDE 1.0  *****
$ These messages are used by the dthelpview program.  They need to be 
$ localized for dthelpview, however, they are not directly used by libDtHelp
$ Messages 1 through 7 are command line usage response messages.
$ Do NOT translate the option name (e.g. -helpVolume).  DO translate
$ the argument descriptor name (e.g. <Help Volume File>)
1 用法：%s...\n
2 \t-helpVolume <Help Volume File>\n
3 \t-locationId  <ID>\n
5 \t-file  <ASCII Text File>\n
6 \t-man\n
7 \t-manPage <Unix Man Page>\n\n\n
$ man page dialog
11 線上援助頁
12 線上援助頁：
13 顯示線上援助頁
$ button labels
16 結束
17 關閉
18 反向搜尋
19 列印 ...
$ title of helpview window
20 輔助說明察看器


$set 8
$ *****  Module: HelpUtil.c  *****
$ Message 1 is the label used in the definition link dialog box 
$ (e.g. Quick help close button)
2 關閉
3 內部錯誤：無法抓取滑鼠\nDtHelpReturnSelectedWidget 中斷。\n
4 內部錯誤：無法抓取鍵盤\nDtHelpReturnSelectedWidget() 中斷。\n
5 輔助說明錯誤 
$ Messages 10 and up were added after message catalogs were taken
$ for translation.  They are not in any order or chosen location.
10 仍舊執行
11 不要執行
12 輔助說明容體想執行一個命令別名。\n別名 '%s' 未定義。
13 輔助說明容體想執行一個命令別名。\n為了安全著想，自動執行命令的功能被關閉。\n命令為： %s
$ End Message set 8


$set 9
$ *****  Module: PathArea.c  *****
$ These messages are used in the general Help Dialog Widget.
$ Message 1 is the label used to label Topic Tree Area  
1 容體：
$ Messages 2 though 5 are used in the Button Box area of our general 
$ help dialog.
2 反向搜尋
3 記錄...
4 索引...
5 頂層


$set 10 
$ *****  Module: VolSelect.c  *****
$ Message 1 is the default label used to label the volume selection dialog.
1 輔助說明 - 容體選擇
2 輔助說明容體
3 請選擇一個或多個輔助說明容體
4 關閉
5 輔助說明


$set 11
$ *****  Module: HelpQuickD.c  *****
$ These messages are used as labels for the quick help dialog.
1 關閉
2 更多 ...
3 反向搜尋
4 列印 ...
5 輔助說明 ...

$set 99
$    DO NOT TRANSLATE THESE MESSAGES
$    DO NOT CHANGE THESE MESSAGES
$    DO NOT LOCALIZE THESE MESSAGES
$    These messages are used for the version information.
$quote "
1 "@(#)version_goes_here"
2 "\n@(#)Message catalog source $XConsortium: DtHelp.msg /main/3 1995/11/08 14:03:11 rswiston $"

