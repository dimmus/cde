$ $XConsortium: dtksh.msg /main/3 1996/07/24 19:02:59 drk $
$set 1
1 DtkshCvtStringToPixel：色彩 '%s' 未定義

$set 5
1 指定了太多資源參數；跳過 '%s'
2 資源規格有誤；格式應為 'name:value'：%s
3 資源分隔元為 ':'，但您使用 '='：%s
4 工具組已起始設定
5 用法：XtInitialize variable applicationName applicationClass [args ...]
6 用法：%s variable name class parent [arg:val ...]
7 用法：XtCreateApplicationShell variable name class [arg:val ...]
8 用法：XtPopup widget GrabNone|GrabNonexclusive|GrabExclusive
9 無法辨識抓取類型 '%s'；因此使用 'GrabNone'
10 用法：XtIsShell widget
11 用法：XtMapWidget widget
12 用法：XtUnmapWidget widget
13 用法：%s variable widget
14 用法：XtScreen variable widget
15 用法：XtWindow variable widget
16 用法：XtCallCallbacks widget callbackName
17 用法：XtHasCallbacks variable widget callbackName
18 用法：XtAddCallback widget callbackName ksh-command
19 用法：XtRemoveCallback widget callbackName ksh-command
20 以下是無效的回叫處理編號：%s
21 指定的回叫處理未註冊
22 用法：XtAddEventHandler widget mask nonMaskable ksh-command
23 用法：XtRemoveEventHandler widget mask nonMaskable ksh-command
24 指定的事件處理處理器未編號
25 用法：XtGetValues widget resource:variable ...
26 以下的資源參數格式錯誤：%s
27 用法：XtSetValues widget arg:val ...
28 用法：XtAddWorkProc variable command
29 用法：XtRemoveWorkProc workProcId
30 workProcId 參數必須為十六進位數：%s
31 用法：XtAddTimeOut variable milliseconds command
32 毫秒參數必須大於零
33 用法：XtRemoveTimeOut intervalId
34 intervalId 參數必須為十六進位數：%s
35 用法：%s widget ...
36 用法：XBell display volume
37 用法：%s variable screen
38 螢幕參數無效：%s
39 用法：XDefineCursor display window cursorId
40 cursorId 參數無效：%s
41 用法：XUndefineCursor display window
42 用法：XtRemoveAllCallbacks widget callbackName
43 用法：XTextWidth variable fontName  string
44 用法：%s display drawable [args ...]
45 drawable 參數無效：%s
46 destination 參數無效：%s
47 圖形函數名稱無法辨識：%s
48 線條樣式無法辨識：%s
49 繪圖選項無法辨識：%s
50 有一些剩餘的點被忽略
51 用法：XCopyArea display source dest sourceX sourceY width height destX destY [args ...]
52 無法將資源類型 '%s' 轉換成 'String'
53 無法將資源類型 'String' 轉換成 '%s' 類型
54 用法：XtAddInput variable [-r] fileDescriptor kshCommand
56 用法：XtRemoveInput inputId
57 inputId 參數必須為十六進位數：%s
59 用法：XFlush display
60 用法：XSync display discard
61 用法：XRaiseWindow display window
62 用法：%s widget [True|False]
63 用法：%s widget translations
64 用法：XtUninstallTranslations widget
65 用法：XtParent variable widget
66 用法：XtLastTimestampProcessed variable display
67 用法：DtSessionRestorePath widget pathVariable saveFile
68 用法：DtSessionSavePath widget pathVariable fileVariable
69 用法：DtShellIsIconified widget
70 用法：DtSetStartupCommand widget command
71 用法：DtSetIconifyHint widget boolean
73 rootWindow 參數無效：%s
74 用法：DtWsmSetCurrentWorkspace widget atom
75 工作空間元素無效：%s
78 用法：DtWsmSetWorkspacesOccupied display window workspaceList
79 用法：DtGetHourGlassCursor variable display
80 用法：DtWsmAddCurrentWorkspaceCallback variable widget ksh-command
81 用法：DtWsmRemoveWorkspaceCallback handle
82 用法：XtNameToWidget variable referenceWidget names
83 用法：DtDbLoad
84 用法：DtDbReloadNotify ksh-command
85 用法：DtActionExists actionName
86 用法：DtActionLabel variable actionName
87 用法：DtActionDescription variable actionName
88 用法：DtActionInvoke widget actionName termParms execHost contextDir useIndicator ksh-command ["FILE" fileName] ...
89 用法：DtDtsLoadDataTypes
90 用法：DtDtsFileToDataType variable fileName
91 用法：DtDtsFileToAttributeValue variable fileName attrName
92 用法：DtDtsFileToAttributeList variable fileName
93 用法：DtDtsDataTypeToAttributeValue variable dataType attrName optName
94 用法：DtDtsDataTypeToAttributeList variable dataType optName
95 用法：DtDtsFindAttribute variable name value
96 用法：DtDtsDataTypeNames variable
97 用法：DtDtsSetDataType variable fileName dataType override
98 用法：DtDtsDataTypeIsAction dataType
99 用法：ttdt_open variable status variable2 toolname vendor version sendStarted
100 用法：tttk_Xt_input_handler procId source id
101 資源參數必須為整數：%s
102 id 參數必須為十六進位數：%s
103 用法：ttdt_close status procId newProcId sendStopped
104 用法：ttdt_session_join variable status sessId shellWidgetHandle join
105 用法：ttdt_session_quit status sessId sessPatterns quit
106 sessPatterns 參數無效：%s
107 用法：ttdt_file_event status op patterns send
108 用法：ttdt_file_quit status patterns quit
109 用法：ttdt_Get_Modified pathName scope timeout
110 用法：%s status pathName scope timeout
112 用法：tt_error_pointer variable ttStatus
113 用法：%s status msg
114 用法：%s status msg msgStatus msgStatusString destroy
115 用法：ttdt_file_join variable status pathName scope join ksh-command
116 用法：tt_file_netfile variable status filename
117 用法：tt_netfile_file variable status netfilename
118 用法：tt_host_file_netfile variable status host filename
119 用法：tt_host_netfile_file variable status host netfilename
120 用法：XtIsSubclass widget class
121 %s 不是有效的 widget 類別名稱
122 用法：XtClass variable widget

