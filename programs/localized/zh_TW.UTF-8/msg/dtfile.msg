 
$ "$TOG: dtfile.msg /main/6 1998/04/29 15:13:40 rafi $"

$ *************************************<+>*************************************
$ *****************************************************************************
$ **
$ **  File:        dtfile.msg
$ **
$ **  Project:     Common Desktop Environment dtfile
$ **
$ **  Description:
$ **  -----------
$ **  This file is the source for the message catalog for dtfile
$ **  Any additional messages are to be added to this file by hand,
$ **  and the associated index number placed in the code.
$ **
$ *****************************************************************************
$ **
$ **     (c) Copyright 1990, 1991, 1992 Hewlett-Packard Company
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
$ 03/25/93 H. Amro            removed the guilty $ from message 30
$ 
$ 
$ 
$ *****************************************************************************

$set 2
$ ****** Module: ChangeDir.c ****(developer comment only)********

$ Messages 3 and 10 are titles for the Change Folder dialog, Message 3 is
$   used when the user has specified a title for dtfile windows
$ Messages 2 and 4 are labels for the Change Folder dialog
$ Message 1 is the title for Change Folder errors
$ Messages 7, 8, 9, 11, and 12 are Change Folder error messages

1 更換檔案夾錯誤
2 開新檔案夾：
3 更換檔案夾
4 主機：
7 下列檔案夾名稱無效。\n\n%s:%s
8 您不能轉至此檔案夾。  您僅可以\n\
檢視在您目前的檔案夾之下的\n\
檔案夾。  您不能指定至新的檔案夾的\n\
絕對路徑。
9 本視區是在受限模態。您不能轉至\n\
指定的檔案夾，因為它不在受限的檔案夾\n\
的相對路徑上。\n
10 檔案管理者 -- 變更檔案夾
11 無法取得新檔案夾名稱。
12 請鍵入檔案夾名稱。

13 前往錯誤
14 目標檔案夾：
15 前往
16 系統名稱：
17 檔案管理者 -- 前往
18 漏失目標檔案夾名稱。\n\
   請鍵入檔案夾名稱，或自列表中選擇檔案夾。

19 "下列檔案夾名稱不合法。\n\n\
%s"

20 "您無法切換到檔案夾\n\n\
%s\n\n\
因為是被限制的檔案夾\n\
所以不允許察看此檔案夾。"

$set 3
$ ****** Module: ChangeDirP.c ****(developer comment only)********

$ Messages 2-6 are labels for the folder Status Line
$ Message 3 is required for SUN systems
$ Message 2-9 are obsolete

2 %d 個檔案 %d 個隱藏
3 %1$d 個檔案 %2$d 個隱藏
4 %3d 個檔案 ...
5 讀取 ...
6 %d 個隱藏

7 %d 個項目 %d 個隱藏
8 %1$d 個項目 %2$d 個隱藏
9 %3d 個項目 ...



10 %d 個項目
11 %d 個項目...
12 %d 個項目 %d 個隱藏



$set 4
$set 5
$set 6
$set 7
$set 8



$set 9
$ ****** Module: File.c ****(developer comment only)********

$ Messages 6 and 7 are the title and text for an action error message
$ Messages 8 and 9 are rename error messages
$ Messages 11-19 are the title and text for warning messages displayed
$   when an object/set of objects is moved/copied/linked to a folder
$   containing object(s) with the same name

6 動作錯誤
7 並無為 %s 所定義的動作。\n

8 已經有檔案以此檔案名稱存在工作空間中。\n\
請選擇不同的檔案名稱。
9 已經有檔案以此名稱存在。\n\
請選擇不同的檔案名稱。

11 檔案管理者 -- 移動/複製/連結 警告
$ Messages 12-15 deal with multiple files/folders
12 以下檔案 (或檔案夾)\n\
已經存在於\n\
%s:
13 \n複製作業將會重寫這些檔案。\n\
          您要繼續進行嗎?\n
14 \n移動作業將會重寫這些檔案。\n\
          您要繼續進行嗎?\n
15 \n連結作業將會重寫這些檔案。\n\
          您要繼續進行嗎?\n

$ Messages 16&17 deal with a single file/folder
16 名稱為\n%s\n之物件已存在此檔案夾中。\n%s
17 名稱為\n%s\n之檔案夾已存在此檔案夾中。\n%s

$ Labels in rename/replace and merge/replace dialogs
20 您要做什麼?
21 取代已存在的物件。
22 將已存在的物件重新命名為：
24 將兩個檔案夾的內容合併。
25 取代已存在的檔案夾。

27 確定
28 取消
29 輔助說明
30 取消移動
31 取消複製
32 取消連結

$ Window title for overwrite dialogs
33 檔案管理者 -- 移動/複製/連結 警告
34 檔案管理者 -- 移動警告
35 檔案管理者 -- 複製警告
36 檔案管理者 -- 連結警告

37 物件重新命名 -- 錯誤
38 %s\n已經存在：請選擇另外的名稱。
39 %s\n無法重新命名為\n%s\n%s
40 物件取代 -- 錯誤
41 這個檔案類別正確的圖像已經顯示。



51 複製中
52 移動中
53 連結中
54 更換中
56 取代已存在的物件
57 以附加編號的方式重新命名物件
58 已複製
59 已移動
60 已連結
61 已變更
68 物件 %s。
69 %d 物件 %s。
70 %s\n%d 物件與檔案夾中的物件同名\n%s
71 %s\n1 物件與檔案夾中的物件同名\n%s
72 複製
73 移動
74 連結
75 作業
76 複製
77 移動
78 連結
79 作業
80 %s 取消 -- 此檔案夾未被 %s.\n\n您要取代的檔案夾不能被刪除\n因為該檔案夾中的物件無法刪除。\n該檔案中的其他物件可能在%s 程序停止之前就以被刪除了。
81 %s\n\n這個問題最可能的原因是您沒有針對該無法除去的物件的寫入權許可。\n您可以變更寫入權或重新命名您正在 %s 的檔案夾之名稱，\n如此它們就不會再有相同的名稱了。
82 %s 已刪除\n\n%s\n無法除去。\n\n這個問題最可能的原因是您沒有該檔案的寫入權許可。\n您可以變更寫入權或重新命名您所在的檔案之名稱，\n如此它們就不會再有相同的名稱了。
83 %s 已取消 -- 物件未被 %s.\n\n接續的物件無法自動被重新命名\n%s\n\n%d 物件在 %s 程序停止前就被重新命名了。
84 %s\n\n這個問題最可能的原因是您對該物件沒有寫入權許可。您可以變更寫入權許可，\n或是對正在 %s 的物件重新命名，如此它就不再有相同的名稱了。
85 %s 已取消 -- 物件未被 %s.\n\n接續的物件無法自動被重新命名\n%s\n\n1 物件在 %s 程序停止前就被重新命名了。
86 %s 已取消 -- 物件未被 %s。\n\n您要取代的檔案夾無法被刪除。\n   檔案夾： %s\n\n此檔案夾中的某些物件可能在\n%s 程序停止前就被刪除了。\n\n1 個其他物件已被刪除。\n\n這個問題最可能的原因是\n您對該檔案夾沒有寫入權許可。您可以變更寫入權許可，\n或是將您正在 %s 的檔案夾重新命名，如此它就不再有相同的名稱了。
87 %s 已取消 -- 物件未被 %s.\n\n您要取代的檔案夾無法被刪除。\n 檔案夾：%s\n\n檔案夾中的某些物件可能在\n%s 程序停止前就被刪除了。\n\n1 個其他的物件已被刪除。\n\n這個問題最可能的原因是\n您對該檔案夾沒有寫入權許可。您可以變更寫入權許可，\n或是對您正在 %s 的檔案夾重新命名，如此它就不會再有相同的名稱了。
88 %s 已取消 -- 物件未被 %s.\n\n您要取代的檔案無法被刪除。\n    檔案：%s\n\n%d 個其他物件已被刪除。\n\n這個問題最可能的原因是\n您對該檔案沒有寫入權許可。您可以變更寫入權許可，\n或是對您正在 %s 的檔案重新命名，如此它就不會再有相同的名稱了。
89 %s 已取消 -- 物件未被 %s.\n\n您要取代的檔案無法被刪除。\n    檔案：%s\n\n%1 個其他物件已被刪除。\n\n這個問題最可能的原因是\n您對該檔案沒有寫入權許可。您可以變更寫入權許可，\n或是對您正在 %s 的檔案重新命名，如此它就不會再有相同的名稱了。
90 名稱衝突。\n\
  物件在工作空間背景定位上。\n\
  在背景定位上的某個物件之名稱\n\
  與您所鍵入的相同。\n\
  請選擇不同的名稱。

