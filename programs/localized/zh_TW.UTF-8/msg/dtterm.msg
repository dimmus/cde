$ $XConsortium: dtterm.msg /main/3 1995/11/08 14:08:49 rswiston $
$ *************************************<+>*************************************
$ *****************************************************************************
$ **
$ **  File:        dtterm.msg
$ **
$ **  Project:     Common Desktop Environment dtterm
$ **
$ **  Description:
$ **  -----------
$ **  This file is the source for the message catalog for dtterm
$ **  Any additional messages are to be added to this file by hand,
$ **  and the associated index number placed in the code.
$ **
$ *****************************************************************************
$ **
$ **     (c) Copyright 1993, 1994 Hewlett-Packard Company
$ **     All Rights reserved
$ **
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
$ **  ----------------GENERAL LOCALIZATION NOTES SECTION END------------------
$ *************************************<L>*************************************

$ *****************************************************************************
$ 
$ ***** NOTE FOR MESSAGE CATALOG TRANSLATORS *****
$ 
$ There may be three types of messages in this file:
$ 
$ 1. Messages that appear in dialogs or are displayed to the user.
$ 
$     These messages are the default and they should ALL BE LOCALIZED.
$     Note that these messages do NOT have any identification (see the
$     comments for type 2 and 3 below).
$ 
$ 2. Messages that only appear in the DT error log file ($HOME?.dt/errorlog).
$ 
$     The localization of these messages is OPTIONAL.  These messages are
$     identified by the following:
$ 
$     MESSAGES xx-yy WILL ONLY APPEAR IN THE DT ERRORLOG FILE
$ 
$ 3. Messages that should not be localized.
$ 
$     These messages are identified by the following:
$ 
$     DO NOT TRANSLATE or CHANGE or LOCALIZE MESSAGES xx-yy
$ 
$ ***** END (NOTE FOR MESSAGE CATALOG TRANSLATORS) *****
$ 
$ ********** PROLOGUE ****************
$  Date    Name               Note
$ ======== =================  ===============================================
$ 03/15/94 B. May             Initial rev
$ 
$ *****************************************************************************

$set 1
$ ****** Module: DtTermMain.c 
$ Message 1 is the window menu addition - do not translate f.separator or
$ f.send_msg.  It will be used as the format string for a sprintf() command,
$ so it is necessary to double up on any percent signs ('%') that you
$ want to keep in the string.
$ Message 2 is an error message

1 \
	Sep             f.separator\n\
	切換\\功能\\軸  f.send_msg %d\
	
2 %s：-S 選項不能和 -server 或 -serverid 選項配合使用\n
3 終端機

$set 2
$ ****** Module: DtTermSyntax.c 
$ These are the usage messages. 
$ DO NOT TRANSLATE Messages 37 or 41 ("End-of-list")

1 -/+132				啟動/取消 80<->132 行轉換
2 -/+aw				啟動/取消自動換行
3 -bg color			背景顏色
4 -/+bs				關閉/開啟終端機背景成選取的顏色
5 -display displayname		要連接的 X 伺服器
6 -e command args		要執行的命令
7 -fb fontset			粗體字形
8 -fg color			前景顏色
9 -fn fontset			正常字形
10 -geometry geom			視窗的大小 (以字元為單位) 與位置
11 -help				顯示本則訊息
12 -/+iconic			啟動/不啟動圖像
13 -/+j				啟動/取消跳換捲動畫面
14 -/+kshMode			啟動/取消 ksh 模態
15 -/+l				啟動/取消日誌
16 -lf filename			日誌檔名
17 -/+ls				啟動/取消登入 shell
18 -/+map				啟動/取消 pty 輸出的對應視窗
19 -/+mb				啟動/取消邊界鈴
20 -ms color			指標顏色
21 -n string			指定圖像名稱
22 -name string			客戶機實例、圖像與標題字串
23 -nb distance			指定右邊界鈴的距離
24 -/+rw				啟動/取消反向換行
25 -/+sb				啟動/取消捲動軸
26 -/+sf				啟動/取消 SUN 功能鍵
27 -sl number[s]			要儲存的捲動行數[螢幕數]
28 -ti name			作為程式識別用的字串
29 -title string			視窗的標題字串
30 -tm string			終端機模態關鍵字與字元
31 -tn name			TERM 環境變數名稱
32 -/+vb				啟動/取消視覺鈴
33 -xrm resourcestring		其他的資源規格
34 -C				主控台模態
35 -Sxxd				在從屬模態 "ttyxx" 的檔案描述元 "d" 
36 -Sxxx.d			在從屬模態 "ttyxxx" 的檔案描述元 "d" 
37 End-Of-List
38 如果要指定 -e 選項，它必須出現在命令行結尾，
39 否則使用者內定的 shell 會啟動。前面加有加號 (+)
40 的選項，可以回復內定值。
41 End-Of-List
47 %s：命令行選項錯誤 "%s"\r\n\n
48 用法：%s
49 \r\n   
50  [%s]
51 \r\n\n鍵入 "%s -help" 即可得到完整的說明。\r\n\n
52 用法：\n
53 \t%s [-options ...] [-e command args]\n\n
54 選項包括：\n