$set 6
1 DtkshCvtWindowToString：'from' 值的大小無效
2 DtkshCvtHexIntToString：'from' 值的大小無效
3 DtkshCvtIntToString：'from' 值的大小無效
4 DtkshCvtBooleanToString：'from' 值的大小無效
5 DtkshCvtStringToWidget：'from' 值的大小無效
6 DtkshCvtStringToWidget：Unable to find a widget named '%s'
7 DtkshCvtStringToCallback：'from' 值的大小無效
8 DtkshCvtCallbackToString：'from' 值的大小無效
9 DtkshCvtWidgetToString：'from' 值的大小無效
10 DtkshCvtWidgetToString：無法找到 widget 的名稱
11 DtkshCvtWidgetClassToString：'from' 值的大小無效
12 DtkshCvtWidgetClassToString：未知的 widget 類別
13 DtkshCvtStringToWidgetClass：未知的 widget 類別名稱
14 DtkshCvtScreenToString：'from' 值的大小無效

$set 11
1 無法找到欄位名稱 '%s' 在 '%s' 結構中
2 DtkshCvtCallbackToString：內部轉換緩衝區溢位
3 資源 '%s' 拼湊 (Hashing) 失敗，在 widget 類別 '%s' 中
4 未提供功能名稱
5 命令無法執行；工具組尚未起始設定
6 widget '%s' 建立失敗
7 顯示參數無效：%s
8 以下特性元素無效：%s
9 widget '%s' 未定義以下資源：%s
10 指定的位置無效：%s
11 指定的字形無效：%s
12 widget 處理編號 '%s' 未參照到現存的 widget
13 widget 必須是一個 'command' widget
14 widget 必須是一個 'mainWindow' widget
15 widget 必須是一個 'scale' widget
16 widget 必須是一個 'scrollBar' widget
17 widget 必須是一個 'toggleButton' widget 或 gadget
18 視窗參數無效：%s
19 無法配置必要的記憶體，結束中
20 無法找到母 widget
21 無法找到符號 '%s'
22 無法找到類型 '%s'
23 無法解析宣告 '%s'；因此使用 'unsigned long'
24 未知的子類型：%s
25 選項旗標無法辨識：%s
26 用法：%s widget position
27 用法：%s widget time
28 用法：%s widget
29 dtksh 錯誤
30 dtksh 警告
31 用法：%s display window
32 用法：%s display window variable
33 用法：%s display rootWindow variable
34 timeout 參數無效：%s
35 msg 參數無效：%s
36 patterns 參數無效：%s