91 停止複製
92 停止連結
93 停止移動

94 "錯誤"

95 "取消移動 - 不再移動檔案夾。\n\n\
因為檔案夾內的物件無法刪除\n\
所以無法刪除您要取代的檔案夾。\n\
檔案夾內的某些物件可能在移動程序\n\
停止前就刪除了。\n\n\
造成此問題的原因可能是對於無法\n\
刪除的物件缺少寫入權。更改\n\
寫入權或手動更改要移動的\n\
檔案夾名稱。"

96 "取消複製 - 不再複製檔案夾。\n\n\
因為檔案夾內的物件無法刪除\n\
所以無法刪除您要取代的檔案夾。\n\
檔案夾內的某些物件可能在複製程序\n\
停止前就刪除了。\n\n\
造成此問題的原因可能是對於無法\n\
刪除的物件缺少寫入權。更改\n\
寫入權或手動更改要複製的\n\
檔案夾名稱。"

97 "取消連結 - 不再連結檔案夾。\n\n\
因為檔案夾內的物件無法刪除\n\
所以無法刪除您要取代的檔案夾。\n\
檔案夾內的某些物件可能在連結程序\n\
停止前就刪除了。\n\n\
造成此問題的原因可能是對於無法\n\
刪除的物件缺少寫入權。更改\n\
寫入權或手動更改要連結的\n\
檔案夾名稱。"

98 "取消作業 - 不更改檔案夾。\n\n\
因為檔案夾內的物件無法刪除\n\
所以無法刪除您要取代的檔案夾。\n\
檔案夾內的某些物件可能在程序\n\
停止前就刪除了。\n\n\
造成此問題的原因可能是對於無法\n\
刪除的物件缺少寫入權。更改 \n\
寫入權或手動更名要更改的\n\
檔案夾。"

99 "取消移動\n\n\
%s\n\
無法刪除。\n\n\
造成此問題的原因可能是對於\n\
此檔案缺少寫入權。\n\
更改寫入權或手動更改\n\
要移動的檔案名稱。"

100 "取消複製\n\n\
%s\n\
無法刪除。\n\n\
造成此問題的原因可能是對於\n\
此檔案缺少寫入權。\n\
更改寫入權或手動更改\n\
要複製的檔案名稱。"

101 "取消連結\n\n\
%s\n\
無法刪除。\n\n\
造成此問題的原因可能是對於\n\
此檔案缺少寫入權。\n\
更改寫入權或手動更改\n\
要連結的檔案名稱。"

102 "取消作業\n\n\
%s\n\
無法刪除。\n\n\
造成此問題的原因可能是對於\n\
此檔案缺少寫入權。\n\
更改寫入權或手動更名\n\
要更改的檔案。"

103 "取消移動 - 沒有移動任何物件。\n\n\
下列物件無法自動更名\n\
%s\n\n\
移動程序停止前更名 %d 個物件。\n\n\
造成此問題的原因可能是對於此物件缺少寫入權。\n\
更改寫入權或手動更名\n\
要移動的物件。"

104 "取消複製 - 沒有複製任何物件。\n\n\
下列物件無法自動更名\n\
%s\n\n\
複製程序停止前更名 %d 個物件。\n\n\
造成此問題的原因可能是對於此物件缺少寫入權。\n\
更改寫入權或手動更名\n\
要複製的物件。"

105 "取消連結 - 沒有連結任何物件。\n\n\
下列物件無法自動更名\n\
%s\n\n\
連結程序停止前更名 %d 個物件。\n\n\
造成此問題的原因可能是對於此物件缺少寫入權。\n\
更改寫入權或手動更名\n\
要連結的物件。"

106 "取消作業 -沒有更改任何物件。\n\n\
下列物件無法自動更名\n\
%s\n\n\
程序停止前更名 %d 個物件。\n\n\
造成此問題的原因可能是對於此物件缺少寫入權。\n\
更改寫入權或手動更名\n\
要更改的物件。"

107 "取消移動 - 沒有移動任何物件。\n\n\
下列物件無法自動更名\n\
%s\n\n\
移動程序停止前更名 1 個物件。\n\n\
造成此問題的原因可能是對於此物件缺少寫入權。\n\
更改寫入權或手動更名\n\
要移動的物件。"

108 "取消複製 - 沒有複製任何物件。\n\n\
下列物件無法自動更名\n\
%s\n\n\
複製程序停止前更名 1 個物件。\n\n\
造成此問題的原因可能是對於此物件缺少寫入權。\n\
更改寫入權或手動更名\n\
要複製的物件。"

109 "取消連結 - 沒有連結任何物件。\n\n\
下列物件無法自動更名\n\
%s\n\n\
連結程序停止前更名 1 個物件。\n\n\
造成此問題的原因可能是對於此物件缺少寫入權。\n\
更改寫入權或手動更名\n\
要連結的物件。"

110 "取消作業 - 沒有更改任何物件。\n\n\
下列物件無法自動更名\n\
%s\n\n\
程序停止前更名 1 個物件。\n\n\
造成此問題的原因可能是對於此物件缺少寫入權。\n\
更改寫入權或手動更名\n\
要更改的物件。"

111 "取消移動 - 沒有移動任何物件。\n\n\
無法刪除您要取代的檔案夾。\n\
   檔案夾: %s\n\n\
移動程序停止前可能已刪除\n\
檔案夾中的某些物件。\n\n\
已刪除 %d 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案夾缺少寫入權。\n\
更改寫入權或手動更名\n\
要移動的檔案夾。"