$set 3
$ ****** Module: TermFunction.c
$ This is a warning message dialog box
1 記憶體已滿\n按「確定」即可清除

$set 4
$ ****** Module: TermPrim.c
$ This is the title of a dialog box
2 終端機 - 警告

$set 5
$ ****** Module: TermFunction.c
$ This is a warning message dialog box
1 記憶體已滿\n按「確定」即可清除

$set 6
$ ****** Module: TermView.c
$ DO NOT TRANSLATE Message 1
$ Messages 2 and 3 are titles for dialog boxes.
1 80x24 132x24
4 終端機 - Man Page
5 終端機 - 輔助說明

$set 7
$ ****** Module: TermViewGlobalDialog.c
$ These are the user-visible labels in the Global Options dialog box.
$ #1 is the title. 
$ The rest are resource types and values - look at the dialog box and 
$ documentation for full specs. 
$ 
$ DO NOT TRANSLATE MESSAGE 15
$ 
2 確定
3 取消
4 輔助說明
5 整體
6 游標控制
7 方塊
8 底線
9 游標樣式
10 啟動
11 取消
12 閃爍游標
13 閃爍率 (毫秒)
14 隱藏
15 blinkRate
16 顏色控制
17 反白
18 正常
19 視窗背景
20 捲動方式
21 取消
22 啟動
23 平滑捲動
24 鈴聲控制
25 聲音 
26 視覺
27 鈴聲類型 
28 啟動
29 取消
30 邊界警告
31 邊界距離 
32 即刻應用
33 重設
34 終端機 - 整體的選項
$set 8
$ ****** Module: TermViewMenu.c
$ These are the menu bar entries and menu pane choices.
$ NOTE TO TRANSLATORS: The Single letter entries are the menu mnemonic
$ for the entry above them (i.e. message 11 "F" is the mnemonic for
$ message 10 "Font Size"). 
$ The items with a "+" in them are the accellerators for an item.
$ The single-character items are the mnemonics for a menu item, where the
$ text of the menu item is specified in the previous message
$ Do not translate messages 24 and 28.
1 內定值
2 視窗大小(W)
3 W
4 -iso8859-1
5 %d 點
6 %.1f 點
7 %.1f 點
8 %.2f 點
9 內定值
10 字形大小(F)
11 F

$   Window menu
12 開新視窗(N)
13 N
14 列印(P)
15 P
16 列印(r)...
17 r
18 關閉(C)
19 C
20 視窗(W)
21 W

$   Edit menu
22 複製(C)
23 C
24 Ctrl osfInsert
25 Ctrl+Insert
26 黏貼(P)
27 P
28 Shift osfInsert
29 Shift+Insert
30 編輯(E)
31 E

$   Option menu

32 功能軸(M) 
33 M 
34 捲動軸(S)
35 S
36 整體(G)...
37 G
38 終端機(T)...
39 T
40 軟式重設(S)
41 S
42 硬式重設(H)
43 H
44 重設(R)
45 R
46 選項(O)
47 O

$ Old Help menu (no longer used for CDE)

48 概觀(O)
49 O
50 工作(T)
51 T
52 參考資料(R)
53 R
54 關於項目(O)
55 O
56 使用輔助說明(U)
57 U
59 A
60 輔助說明(H)
61 H

62 Alt+F4

63 dtterm

$ Current CDE Help menu

64 概觀(v)
65 v
66 索引(I)
67 I
68 目錄(C)
69 C
70 工作(T)
71 T
72 參考資料(R)
73 R
74 鍵盤(K)
75 K
76 使用輔助說明(U)
77 U
79 A
80 輔助說明(H)
81 H
82 關於終端機
83 關於終端機

$set 9
$ ****** Module: TermTerminalDialog.c
$ These are the messages for the terminal options dialog
2 確定
3 取消
4 輔助說明
5 鍵盤控制
6 正常
7 應用程式
8 游標鍵模態
9 數值
10 應用程式
11 輔助鍵組模態
12 歸位
13 歸位/換行
14 換行順序
15 鎖定
16 解除鎖定
17 使用者功能鍵
18 螢幕控制
19 啟動
20 取消
21 轉換成 132 行
22 行結尾換行
23 反向行結尾換行
24 即刻應用
25 重設
26 終端機 - 終端機選項