$set 14
1 widget 起始設定時，內部拼湊表失敗；結束中
2 無法找到名為 '%s' 的 widget 類別
3 識別字 '%s' 不是有效的 widget 處理編號
4 無法找到 widget 類別
5 用法：DtLoadWidget widgetClassName widgetClassRecordName
6 無法找到名為 '%s' 的 widget 類別記錄
7 widget 類別 '%s' 起始設定時，內部拼湊表失敗
8 \n%sRESOURCES FOR %s%s%s:\n
9 CONSTRAINT 
10 R
11 M
12 S
13 %-15s %-6s %-6s %-18s %-6s %s\n
14 ENV VARIABLE    HANDLE PARENT CLASS              STATUS NAME\n
15 \n用法：\tDtWidgetInfo [widgetHandle]\n\tDtWidgetInfo -r <widgetHandle|className>\n\tDtWidgetInfo -R <widgetHandle|className>\n\tDtWidgetInfo -c [className]\n\tDtWidgetInfo -h [widgetHandle]

$set 15
1 無法起始設定 Toolkit
2 以下資源無法在建立 widget\n時設定。請在建立後使用 XtSetValues 設定：%s
3 用法：%s variable parent name [argument:value ...]
4 用法：DtHelpReturnSelectedWidgetId variable widget variable
5 用法：DtHelpSetCatalogName catalogName
6 用法：DtHelpQuickDialogGetChild variable quickHelpWidget child
7 widget 必須是一個 'quickHelp' widget
8 widget 必須是一個 'list' widget
9 用法：%s widget position item
10 用法：%s widget position itemList
11 用法：%s widget item
12 用法：XmListDeleteItemsPos widget count position
13 用法：XmListDeleteItems widget itemList
14 用法：XmListDeletePositions widget positionList
15 用法：XmListGetKbdItemPos variable widget
16 用法：XmListItemExists widget item
17 用法：XmListItemPos variable widget item
18 用法：XmListPosSelected widget position
19 用法：XmListPosToBounds widget position variable variable variable variable
20 用法：%s widget position notifyFlag
21 用法：%s widget item notifyFlag
22 用法：XmListSetAddMode widget boolean
23 用法：XmListSetKbdItemPos widget position
24 用法：XmMainWindowSetAreas mainwindow menu command hscroll vscroll work
25 'mainWindow' 處理編號是 NULL
26 用法：%s variable mainwindow
27 用法：XmProcessTraversal widget direction
28 未知的移動方向：%s
29 用法：XmInternAtom variable display name onlyIfExists
30 用法：XmGetAtomName variable display atom
31 指定的元素無效：%s
32 用法：XmGetColors widget background foreground topshadow bottomshadow select
33 背景圖像單元無效：%s
34 用法：XmUpdateDisplay widget
35 用法：%s widget protocol [protocol ...]
36 指定的元素無效：%s
37 用法：XmAddWMProtocolCallback widget protocol ksh-command
38 用法：XmRemoveWMProtocolCallback widget protocol ksh-command
39 用法：XmMenuPosition menu event
40 指定的事件無效：%s
41 用法：XmCommandAppendValue commandWidget string
42 用法：XmCommandError commandWidget errorMessage
43 用法：XmCommandSetValue commandWidget command
44 用法：XmCommandGetChild variable commandWidget child
45 用法：XmMessageBoxGetChild variable commandWidget child
46 widget 必須是一個 'messageBox' widget
47 用法：XmFileSelectionBoxGetChild variable widget child
48 widget 必須是一個 'file selection box' widget
49 用法：XmSelectionBoxGetChild variable widget child
50 widget 必須是一個 'selection box' widget
51 用法：XmScaleGetValue scaleWidget variable
52 用法：XmScaleSetValue scaleWidget value
53 指定的比例尺值無效：%s
54 用法：XmScrollBarGetValues scrollbar variable variable variable variable
55 用法：XmScrollBarSetValues scrollbar value sliderSize increment pageIncrement notify
56 指定的值無效：%s
57 指定的滑動器大小無效：%s
58 指定的增加量無效：%s
59 指定的頁碼增加量無效：%s
60 用法：XmScrollVisible scrolledWin widget leftRightMargin topBottomMargin
61 widget 必須是一個 'scrolledWindow' widget
62 要顯示的 widget 不存在。
63 指定的左/右邊界無效：%s
64 指定的上/下邊界無效：%s
65 用法：%s widget state notify
66 用法：catopen variable catName
67 用法：catclose catId
68 用法：catgets variable catId setNum msgNum dftMsg
69 widget 必須是一個 'text' 或 'textField' widget
70 用法：%s variable widget
71 用法：%s widget boolean
72 指定的參數無效：%s
73 用法：%s widget lines
74 用法：%s widget maxLength
75 用法：%s widget string
76 用法：%s widget variable variable
77 用法：%s widget position string
78 用法：%s widget position variable variable
79 用法：%s widget fromPosition toPosition string
80 指定的 'from' 位置無效：%s
81 指定的 'to' 位置無效：%s
82 用法：%s widget firstPosition lastPosition time
83 指定的第一個位置無效：%s
84 指定的最後一個位置無效：%s
85 指定的時間無效：%s
86 用法：%s variable widget x y
87 指定的 x 位置無效：%s
88 指定的 y 位置無效：%s
89 用法：%s widget left right mode
90 指定的左邊位置無效：%s
91 指定的右邊位置無效：%s
92 用法：%s widget start string direction variable
93 指定的起始位置無效：%s
94 用法：XmListGetSelectedPos variable widget
95 用法：XmListGetMatchPos variable widget item
96 用法：XmOptionLabelGadget variable widget
97 用法：XmOptionButtonGadget variable widget
98 用法：XmGetVisibility variable widget
99 用法：XmGetTearOffControl variable widget
100 用法：XmGetTabGroup variable widget
101 用法：XmGetPostedFromWidget variable widget
102 用法：XmGetFocusWidget variable widget
103 用法：XmFileSelectionDoSearch widget directoryMask