112 "取消複製 - 沒有複製任何物件。\n\n\
無法刪除您要取代的檔案夾。\n\
   檔案夾: %s\n\n\
複製程序停止前可能已刪除\n\
檔案夾中的某些物件。\n\n\
已刪除 %d 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案夾缺少寫入權。\n\
更改寫入權或手動更名\n\
要複製的檔案夾。"

113 "取消連結 - 沒有連結任何物件。\n\n\
無法刪除您要取代的檔案夾。\n\
   檔案夾: %s\n\n\
連結程序停止前可能已刪除\n\
檔案夾中的某些物件。\n\n\
已刪除 %d 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案夾缺少寫入權。\n\
更改寫入權或手動更名\n\
要連結的檔案夾。"

114 "取消作業 - 沒有更改任何物件。\n\n\
無法刪除您要取代的檔案夾。\n\
   檔案夾: %s\n\n\
程序停止前可能已刪除\n\
檔案夾中的某些物件。\n\n\
已刪除 %d 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案夾缺少寫入權。\n\
更改寫入權或手動更名\n\
要更改的檔案夾。"

115 "取消移動 - 沒有移動任何物件。\n\n\
無法刪除您要取代的檔案夾。\n\
   檔案夾: %s\n\n\
移動程序停止前可能已刪除\n\
檔案夾中的某些物件。\n\n\
已刪除 1 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案夾缺少寫入權。\n\
更改寫入權或手動更名\n\
要移動的檔案夾。"

116 "取消複製 - 沒有複製任何物件。\n\n\
無法刪除您要取代的檔案夾。\n\
   檔案夾: %s\n\n\
複製程序停止前可能已刪除\n\
檔案夾中的某些物件。\n\n\
已刪除 1 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案夾缺少寫入權。\n\
更改寫入權或手動更名\n\
要複製的檔案夾。"

117 "取消連結 - 沒有連結任何物件。\n\n\
無法刪除您要取代的檔案夾。\n\
   檔案夾: %s\n\n\
連結程序停止前可能已刪除\n\
檔案夾中的某些物件。\n\n\
已刪除 1 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案夾缺少寫入權。\n\
更改寫入權或手動更名\n\
要連結的檔案夾。"

118 "取消作業 - 沒有更改任何物件。\n\n\
無法刪除您要取代的檔案夾。\n\
   檔案夾: %s\n\n\
程序停止前可能已刪除\n\
檔案夾中的某些物件。\n\n\
已刪除 1 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案夾缺少寫入權。\n\
更改寫入權或手動更名\n\
要更改的檔案夾。"

119 "取消移動 - 沒有移動任何物件。\n\n\
無法刪除您要取代的檔案。\n\
    檔案: %s\n\n\
已刪除 %d 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案缺少寫入權。\n\
更改寫入權或手動更名\n\
要移動的檔案。"

120 "取消複製 - 沒有複製任何物件。\n\n\
無法刪除您要取代的檔案。\n\
    檔案: %s\n\n\
已刪除 %d 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案缺少寫入權。\n\
更改寫入權或手動更名\n\
要複製的檔案。"

121 "取消連結 - 沒有連結任何物件。\n\n\
無法刪除您要取代的檔案。\n\
    檔案: %s\n\n\
已刪除 %d 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案缺少寫入權。\n\
更改寫入權或手動更名\n\
要連結的檔案。"

122 "取消作業 - 沒有更改任何物件。\n\n\
無法刪除您要取代的檔案。\n\
    檔案: %s\n\n\
已刪除 %d 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案缺少寫入權。\n\
更改寫入權或手動更名\n\
要更改的檔案。"

123 "取消移動 - 沒有移動任何物件。\n\n\
無法刪除您要取代的檔案。\n\
\ 檔案: %s\n\n\
已刪除 1 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案缺少寫入權。\n\
更改寫入權或手動更名\n\
要移動的檔案。"

124 "取消複製 - 沒有複製任何物件。\n\n\
無法刪除您要取代的檔案。\n\
\ 檔案: %s\n\n\
已刪除 1 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案缺少寫入權。\n\
更改寫入權或手動更名\n\
要複製的檔案。"

125 "取消連結 - 沒有連結任何物件。\n\n\
無法刪除您要取代的檔案。\n\
\ 檔案: %s\n\n\
已刪除 1 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案缺少寫入權。\n\
更改寫入權或手動更名\n\
要連結的檔案。"

126 "取消作業 - 沒有更改任何物件。\n\n\
無法刪除您要取代的檔案。\n\
\ 檔案: %s\n\n\
已刪除 1 個其他物件。\n\n\
造成此問題的原因可能是對於此檔案缺少寫入權。\n\
更改寫入權或手動更名\n\
要更改的檔案。"

127 "已移動 1 個物件。"

128 "已移動 %d 個物件。"

129 "已複製 1 個物件。"

130 "已複製 %d 個物件。"

131 "已連結 1 個物件。"

132 "已連結 %d 個物件。"

133 "已更改 1 個物件。"

134 "已更改 %d 個物件。"

135 "無法將物件 %s 移動到它本身。  "

136 "無法將物件 %s 複製到它本身。  "

137 "無法將物件 %s 連結到它本身。  "

138 "下列物件更名失敗\n\n\
%s\n\n\
可能因為沒有\n\
更改物件名稱的權限。"

139 "下列物件取代失敗\n\n\
%s\n\n\
可能因為沒有\n\
取代物件的權限。"

$ "NEW-"
140 Rename new object to:

$ "NEW-"
141 Rename new objects by appending a number


$set 10
$ ****** Module: FileDialog.c ****(developer comment only)********

$ Messages 1, 2, 20, and 21 are titles for error dialogs
$ Message 3 is an error message for dialogs requiring new file names
$ Messages 5 and 27 are titles for the Copy File dialog, Message 5 is used
$   when the user has specified a title for dtfile windows
$ Messages 6-9 are labels for the File Copy, New Dir, and New File dialogs
$ Messages 10-13 and 25 are error messages for the file copy operation
$ Messages 14 and 28 are titles for the New Folder dialog, Message 14 is
$   used when the user has specified a title for dtfile windows
$ Message 16 is a label for the New Folder dialog
$ Messages 22-24 are error messages for the New File/Folder operations
$ Messages 26 and 29 are titles for the New File dialog, Message 26 is used
$   when the user has specified a title for dtfile windows
$ Messages 30 and 31 are titles for the Move File dialog, Message 30 is used
$   when the user has specified a title for dtfile windows
$ Messages 32 and 33 are titles for the Link File dialog, Message 32 is used
$   when the user has specified a title for dtfile windows

1 檔案重新命名錯誤
2 檔案複製錯誤

3 檔案或檔案夾名稱中不允許有空格

5 複製檔案

6 目前的檔案名稱：
7 新的檔案名稱：
8 檔案類型
9 顯示檔案類型

10 原先選擇的檔案不再存在。
11 檔案或檔案夾不可有空白的名稱。
13 無效的檔案或檔案夾名稱。

14 開新檔案夾

16 新檔案夾名稱：

20 建立檔案夾錯誤
21 建立檔案錯誤

22 無法建立以空白為名稱的檔案或檔案夾。
23 無效的檔案夾名稱。
24 無效的檔案名稱。

