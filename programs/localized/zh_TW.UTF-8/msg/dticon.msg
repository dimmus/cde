$ $XConsortium: dticon.msg /main/3 1995/11/08 14:06:53 rswiston $
$ *************************************<+>*************************************
$ *****************************************************************************
$ **
$ **  File:        dticon.msg
$ **
$ **  Project:     HP DT Icon Editor
$ **
$ **  Description:
$ **  -----------
$ **  This file is the source for the message catalog for dticon
$ **
$ **
$ *****************************************************************************
$ **
$ **     (c) Copyright 1992, 1993, 1994 Hewlett-Packard Company
$ **     (c) Copyright 1993, 1994 International Business Machines Corp.
$ **     (c) Copyright 1993, 1994 Sun Microsystems, Inc.
$ **     (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
$ **         Novell, Inc.
$ **
$ **     All Rights reserved
$ **
$ **
$ *****************************************************************************
$ **

$set 2
$ *****************************************************************************
$ ** _DtMessage set for fileIODialog.c
$ **
$ ** messages 10-16 are for the FileSelectionBox widget (default labels)
$ *****************************************************************************

2 圖像編輯器 -- 檔案輸入/輸出
4 圖像編輯器 -- 檔案輸入/輸出
6 圖像編輯器 -- 儲存為
8 圖像編輯器 -- 開啟檔案
10 更新
12 檔案夾
14 檔案
16 輸入檔名：
17 輸入檔名：
18 更新
20 未定標題
22 開啟
24 儲存

$set 4
$ *****************************************************************************
$ ** _DtMessage set for newIconDialog.c
$ **
$ *****************************************************************************

2 新圖像對話框
4 圖像編輯器 -- 重新更改大小
6 確定
8 取消
10 輔助說明
12 新寬度：
14 新高度：

$set 6
$ *****************************************************************************
$ ** _DtMessage set for queryDialog.c
$ **
$ *****************************************************************************

2 圖像編輯器 -- 警告
6 確定
$ ** DO NOT LOCALIZE - the user should never see this message
8 Do ya really wanna?
10 取消

$set 8
$ *****************************************************************************
$ ** _DtMessage set for stdErrDialog.c
$ **
$ *****************************************************************************

2 圖像編輯器 -- 錯誤
4 繼續

$set 10
$ *****************************************************************************
$ ** _DtMessage set for utils.c
$ **
$ *****************************************************************************

$ ** DO NOT LOCALIZE messages 2-48, they appear in the errorlog only
2 Cannot initialize button icon for circle
4 Cannot initialize button icon for solid circle
6 Cannot initialize button icon for polyline
8 Cannot initialize button icon for polygon
10 Cannot initialize button icon for solid polygon
12 Cannot initialize button icon for ellipse
14 Cannot initialize button icon for solid ellipse
16 Cannot initialize button icon for eraser
18 Cannot initialize button icon for line
20 Cannot initialize button icon for flood
22 Cannot initialize button icon for point
24 Cannot initialize button icon for rectangle
26 Cannot initialize button icon for solid rectangle
28 Cannot initialize button icon for select
30 Unable to parse static color no.
32 Unable to allocate static color no.
38 No memory(0)
40 No memory(1)
42 No memory(2)
44 No memory(3)
46 No memory(4)
48 No memory(5)

50 無法初使化應用程式圖像儲存器
52 '-f' 參數之無效使用
54 '-i' 參數之無效使用
56 '-session' 參數之無效使用
58 無效的大小規格
60 螢幕影像抓取失敗
62 作業失敗。\n色彩圖已滿
64 無效的範圍參數

66 (未命名)


$set 12
$ *****************************************************************************
$ ** _DtMessage set for dtIconShell.c
$ **
$ *****************************************************************************

1 圖像編輯器
2 雙色
3 圖像編輯器 -- (未命名)
4 歡迎使用圖像編輯器。
6 填入實體
8 靜態色彩
10 動態色彩
12 靜態灰色
14 前景
16 背景
18 頂端陰影
20 底端陰影
22 選取
24 透明

$ ** message 26-198 are menu items with corresponding mnemonics
$ ** (for set grouping information, see the comment at the end-of-file)
26 N
28 清除(N)...
30 O
32 開啟(O)...
34 S
36 儲存(S)
38 A
40 儲存為(A)...
42 x
44 結束(x)
46 F
48 檔案(F)
50 U
52 復原(U)
54 t
56 剪取區域(t)
58 C
60 輔助區域(C)
62 P
64 粘貼區域(P)
68 L
70 左(L)
72 R
74 右(R)
76 R
78 旋轉區域(R)
82 V
84 垂直(V)
86 H
88 水平(H)
90 F
92 正反轉區域(F)
94 S
96 比例尺區域(S)
98 z
100 更改大小的圖像(z)...
102 A
104 增加作用點(A)
106 D
108 刪除作用點(D)
110 G
112 抓取螢幕影像(G)
114 e
116 清除圖像(e)
118 E
120 編輯(E)
122 V
124 可見網點(V)
128 B
130 XBM(B)
134 P
136 XPM(P)
138 O
140 輸出格式(O)
144 2x
146 3x
148 4x
150 5x
152 6x
154 8x
156 10x
158 12x
160 M
162 放大(M)
164 O
166 選項(O)
168 v
170 概觀(v)...
172 T
174 工作(T)...
176 R
178 參考資料(R)...
180 O
182 關於項目(O)
188 U
190 使用輔助說明(U)...
192 A
194 關於圖像編輯器(A)...
196 H
198 輔助說明(H)

$ ** message 200-are used in the "instruction" line for current drawing mode
200 目前編輯模式
202 (已填入)
204 鉛筆
206 填滿
208 線條
210 多重線
212 長方形
214 多邊形
216 圓
218 橢圓
220 消除
222 選取
224 無效

$set 14
$ *****************************************************************************
$ ** _DtMessage set for help.c
$ **
$ *****************************************************************************

2 圖像編輯器 -- 輔助說明
4 您必須在圖像編輯器內\n選擇一個項目。

$set 16
$ *****************************************************************************
$ ** _DtMessage set for process.c
$ **
$ *****************************************************************************

2 本檔案無法存取\n或包含無效的資料
4 無法將資料寫入檔案
6 這個特性尚無法使用
8 無先前的\n影像可用
10 指定了無效的\n寬度與/或高度
12 未選取區域
14 無區域在先前被\n剪取或複製
16 目前的圖像尚未儲存。\n\n您做的更改會漏失。
20 檢查檔案許可權
22 無法自檔案中回復階段檔案
24 無法將階段檔案存入檔案中

$ *****************************************************************************
$ ** End of _DtMessage Catalog File
$ ** (Comments only below this line)
$ *****************************************************************************
$ **
$ *****************************************************************************
$ ** Unique mnemonic groupings from set 12
$ *****************************************************************************
$ **
$ ** Unique Set 1:  (Main Menu Bar)
$ **
$ **   46 F
$ **   48 File
$ **
$ **   118 E
$ **   120 Edit
$ **
$ **   164 O
$ **   166 Options
$ **
$ **   196 H
$ **   198 Help
$ **
$ **
$ ** Unique Set 2:  (File Menu)
$ **
$ **   26 N
$ **   28 New...
$ **   30 O
$ **   32 Open...
$ **   34 S
$ **   36 Save
$ **   38 A
$ **   40 Save As...
$ **   42 x
$ **   44 Exit
$ **
$ **
$ ** Unique Set 3:  (Edit Menu)
$ **
$ **   50 U
$ **   52 Undo
$ **   54 t
$ **   56 Cut Area
$ **   58 C
$ **   60 Copy Area
$ **   62 P
$ **   64 Paste Area
$ **
$ **        Unique Set 3a:  (Rotate Sub-Menu)
$ **        68 L
$ **        70 Left
$ **        72 R
$ **        74 Right
$ **   76 R
$ **   78 Rotate Area
$ **
$ **        Unique Set 3b:  (Flip Sub-Menu)
$ **        82 V
$ **        84 Vertical
$ **        86 H
$ **        88 Horizontal
$ **   90 F
$ **   92 Flip Area
$ **   94 S
$ **   96 Scale Area
$ **   98 z
$ **   100 Resize Icon...
$ **   102 A
$ **   104 Add Hot Spot
$ **   106 D
$ **   108 Delete Hot Spot
$ **   110 G
$ **   112 Grab Screen Image
$ **   114 e
$ **   116 Clear Icon
$ **
$ **
$ ** Unique Set 4:  (Options Menu)
$ **
$ **   122 V
$ **   124 Visible Grid
$ **
$ **        Unique Set 4a:  (Output Sub-Menu)
$ **        128 B
$ **        130 XBM
$ **        134 P
$ **        136 XPM
$ **   138 O
$ **   140 Output Format
$ **   160 M
$ **   162 Magnification
$ **
$ **
$ ** Unique Set 5:  (Help Menu)
$ **
$ **   168 I
$ **   170 Introduction...
$ **   172 T
$ **   174 Tasks...
$ **   176 R
$ **   178 Reference...
$ **   180 O
$ **   182 On Item
$ **   188 U
$ **   190 Using Help...
$ **   192 V
$ **   194 Version...
$ **