$set 16
1 DtkshCvtNamedValueToString：找不到符合者
2 DtkshCvtStringToNamedValue：無法轉換字串 '%s'


$ The following messages are for ksh93 itself.  The message numbers must
$ match those specified in the MsgStr array allmsgs in 
$ ksh93/src/cmd/ksh93/sh/init.c, as must the set number.
$set 25

1  完成
3  執行中
4  [-n] [arg...]
5  [arg...]
6  [dir] [list]
7  [job...]
8  [n]
9  [name [pathname] ]
10  [name]
11  [top] [base]
12  expr...
13  format [arg...]
14  是一個函數
15  是一個關鍵字
16  是一個內建 shell
17  是一個轉存函數
18  是一個未定義的函數
19  name [arg...]
20  optstring name [args...]
21  秒
23 ${HOME:-.}/.profile
24 %c：在表示式中有無效的字元 - %s
25 %c：未知的格式指定元
26 %d-%d：無效的範圍
27 %d：無效的二元命令集版本
28 %s 是一個別名，屬於 
29 %s 是一個輸出的別名，屬於
30 缺少 %s
31 %s 未知的基底
32 %s：'?' 運算子應使用 ':'
33 %s：有歧義
34 %s：引數應為 %job 或處理 ids
35 %s：找不到別名\n
36 %s：算數語法錯誤
37 %s：指定需要 lvalue
38 %s：檔案單元號碼錯誤
39 %s：格式錯誤
40 %s：號碼錯誤
41 %s：選項錯誤
42 %s：代換錯誤
43 %s：設陷錯誤
44 %s：無法建立
45 %s：無法執行
46 %s：無法開啟
47 %s：除數為零
48 %s：領域例外
49 %s：無法執行完成 %s
50 %s：檔案已存在
51 %s：不合法的函數名稱
52 %s：無效的別名
53 %s：無效的規則 (discipline) 函數
54 %s：無效的轉存名稱
55 %s：無效的函數名稱
56 %s：無效的名稱
57 %s：無效的一般表示式
58 %s：無效的自我參照
59 %s：無效的使用：
60 %s：無效的變數名稱
61 %s：不是一個識別字
62 %s：只能讀取
63 %s：標籤尚未製作
64 %s：超出限制
65 %s：應有更多符記
66 %s：沒有上層節點
67 %s：沒有參照名稱
68 %s：無法找到
69 %s：未製作
70 %s：運算元類型不相容
71 %s：溢位例外
72 %s：未設定參數
73 %s：參數空白或未設定
74 %s：遞迴太深
75 %s：參照變數不能是陣列
76 %s：需要路徑名稱引數
77 %s：有限制
78 %s：單數例外
79 %s：註標超出範圍
80 %s：括弧不對稱
81 %s：未知的函數
82 %s：未知的語言別
83 %s：未知的運算子
84 %s：未知的訊號名稱
85 %s：將引起迴路
86 (磁蕊傾出)
87 -c 需要引數
88 -e - 需要單一引數
89 /vpix
90 <未知的命令>
91 ACE#?F#?H:[name]L#?R#?Z#?fi#?[base]lnprtux [name=[value]...]
92 AE#?F#?HL#?R#?Z#?fi#?[base]lnprtux [name=[value]...]
93 中斷
94 Ad:[delim]prst#[timeout]u#[filenum] [name...]
95 警告呼叫
96 根節點規格錯誤
97 系統呼叫錯誤
98 管道中段
99 匯流排錯誤
100 無法啟動工作控制
101 現行選項設定
102 DIL 訊號
103 子節點停止運作
104 DircabefhkmnpstuvxCI:[file]o:?[option]A:[name] [arg...]
105 DircabefhkmnpstuvxCo:?[option]A:[name] [arg...]
106 EMT 設陷
107 超出 CPU 時間限制
108 超果檔案大小限制
109 浮動例外
110 HSacdfmnstv [limit]
111 中止
112 IO 訊號
113 不合法的指令
114 岔斷
115 清除
116 LP [dir] [change]
117 記憶體錯誤
118 移轉處理
119 沒有工作控制
120 電話岔斷
121 輪詢警告
122 電源中斷
123 時間警告側錄
124 結束
125 資源遺失
126 回復成舊的 tty 驅動器...
127 S [mask]
129 安全標籤已改變
130 插座岔斷
131 聲音完成
132 停止 (訊號)
133 停止 (tty 輸入)
134 已停止的處理現在繼續
135 停止
136 停止 (tty 輸出)
137 轉換成新的 tty 驅動器...
138 系統即將當掉
139 終止
140 追蹤/BPT 設陷
141 無法辦識的版本
142 請使用 'exit' 來停止這個 shell
143 使用者訊號 1
144 使用者訊號 2
145 版本未定義
146 虛擬時間警告
147 視窗大小改變
148 您有工作正在執行
149 您有中止的工作
150 [_[:alpha:]]*([_[:alnum:]])
151 \n@(#)Version 12/28/93\0\n
152 \n@(#)Version M-12/28/93\0\n
153 \n真實
154 \r\n\007由於未作用，shell 將在 60 秒內逾時
155 a name...
156 a:c [command [args...] ]
157 afpv name...
158 警告 %s %.3f\n
159 alarm -r %s +%.3g\n
160 應有引數
161 目錄錯誤
162 檔案單元號碼錯誤
163 代換錯誤
164 無法存取母目錄
165 無法建立管道
166 無法建立暫時檔案
167 無法產生子程序
168 無法取得 %s
169 無法設定 %s
170 無法設定警告聲
171 需要條件
172 dsf:[library] [name...]
173 e:[editor]lnrsN# [first] [last]
174 檔案結束
175 f:[format]nprsu:[filenum] [arg...]
176 fnv name...
177 hist -e "${VISUAL:-${EDITOR:-vi}}" 
178 記錄檔案未開啟
179 語法錯誤
180 類型 %c 的引數無效
181 是一個 shell 內建版本，屬於
182 是一個追蹤別名，屬於
183 清除
184 第 %d 行：$ 前面不是 \\
185 第 %d 行：${} 內的 %c 應該加引號
186 第 %d 行：%s 是未知的標籤
187 第 %d 行：[[...]] 中的 %s 現已廢棄不用，改用 ((...))
188 第 %d 行：'=' 現已廢棄不用，改用 '=='
189 第 %d 行：-a 現已廢棄不用，改用 -e
190 第 %d 行：%c 前面的 \\ 保留以備將來使用
191 第 %d 行：`...` 現已廢棄不用，改用 $(...)
192 第 %d 行：%c 轉義以避免產生歧義
193 第 %d 行：忽略標籤 %s
194 第 %d 行：請將 %c 加上引號，以避免產生歧義
195 第 %d 行：set %s 現已廢棄不用
196 第 %d 行：嵌套次 shell 需加入空格
197 第 %d 行：使用大括弧括住，以免 $id[...] 產生歧義
198 第 %d 行：使用空格或跳格來分隔運算子 %c 與 %c
199 ln#[signum]s:[signame] sig...
200 禁止登入 setuid/setgid shell
201 對應
202 新行
203 nlp [job...]
204 沒有記錄檔
205 沒有查詢處理
206 沒有這個工作
207 沒有這個處理
208 未支援
209 關閉
210 開啟
211 超出開啟檔案限制
212 記憶體耗盡
213 p [action condition...]
214 p [name[=value]...]
215 參數未設定
216 不具許可權
217 此處理已存在
218 ptx [name=[value]...]
219 pvV name [arg]...
220 r [varname seconds]
221 第 %d 行語法錯誤：`%s' %s
222 第 %d 行語法錯誤：標籤 %s 重複
223 語法錯誤：`%s' %s
224 sys
225 等待輸入逾時
226 非期待中
227 領域無法存取
228 未限制
229 不符合
230 使用者
231 版本
232 寫入 %d 失敗
233 在 $_ 有您的郵件
234 零位元組