25 您不能使用複製命令將檔案\n\
複製至工作空間。您必須使用\n\
"檔案"功能表中的`放入工作空間` 命令，或是\n\
將檔案定位於工做空間中。

26 開新檔案

27 檔案管理者 -- 複製檔案
28 檔案管理者 -- 開新檔案夾
29 檔案管理者 -- 開新檔案

30 移動檔案
31 檔案管理者 -- 移動檔案

32 連結檔案
33 檔案管理者 -- 連結檔案
34 目標檔案夾： 
35 物件名稱：
36 顯示圖像

37 選定的物件：
38 複本名稱：

39 改名錯誤
40 這個物件是一個動作。\n\
   動作圖像標籤不能直接改名。

41 正確的資料類別圖像已顯示.

42 "移動物件"

43 "檔案管理者 - 移動物件"

44 "複製物件"

45 "檔案管理者 - 複製物件"

46 "連結物件"

47 "檔案管理者 - 連結物件"

48 "新物件名稱:"


$set 11
$ ****** Module: FileManip.c ****(developer comment only)********

$ Message 6 is the title for an error dialog
$ Messages 9, 14, 16, 28, and 30-36 are file manipulation error messages

6 檔案處理錯誤

9 您無法將檔案自\n\
檔案垃圾檔定位至工作空間中。

14 無法找到檔案夾的位置。
16 檔案夾無法移至本身之中。\n%s
28 無法找到 %s。 
30 無法移動或重新命名檔案夾 %s。\n\
所有為檔案夾或其子檔案夾顯示的檔案管理者視區\n\
皆必須在檔案夾被移動或重新命名前關閉。
31 讀取%s時發生錯誤。
32 無法重新命名 %s
33 無法開啟 %s
34 您沒有移動 %s 的許可。\n\
然而，您可以複製該檔案 (請在\n\
拖曳該檔案時使用 <CTRL>)。
35 無法複製檔案夾至本身. 
36 無法建立子程序。

37 您不能將檔案自\n\
檔案垃圾檔中拖曳至工作空間。\n\
若要自檔案垃圾檔中除去物件，\n\
   --請選擇該物件，然後\n\
   -請選擇 '放回'\n\
以上動作是在"檔案"功能表或該物件的即現功能表中完成。\n\
然後您可以將該物件自檔案功能表中拖曳至\n\
工作空間背景定位上放置。

38 您沒有移動%s的許可。\n\
然而，您可以移動該物件。\n\
若要複製物件：\n\
  -- 請按住 <Ctrl>鍵，然後\n\
  -- 以您的滑鼠拖曳該物件。\n\
或是\n\
  -- 使用功能軸中"選取"即現功能表中的"複製到"命令。

39 無法建立子程序。\n\
   系統中程序之數目已達最大量。\n\
   請停止目前正在執行的某些程序或程式，\n\
   然後重新試該功能。

40 來源物件與目標物件相同。\n%s
41 檔案夾\n%s\n不存在。
42 您要複製的目的地沒有\n\
   空間可用。
43 您要移動的目的地沒有\n\
   空間可用。
44 沒有對物件 %s 執行搬移動作。\n\
   不能搬移該物件至已有該物件之檔案夾。\n\
   您必須輸入一個不同的目的地檔案夾\n\
   名稱才能搬移該物件。
45 沒有對物件 %s 執行複製動作。\n\
   必須更改目的檔案夾名稱或複\n\
   本名稱，才能建立複本
46 沒有對物件 %s 執行連結動作。\n\
   必須更改目的檔案夾名稱或複\n\
   本名稱，才能建立複本。

47 無法搬移檔案夾至本身。

48 "無法將下列物件複製到檔案夾 "%s"\n\n\
%s\n\n\
可能因為沒有寫入目標檔案夾\n\
的權限。"

49 "無法將下列物件移動到檔案夾 "%s"\n\n\
%s\n\n\
可能因為沒有寫入目標檔案夾\n\
的權限。"

50 "無法將下列物件移動到檔案夾 "%s"\n\n\
%s\n\n\
可能因為沒有移動來源物件\n\
的權限。"
$        
$ Messages 51 - 56 are new since CDE/SI.          
$        

51 "因為將超過複製目的\n\
的磁碟空間，所以不能複製\n\
此檔案或檔案夾。"

52 "因為將超過移動目的\n\
的磁碟空間，所以不能移動\n\
此檔案或檔案夾。"

53 "因為某些內部錯誤而導致\n\
複製檔案或檔案夾失敗。\n\
內部錯誤是:"

54 "因為某些內部錯誤而導致\n\
移動檔案或檔案夾失敗。\n\
內部錯誤是:"

55 "請參考系統管理者"

56 "(未知的):"

58 Process Create Error
59 Cannot create a child process to delete the dropped files.

$set 12
$ ****** Module: FileMgr.c ****(developer comment only)********

$ Message 3 is a dialog title
$ Messages 4-6 are movement warning or error messages
$ Message 7 is the title for the folder popup menus

3 移動檔案

4 物件 %s 目前在檔案夾 %s 中。\n\
您目前正將物件移往檔案夾 %s。\n\
您要這樣做嗎?
5 物件 %1$s 目前在檔案夾 %2$s 中。\n\
您目前正將物件移往檔案夾 %3$s。\n\
您要這樣做嗎?
6 您已定位的檔案中致少有一個是來自不同的檔案夾。\n\
您正在將所有的這些檔案移往 %s。\n\
您要這樣做嗎?

7 檔案管理者

18 檔案夾 %s 是空的.
19 檔案夾 %s 中沒有子目錄.


$set 13
$ ****** Module: Filter.c ****(developer comment only)********

$ Messages 11 and 21-22 are button labels for the Modify Filter List dialog
$ Messages 19 and 24 are titles for the Link File dialog, Message 19 is used
$   when the user has specified a title for dtfile windows
$ Message 23 is a text field label for the Modify Filter List dialog
$ Message 25 is a label for the Modify Filter List dialog

11 內定值

19 設定過濾器選項

21 全部選取
22 取消所有選取

23 過濾器字串 (選擇性)：

24 檔案管理者 -- 設定過濾器選項

25 選取欲隱藏之檔案類別：

26 亦隱藏 (選擇性)

27 已顯示

28 已隱藏

29 同時顯示 (選擇性)：

30 選取要隱藏的資料類別:

31 "選取資料纇型是:"


$set 14



$set 15
$ ****** Module: Find.c ****(developer comment only)********

$ Message 1 is the title for an error dialog
$ Messages 14 and 15 are button labels for the Find Files dialog
$ Messages 23, 26, and 27 are Find Files error messages
$ Messages 29 and 41 are titles for the Find Files dialog, Message 29 is used
$   when the user has specified a title for dtfile windows
$ Messages 30-40 and 42 are labels for the Find Files dialog

1 尋找檔案錯誤

14 啟動
15 停止

23 無效的尋找資訊。\n\n\
須指定欲搜尋的檔案夾。
26 無效的尋找資訊。\n\n\
選取的檔案已不存在。
27 沒有檔案符合搜尋資料。

29 尋找檔案或檔案夾

30 請填入一個或多個欄位以指定欲尋找的檔案：
31 檔案或檔案夾名稱：
32 檔案內容：

$ Messages 33-35 are used on hpux systems
33 開啟
34 關閉
35 跟隨連結：

36 開啟新視區
37 放入工作空間
38 找到的檔案 (根據內容)：
39 找到的檔案 (根據名稱與內容)：
40 找到的檔案 (根據名稱)：

41 檔案管理者 -- 尋找檔案或檔案夾

42 搜尋檔案夾：

43 尋找錯誤

44 缺少"搜尋檔案夾"名稱引數。\n\
請鍵入您欲開始搜尋的檔案夾名稱：

45 選取的檔案已不存在。\n\n\
檔案在搜尋程序完成後已被某人刪除。

46 尋找

47 請填入一個或多個欄位以指定欲尋找的項目：

48 開啟檔案夾

49 檔案管理者 -- 尋找

50 缺少"搜尋檔案夾"名稱或"檔案內容"引數\n\
請鍵入您欲開始搜尋的檔案夾名稱\n\
或鍵入您欲搜尋的字串。


$set 16
$set 17



$set 18
$ ****** Module: Main.c ****(developer comment only)********

$ Message 1 is a blank line
$ Messages 2, 6, 17-18 and 27-28 are error messages which will ONLY APPEAR
$   in the DT ERRORLOG FILE
$ Messages 16 and 32 are titles for the Save Settings dialog, Message 16 is
$   used when the user has specified a title for dtfile windows
$ Messages 19-21 are error messages for invalid directory input.
$ Message 22 is the title for an error dialog
$ Messages 25 and 26 are usage strings
$ Messages 29 and 30 are text for the Save Settings dialog
$ Messages 34 and 35 are drag/drop error messages

$ DO NOT TRANSLATE or CHANGE or LOCALIZE MESSAGE 1
$quote "
1 ""
$quote

$ MESSAGE 2 WILL ONLY APPEAR IN THE DT ERRORLOG FILE
2 無法建立 .dt 檔案夾或子檔案夾。

$ MESSAGE 6 WILL ONLY APPEAR IN THE DT ERRORLOG FILE, and stderr
6 發生 X Toolkit 錯誤... 結束。\n

16 儲存為內定值選項

$ MESSSAGES 17-18 WILL ONLY APPEAR IN THE DT ERRORLOG FILE
17 無法建立儲存內定值選項的檔案。
18 無法開啟階段檔案。

19 檔案夾規格，\n\
%s\n\
不是一個檔案夾。
20 檔案夾規格，\n\
%s\n\
不存在。
21 無效的檔案夾規格， %s：%s。

22 拖曳錯誤

23 \n用法： %s...\n\n\
   -noview\n\n\
\tDtfile 在伺服器模式下執行，\n\n\
   -session "階段檔案"\n\n\
\tDtfile 執行在"階段檔案"中指定的階段檔案\n\
\t參數。\n\n\
   -folder 檔案夾[檔案夾，檔案夾]\n\
   -dir 檔案夾[，檔案夾，檔案夾]\n\n\
\tDtfile 為每個在"檔案夾"參數中指定的檔案夾\n\
\t顯示一個視窗。 "檔案夾"參數可包含許多\n\
\t以逗號隔開的檔案夾。檔案夾可能在\n\
\t'路徑'中。\n\n\
   -title "標題"\n\n\
\tDtfile 使用在"標題"參數中止定的字串\n\
\t作為視窗的標題。\n\n\
   -help_volume "輔助說明容體"\n\n\
\tDtfile 使用在"輔助說明容體"參數中指定的輔助說明容體。\n\n\
   -restricted\n\n\
\tDtfile 不會顯示在受限檔案夾之上的檔案夾。\n\
\t若 -dir 選項被使用，則該選項中指定的檔案夾\n\
\t就是受限檔案夾。若 -dir 選項未被使用，\n\
\t使用者的目前檔案夾就是受限檔案夾。\n\n\
   -grid 開啟／關閉\n\n\
\t開啟    ＝ 檔案以網點型態顯示。\n\
\t關閉    ＝ 檔案以其被放置的型態顯示。\n\n\
   -tree 開啟／關閉\n\n\
\t開啟     ＝ 檔案以單檔案夾模式顯示。\n

24 \t關閉    ＝ 檔案以檔案夾樹模式顯示。\n\n\
   -tree_files /never/always/choose\n\n\
\tnever  ＝ 樹模式有兩個狀態：部份擴張或摺疊。\n\
\talways ＝ 樹模式有兩個狀態：完全擴張或摺疊。\n\
\tchoose ＝ 樹模式有三個狀態：部份擴張，完全\n\
\t擴張，或摺疊。\n\n\
   -order 字母順序／檔案類別／日期／大小\n\n\
\t檔案以指定的順序顯示：根據字母順序，根據\n\
\t檔案類別，根據日期，或根據大小。\n\n\
   -view 無圖像／大圖像／小圖像／屬性\n\n\
\t檔案以指定的格式顯示：純文字，文字與\n\
\t大圖像，文字與小圖像，以及屬性。\n\n\
   -direction 升序／降序\n\n\
\t檔案以指定的方向顯示：升序或\n\
\t降序。\n\n\
   -large_icon_width <size>\n\n\
   -large_icon_height <size>\n\n\
   -small_icon_width  <size>\n\n\
   -small_icon_height <size>\n\n\
       在檔案管理者中圖像影像的顯示區域大小\n\
       大於這個大小的圖像影像將會被裁剪成這個大小\n\
       內定的顯示區域大小，大的是 38，小的是 24\n\n
26 \n受限模式用法： %s...\n\n\
   -folder Folder[,Folder,Folder]\n\
   -dir Folder[,Folder,Folder]\n\n\
\t"Folder"是一個在使用者的起始檔案夾之下，且/或包括該起始\n\
\t檔案夾。\n\n

$ MESSSAGES 27-28 WILL ONLY APPEAR IN THE DT ERRORLOG FILE
27 無法復原已儲存的內定起始設定檔案，將使用內定值。\n
28 無法復原已儲存的內定工具設定檔案，將使用內定值。\n

$ Messages 29-30 are the strings displayed in a question dialog
29 您要儲存目前的"檔案管理者視區"的視窗大小，\n\
優先權，與過濾器設定嗎?\n\n\
它們會在任一自"前方面板"中啟動的\n\
新的"檔案管理者視區"視窗中被使用。
30 您要儲存目前的"受限檔案管理者視區"的視窗大小，\n\
優先權，與過濾器設定嗎?\n\n\
它們會在任一自"前方面板"中啟動的\n\
新的"受限檔案管理者視區"視窗中被使用。

32 檔案管理者 -- 儲存為內定值選項

34 您無法將檔案或檔案夾\n\
複製或連結到檔案垃圾檔。

$ 35 您無法自檔案垃圾檔中複製\n\
$ 或連結檔案或檔案夾。

35 您要儲存目前的"應用程式管理者視區"的視窗大小，\n\
優先權，與過濾器設定嗎?\n\n\
它們會在任一自"前方面板"中啟動的\n\
新的"應用程式管理者視區"視窗中被使用。

36 您無法自檔案垃圾檔中複製或連結檔案或檔案夾。\n\
請將該物件自檔案垃圾檔中移出，並將其放入"檔案管理者"。\n\
您可以在那兒複製或連結該物件。

37 您無法自檔案垃圾檔中複製或連結檔案或檔案夾。\n\
請將該物件自檔案垃圾檔中移出，並將其放入"檔案管理者"。\n\
您可以在那兒複製或連結該物件。

38 "指定檔案夾不合法, %s"

$set 19



$set 20
$ ****** Module: Menu.c ****(developer comment only)********

$ Messages 1-8, 123 and 9 are the text and mnemonics for the dtfile menu bar
$ Messages 10/11, 128/129, 124/125, 121/122, 126/127, 16/17, 18/19, 84/85,
$   90/91, 22/23, 117/118, are the text and mnemonics for the Fill 
$   pulldown menu 
$ Messages 10/11, 30/33, 92/93, 34/37, 38/41, 42/94, and 97/98 are the text
$   and mnemonics for the Folder pulldown menu
$ Messages 46/47, 99/100, 48/51, 52/55, 101/102, 103/104, 119/120, and 
$   62/63 are the text and mnemonics for the View pulldown menu
$ Messages 105-116 are the text and mnemonics for the Help pulldown menu

1 檔案(F)
2 F
3 檔案夾(D)
4 D
5 視區(V)
6 V
7 選取(S)
8 S

9 H

10 新增(N)...
11 N
16 特性(P)...
17 P
18 尋找(F)...
19 F
22 顯示檔案垃圾檔(S)...
23 S
30 更新(U)
33 U
34 更改為(C)...
37 C
38 回到起始處(H)
41 H
$ NOTE: The mnemonic for Up is Message 94 in this set.
42 向上(U)
46 新增(N)
47 N
48 全部選取(S)
51 S
52 取消所有選取(D)
55 D
62 儲存全部特性(t)...
63 t
84 放入工作空間(W)
85 W
90 刪除並放入垃圾檔(T)
91 T
92 快速更改為(F)
93 F
$ NOTE: Message 94 is the mnemonic for the "Up" menu item
94 U
97 終端機(T)
98 T
99 清除(C)
100 C
101 顯示隱藏檔案(H)
102 H
103 修改過濾器列表(F)...
104 F
105 概觀(v)
106 v
107 工作(T)
108 T
109 參考資料(R)
110 R
111 關於項目(O)
112 O
113 使用輔助說明(U)
114 U
115 關於檔案管理者(A)
116 A
117 關閉(C)
118 C
119 設定視區特性(P)...
120 P
121 複製(y)...
122 y
123 輔助說明
124 移動(M)...
125 M
126 連結(L)...
127 L
128 重新命名(R)
129 R
130 開新檔案夾(N)...
131 N
132 開新檔案(w)...
133 w
134 前往(G)...
135 G
136 開啟垃圾檔
137 重新命名...
138 開啟新視區(O)
139 O
140 複製為連結...
141 設定視區選項...
142 儲存為內定值選項.(S)..
143 S
144 設定過濾器選項(C)...
145 C
146 開啟終端機(O)
147 O
148 目錄(C)
149 C
150 更改許可權...
151 放入垃圾檔
152 移動到...
153 複製到...
154 複製為連結...
155 往前
156 顯示隱藏物件
157 開於應用程式管理者


$set 21
$ ****** Module: ModAttr.c ****(developer comment only)********

$ Messages 1 and 21 are titles for the File Properties dialog, Message 1 is
$   used when the user has specified a title for dtfile windows
$ Messages 2-13 and 18 are labels for the File Properties dialog
$ Message 14 is the output of the file modification date; DO NOT LOCALIZE
$ Messages 15 and 16 are file properties error messages
$ Message 17 is the title for an error dialog
$ Message 19 is the default file owner/group, if real one can't be determined
$ Messages 22-28 are error messages that can be generated when dtfile
$   tries to use the dtfile.config configuration file, this file is
$   used to customize the File Properties dialog

1 檔案許可權

2 檔案名稱：
3 所有者名稱：
4 群組名稱：
5 許可權：
6 讀取
7 寫入
8 執行
9 所有者
10 群組：
11 其他：
12 大小 (位元組)：
13 修改：

$ DO NOT LOCALIZE MESSAGE 14
14 %x %X

15 無效的檔案屬性資訊。\n\n\
指定的使用者不存在。
16 無效的檔案屬性資訊。\n\n\
指定的群組不存在。

17 特性錯誤

18 被連結到：

19 未知

21 檔案管理者 -- 檔案許可權

22 無法開啟檔案管理者配置檔案：
24 檔案管理者配置檔案中有未知欄位標籤：
25 檔案管理者配置檔案中找不到關於檔案系統是別字的資訊
26 無法為檔案系統對話框建立程序
27 錯誤地執行了檔案系統對話框
28 嘗試執行：

$ do not localize "fsDialog"
29 dtfile 的配置檔案中沒有提供 fsDialog 任何值。

30 名稱：

31 所有者名稱無效。\n\
指定的使用者不存在。

32 群組名稱無效。\n\n\
您的系統中沒有以該名稱建立的使用者群組。\n\
若您要以該名稱建立使用者群組，\n\
請使用您的系統管理軟體。

33 許可權錯誤

34 檔案管理者 -- 許可權

35 "物件的許可權對話\n\n\
%s/%s\n\n\
關閉，因為該物件已自系統\n\
刪除。"

36 "更改許可權警告"

37 "更改現行檔案夾的許可權，\n\
不再允許察看檔案夾。\n\
檔案夾將自動關閉\n\
且不可再重新開啟\n\
此檔案夾。\n\n\
您要如此作嗎?"


38 File Manager Error
39 File Open Error

$set 22
$ ****** Module: ModAttrP.c ****(developer comment only)********

$ Messages 1 and 3 are file properties error messages
$ Message 2 is the title for the properties error dialog

1 無法開啟檔案，作業中斷
2 特性錯誤
3 無法為 %s 更改特性
4 無法開啟物件。
5 設定許可權錯誤



$set 23
$ ****** Module: Prefs.c ****(developer comment only)********

$ Messages 1 and 22 are titles for the Set Preferences dialog, Message 1 is
$   used when the user has specified a title for dtfile windows
$ Messages 2-20 and 23-29 are labels for the Set Preferences dialog

1 設定視區選項

2 開啟視區
3 依據名稱
4 依據名稱與圖像
5 依據名稱與小圖像
6 依據名稱，日期，大小 ...
7 顯示
8 依據單檔案夾
9 依據樹狀
10 次序
11 依據檔案類別
12 依據字母順序
13 依據日期
14 依據大小
15 方向
16 升序
17 降序
18 放置位置
19 排序後的網點
20 依放置位置

22 檔案管理者 -- 設定視區選項

23 表頭
24 圖像路徑
25 目前檔案夾
26 狀態行
27 儘檔案夾
28 檔案夾，然後檔案
29 檔案夾與檔案
30 內定值

31 表示方式
32 儘根據名稱
33 根據大圖像
34 根據小圖像
35 列與行
36 文字路徑
37 訊息行


$set 24
$set 25
$set 26



$set 27
$ ****** Module: Trash.c ****(developer comment only)********

$ Messages 3, 4, and 81 are titles for error warning dialogs
$ Message 5-8, 56, 58, 78, 82, and 83 are trash error messages
$ Message 9 is the title of the trash container
$ Messages 24/26 and 28/30 are the text and mnemonics for the File pulldown
$   menu
$ Message 73 is an error dialog title
$ Messages 74, 75, 79, and 80 are trash warning messages

3 檔案垃圾檔錯誤
4 檔案垃圾檔警告

5    下列檔案或檔案夾無法被放在垃圾檔內：     \n
6    下列為非空檔案夾，    \n   \
或您沒有寫入許可權的檔案。    \n   \
您要繼續嗎?\n
7    您對以下檔案沒有寫入許可權，    \n   \
您要繼續嗎?\n
8    下列檔案夾都不是空的。    \n   \
您要繼續嗎?\n

9 檔案垃圾檔

24 放回(P)
26 P
28 清除垃圾檔(h)
30 h

56    下列檔案無法被放回：\n
$ Message 58 supplements Message 56
58 (另加 %d 個額外的檔案)

73 Shred File(s)

74 以下檔案亦在工作空間中：\n\
   %s\n\
將該檔案移至垃圾檔中會將其自\n\
工作空間中除去。
75 下列檔案亦在工作空間中：\n\
   %s\n\
將該檔案移至垃圾檔中會將其自\n\
工作空間中除去。

78 檔案管理者不允許檔案夾或\n\
檔案的名稱中有空白，\n\
所以我們不能刪除他們。\n

79 工作空間檔案 %s 亦在您的檔案系統中，它位於：\n\
   %s\n\
將該檔案移至垃圾檔中會將其\n\
自檔案系統中除去。

80 工作空間物件亦在您的檔案系統中，它位於：\n\
   %s\n\
將該檔案移至垃圾檔中會將其\n\
自檔案系統中除去。

81 Shred File/Folder 錯誤

82 下列檔案無法自檔案系統中徐去：   \n

83 $HOME/.dt/Trash 目錄中的 .trashinfo 檔無法開啟。\n\
   您必須存取該檔，以使垃圾檔能工作。\n\
   請檢察官於該檔案及檔案夾的許可權。\n
84 一旦垃圾檔中的物件被清除，他們就永遠\n\
   自您的系統中消失了。\n\n\
   您確定要清除垃圾檔嗎?\n
85 您正在刪除的工作空間物件是一個位於 %s\n\
   檔案管理者中的連結複本。\n\
   若您按下"確定"，該連結與原使物件都會被刪除。\n\
   若您只要刪除連結，\n\
      1) 請在對話框中按下"取消"\n\
      2) 然後在該連結物件上按下功能表鈕，以顯現'工作空間物件'功能表。\n\
      3)  然後選擇"自工作空間中除去"
86 您正在刪除的物件是連結到位於 %s\n\
   的檔案管理者中的原始物件。\n\
   若您按下"確定"，該連結複本與原始物件都會被刪除。\n\
   若您只要刪除工作空間物件，\n\
      1) 請在對話框中按下"取消"\n\
      2) 然後由工作空間物件的即現式功能表中,選擇"自工作空間中除去"
87 垃圾檔中的物件無法開啟。\n\n\
   若要開啟該物件，請使用"放回"將該物件放回檔案管理者，\n\
   然後在那兒將之開啟。

88 無法寫入暫時性檔案中。\n\
   也許您的檔案系統已經滿了。

89 無法開啟垃圾檔目錄\n\
   %s\n\
   (認證，存取許可權問題)\n\n\
   所有的垃圾檔作業將不會被執行。

90 檔案垃圾檔無法做起始設定。\n\
   請檢查：\n\
        1)$HOME/.dt/Trash 以獲得關於許可權的資訊。\n\
        2)可用的磁碟空間。\n\
        3)認證。

$ 91 is not used
91 A linked copy of the following object is on\n\
the Workspace backdrop:\n\
Moving the object to the trash will remove it from\n\
both the File Manager and Workspace backdrop.

$ 92 is not used
92 A linked copy of the following objects are also on\n\
the Workspace backdrop:\n\
Moving the objects to the trash will remove them from\n\
both the File Manager and Workspace backdrop.

93 無法存取關於垃圾檔資訊的檔案\n\
   %s\n\
   所有的垃圾檔作業將不會被執行。\n\
   最普遍的原因是：\n\
     -- 網路認證問題。\n\
     -- 磁碟空間不夠。\n\
     -- 錯誤的許可權 $HOME/.dt/Trash。

94 檔案管理者不允許物件\n\
名稱中有空白，\n\
所以我們不能刪除它們。\n\
若要刪除這些物件：\n\
  -- 請自"檔案管理者"功能軸中選取"開啟終端機"\n\
  -- 請下除去命令來刪除他們。

95 無法存取垃圾檔目錄：\n\
   %s\n\
   所有的垃圾檔作業將不會被執行。\n\
   最普遍的原因是：\n\
     -- 網路認證問題。\n\
     -- 磁碟空間不足。\n\
     -- 許可權不對。

96 下列物件無法自檔案系統中除去：   \n

97 下列物件無法放在垃圾檔內：     \n

98 (加上 %d 個額外的物件)

$ message 99 is not used
99 您對下列檔案沒有寫入許可權：\n\
您要繼續嗎?\n

100 下列檔案夾都不是空的：\n\
您要繼續嗎?\n

101 下列物件無法被放回：\n

102 以下物件的一個已連結的複製檔是在\n\
工作空間背景定位上：\n\
%s\n\
將該物件移至垃圾檔中會將其\n\
同時自檔案管理者與工作空間背景定位中除去。

103 下列物件的一個已連結的複製檔是在\n\
工作空間背景定位上：\n\
%s\n\
將該物件移至垃圾檔中會將其\n\
同時自檔案管理者與工作空間背景定位中除去。

104 您對下列檔案沒有寫入許可權：\n\
您要繼續嗎?\n

105 某個垃圾物件的內定動作無法執行，\n\n\
    要執行這個垃圾物件的內定動作，\n\
    請使用「放回」讓它返回檔案管理者，\n\
    再於該處執行。

106 關於檔案垃圾檔

107 "下列檔案夾中至少有一個物件無法刪除。\n\n\
%s\n\n\
可能因為沒有刪除該物件 \n\
的權限。"

108 "無法刪除下列檔案。\n\n\
%s\n\n\
可能因為沒有刪除該物件 \n\
的權限。"

109 "Put Back 警告"

110 "有個物件稱為\n\n\
    %s\n\n\
如果要以垃圾檔的物件，\n\
取代此物件，請選取確定。\n\n\
如果要保留此物件，請選取取消。\n\
如果要更改物件名稱，將可以\n\
重複 Put Back 及保有兩個物件。"

111 "Put Back 錯誤"

112 "無法刪除下列物件。\n\
可能因為這些物件\n\
已經被刪除。\n\n"

113 "下列檔案夾至少有一個物件，或 \n\
檔案夾本身無法刪除。\n\
可能因為沒有刪除\n\
物件的權限。\n\n"

114 "無法刪除檔案。可能因為 \n\
沒有刪除檔案的權限。\n\n"

115 "無法刪除下列物件。\n\
缺少該物件桌上管理系統就無法運作。\n\n"

$set 28
$ ****** Module: Desktop.c ****(developer comment only)********

$ Messages 1-3 Workspace error messages
$ Message 4 is the title for the Workspace popup menus
$ Messages 5-8 are the labels and mnemonics for the Workspace obj popup menu

1 已有檔案以此名稱存在工空間中。\n\
請選擇不同的名稱。
2 已有檔案以此名稱存在原始檔案夾中\n\
(該檔案所來自的檔案夾).\n\
請選擇不同的名稱。
3 名稱為 %s 的檔案已存在工作空間中。\n\
您不能將同名的另一個檔案放入工作空間。\n\
若要如此，請將其中之一重新命名。

4 工作空間物件

5 自工作空間中除去(R)
6 R
7 開啟母檔案夾(O)
8 O

9 已有物件以此名稱存在工空間中。\n\
請選擇不同的名稱。

10 已有物件以此名稱存在原始檔案夾中\n\
(該物件所來自的檔案夾).\n\
請選擇不同的名稱。

11 名稱為 %s 的物件已存在工作空間中。\n\
您不能將同名的另一個物件放入工作空間。\n\
若要如此，請將其中之一重新命名。

12 "越過不合法的工作空間。內定工作空間仍將是\n\
一種工作空間及物件稱為:\n\
   %s\n\
已存在工作空間的背景定位。\n\
無法將其他同名物件放入此工作空間。\n\
若要將兩同名物件放入此工作空間，請將其中一個更名。"

13 "物件:\n\
   %s\n\
不在此檔案系統中。"

$set 29
$ ****** Module: Help.c ****(developer comment only)********

$ Message 1 is the title for the dtfile help dialog
$ Messages 1 and 2 are titles for the Help dialog, Message 2 is
$   used when the user has specified a title for dtfile windows
$ Message 3 is the text for help on the "Open New View" action
$ Message 4 is the text for help on the "Open In Place" action
$ Messages 5-6 are help error messages

1 檔案管理者輔助說明
2 %s 輔助說明

3 `%s` 動作為被要求的檔案夾開啟了一個新的檔案管理者視區。

4 `%s` 動作將被要求的檔案夾顯示於目前的檔案管理者視區。

5 沒有針對 %s 動作的輔助說明
6 沒有針對 %s 檔案類別的輔助說明

7 `%s` 動作為被要求的檔案夾開啟了一個新的\n\檔案管理者視區視窗。

8 `%s` 動作將被要求的檔案夾顯示於目前的\n\檔案管理者視區視窗。

9 檔案垃圾檔輔助說明

10 "多重選取物件沒有輔助說明。"

$set 30
$ ****** Module: SharedMsgs.c ****(developer comment only)********

$ The following messages are used in more than one place within the
$ dtfile source.  Unless otherwise specified, they are error messages.

1 無法自 %s 讀取

$ Message 2 is used to format the file attributes in 'by attributes' mode
2 %b %d %H:%M:%S %Y

3 無法將檔案 %s 自主機 %s 移動或複製。\n\
主機 %s 可能在您的檔案系統中裝設得不恰當。
4 無法將檔案 %1$s 自主機 %2$s 移動或複製。\n\
主機 %3$s 可能在您的檔案系統中裝設得不恰當。
5 無法讀取 %s
6 無法寫入檔案夾 %s
7 無法建立 %s
8 無法刪除 %s
9 名稱為\n\
  %s\n\
  的檔案或檔案夾已存在且不能被重寫。\n\
  請鍵入不同的名稱。

$ Message 10 is the help topic string for help on a file icon or action
10 檔案類別/動作的敘述

11 您不能將路徑指定為檔案名稱的一部份。
12 無效的尋找資訊。\n\n\
以下檔案夾不存在，或無法存取：\n\n\
     %s\n

$ Messages 13-16 are dialog titles
13 %s 概觀
14 %s 指導
15 關於 %s
16 檔案管理者

$ Message 18 is appended to ".." folders
18 (往上一層)

$ Messages 19 and 20 are messages which appear when a user clicks item help
$ outside the application.  19 is the title, while 20 is the message.
19 關於項目輔助說明錯誤
20 您必須在檔案管理者中\n\
選擇一元件。

$ Messages 21 and 22 are titles for error dialogs
21 檔案移動錯誤
22 檔案連結錯誤
23 無法建立 '%s'\n\n\
最普遍的原因是\n\
您對該相關檔案或檔案夾\n\
沒有正確的許可權。\n\n\
若要檢視許可權， 請選取該物件，\n\
然後選取"更改許可權...'

24 您必須在檔案管理者中選取一個物件。\n\
"關於項目" 輔助說明只有在您啟動的應用程式中\n\
有作用。

25 無法建立 '%s'\n\n\
最普遍的原因是\n\
您對該相關檔案或檔案夾\n\
沒有正確的許可權。\n\n\
若要檢視許可權， 請選取該物件，\n\
然後自即現功能表中選取"更改許可權...'

26 您必須在應用程式管理者中選取一個物件。\n\
"關於項目" 輔助說明只有在您啟動的應用程式中\n\
有作用。


$set 31

$set 32
$ ****** Module: dterror.src ****(developer comment only)********
$ Message 1 is a title for dterror
1 dt 錯誤
2 檔案管理者開啟目錄錯誤 

$set 33
1 多重物件
2 目前檔案夾

$quote "
3 "移動物件錯誤"

4 "複製物件錯誤"

5 "連結物件錯誤"

6 "嘗試移入:\n\n\
   %s\n\n\
不存在檔案系統中。"

7 "嘗試複製到:\n\n\
   %s\n\n\
不存在檔案系統中。"

8 "嘗試連結到:\n\n\
   %s\n\n\
不存在檔案系統中。"

9 "嘗試移動到:\n\n\
   %s\n\n\
不是檔案夾。"

10 "嘗試複製到:\n\n\
   %s\n\n\
不是檔案夾。"

11 "嘗試連結到:\n\n\
   %s\n\n\
不是檔案夾。"

12 "嘗試移動的物件:\n\n\
   %s\n\n\
不存在檔案系統中。"

13 "嘗試移動的物件:\n\n\
   %s\n\n\
不存在檔案系統中。\n\
不移動任何一個。"

14 "嘗試複製的物件:\n\n\
   %s\n\n\
不存在檔案系統中。"

15 "嘗試複製的物件:\n\n\
   %s\n\n\
不存在檔案系統中。\n\
不複製任何一個。"

16 "嘗試連結的物件:\n\n\
   %s\n\n\
不存在檔案系統中。"

17 "嘗試連結的物件:\n\n\
   %s\n\n\
不存在檔案系統中。\n\
不連結任何一個。"


$set 99
$ ****** Version Information ********
$ DO NOT TRANSLATE THESE MESSAGES
$ DO NOT CHANGE THESE MESSAGES
$ DO NOT LOCALIZE THESE MESSAGES
$ These messages are used for the version information.
1 "@(#)version_goes_here"
2 "\n@(#)_DtMessage catalog source $TOG: dtfile.msg /main/6 1998/04/29 15:13:40 rafi $"

