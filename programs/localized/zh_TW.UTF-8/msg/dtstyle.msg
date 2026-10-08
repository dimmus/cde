 
$ "$XConsortium: dtstyle.msg /main/4 1995/12/08 09:50:58 rswiston $"

$ *************************************<+>*************************************
$ *****************************************************************************
$ **
$ **  File:        dtstyle.msg
$ **
$ **  Project:     DT Style Manager
$ **
$ **  Description:
$ **  -----------
$ **  This file is the source for the message catalog for dtstyle
$ **
$ **
$ *****************************************************************************
$ **
$ **     (c) Copyright 1990 Hewlett-Packard Company
$ **     All Rights reserved
$ **
$ **
$ *****************************************************************************
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
$ 2. Messages that only appear in the DT error log file ($HOME/.dt/errorlog).
$ 
$     The localization of these messages is OPTIONAL.  These messages are
$     identified by the following:
$ 
$  MESSAGES xx-yy IN SET zz WILL ONLY APPEAR IN THE DT ERRORLOG FILE
$ 
$ 3. Messages that should not be localized.  
$ 
$     These messages are identified by the following:
$ 
$  DO NOT TRANSLATE or CHANGE or LOCALIZE MESSAGES xx-yy from set zz
$ 
$ ***** END (NOTE FOR MESSAGE CATALOG TRANSLATORS) *****
$ 
$ ******************************************************************************
$ *************************************<L>*************************************
$ **  -----------------GENERAL LOCALIZATION NOTES SECTION---------------------
$ * Comment lines begin with a $ except $set which indicates the start of a 
$ * new set.
$ *
$ * Do not delete any comments; you may add comments for your use.
$ *
$ * 
$ **  ----------------GENERAL LOCALIZATION NOTES SECTION END------------------
$ *************************************<L>*************************************

$set 2
$ Message 1 from set 2 was deleted from the message catalog

$ _DtMessage 2 is the title of all Information dialog boxes
2 通告
$ _DtMessage 3 is the title of all Error dialog boxes
3 錯誤
$ _DtMessage 4 WILL ONLY APPEAR IN THE DT ERRORLOG FILE
4 Cannot communicate with the session\n\
manager... Exiting.
 
$  MESSAGES 5-6 IN SET 2 WILL ONLY APPEAR IN THE DT ERRORLOG FILE
5 Style Manager is already running,\n\
second attempt aborted.
6 An X Toolkit error occurred...\n\
Exiting.

$ _DtMessage 7 is the title of all Help dialog boxes
7 外型式樣管理者 - 輔助說明

$ _DtMessage 8 is an error message that appears when you are using Itemh Help
$ and select on something outside of the Style Manager
8 您必須在外型式樣管理者\n\
中選擇一個項目。

$ _DtMessage 9 WILL ONLY APPEAR IN THE DT ERRORLOG FILE
9 Could not obtain screen saver information\n\
from the session manager. Start up settings\n\
may be incorrect.


$set 4
$ _DtMessage set 4 is for the Style Manager main window
$ Messages 6,8-26,34,39 from set 4 were deleted from the message catalog

$ _DtMessage 1 is the title of the main dtstyle window
1 外型式樣管理者

$ mnemonic and label for the File menubar item.
2 F
3 檔案(F)
$ mnemonic and label for the Help menubar item.
4 H
5 輔助說明(H)

$ _DtMessage 7 is the label for the Exit button in the File pulldown
$ the mnenomic for the Exit button is set 4 _DtMessage 40
7 結束

$ Messages 27-33,35  are the labels on the buttons in the dtstyle main window.
27 顏色
28 字形
29 背景定位
30 鍵盤
31 滑鼠
32 嗶聲
33 螢幕
35 啟動

$  MESSAGE 36 IN SET 4 WILL ONLY APPEAR IN THE DT ERRORLOG FILE
36 No Dtstyle restore information available for current session

$ _DtMessage 37 is the label on the dtstyle icon
37 外型式樣 

$ Message 38 is the label on the Window button in the dtstyle main window.
38 視窗

$ _DtMessage 40 is replaced with message 53
$ 40 E

$ *
$ * mnemonic and label for the Help menu items.
$ * 

$ Messages 41-42 have been replaced with messages 54-55
$ Overview... menu item
$ 41 I
$ 42 Introduction...

$ Tasks... menu item 
43 T
$ Message 44 has been replaced with messages 56
$ 44 Tasks...

$ Reference... menu item
$ 45 R
$ Message 46 has been replaced with messages 57
$ 46 Reference...

$ On Item menu item
$ Keybaord menu item
47 O
48 關於項目(O)

$ Using Help... menu item
49 U
$ Message 50 has been replaced with messages 58
$ 50 Using Help... 

$ Messages 51-52 have been replaced with messages 59-60
$ Overview menu item
$ 51 V
$ 52 Version...

$ Message 53 is replaced with message 40
53 x

$ Messages 54-55 are replacements for messages 41-42
54 v
55 概觀(v)

$ Message 56 is a replacement for messages 44
56 工作(T)

$ Message 57 is a replacement for messages 46
57 參考(R)

$ Message 58 is a replacement for messages 50
58 使用輔助說明(U)

$ Messages 59-60 are replacements for messages 51-52
59 A
60 關於外型式樣管理者(A)

$set 5
$ _DtMessage set 5 is for the Font dialog
$ Messages 1-6, 7-16 from set 5 were deleted from the message catalog

$ _DtMessage 17 is the label of the titlebox of the preview font area
17 預覽

$ _DtMessage 18 appears in a Notice dialog after the user selects a new font
$ and presses Ok - when the resource *WriteXrdbImmediate is True
18 字形的改變在下次啟動某些應用程\n\
式時會顯現出來。其餘的應用程式，\n\
例如檔案管理者和應用程式管理者，\n\
除非您結束桌上管理系統並再重新登\n\
入，否則不會立即反映出來。

$ _DtMessage 19 appears in a Notice dialog after the user selects a new font
$ and presses Ok - when the resource *WriteXrdbImmediate is False
19 重新啟動這個階段作業之後，\n\
選定的字形就會被使用。

$ _DtMessage 20 and 21 is the text that initially appears in the Font Preview 
$ area displaying sample user and system fonts.
20 AaBbCcDdEeFfGg0123456789
21 AaBbCcDdEeFfGg0123456789

$ _DtMessage 22 is the label of the titlebox containing the list of font sizes
22 大小

$ _DtMessage 23 is the title of the Fonts dialog
23 外型式樣管理者 - 字形

$ _DtMessage 24 is an informative message used by the Fonts dialog 

24 大小列表中沒有您目前桌面上使用\n\
的字型. 如果選取並應用新字型,\n\
即使用外型式樣管理者 - 字型對話,\n\
也無法回到目前的字型.

$set 6
$ _DtMessage set 6 is for the Beep dialog
$ _DtMessage 1,6 from set 6 have been deleted from the message catalog

$ Messages 2 through 5 are the default buttons and labels of the scales.  
2 內定值
3 音量
4 音調
5 期間

$ _DtMessage 7 is the title of the Beep dialog box
7 外型式樣管理者 - 嗶聲  


$set 7
$ _DtMessage set 7 is for the Startup dialog
$ Messages 1-2 from set 7 have been deleted from the message catalog

$ Messages 3 through 9 are labels of the buttons and text visible in the 
$ Session Startup dialog box. 
3 在登入時：
4 繼續現行階段作業
5 返回起始階段作業
6 登出時詢問
7 登出確認對話：
8 開啟
9 關閉
10 設定起始階段作業...

$ _DtMessage 11 is a warning message when the user pushes Set Home Session...
$ Notice the spaces before Continue .. the spaces make it centered.
11 如此您的起始階段作業將會以\n\
現行階段作業取代。\n\n\
                 要繼續嗎?

$ _DtMessage 12 is the title of the warning dialog box when Set Home Session is 
$ pushed. 
12 警告

$ _DtMessage 13 is an error message
13 這個階段作業是由一個 X 視窗系統\n\
啟動命令集所啟動。但啟動設定無效，\n\
因為這個階段作業無法回復。

$ _DtMessage 14 is the title of the Session Startup dialog box.
14 外型式樣管理者 - 啟動

$ _DtMessage 15 is an error message
15 階段作業管理者沒有在執行。\n\
啟動設定無效。


$set 8
$ All of the messages from set 8 have been deleted from the message catalog


$set 9
$ _DtMessage set 9 is for the Mouse dialog
$ Message 7 from set 9 have been deleted from the message catalog

$ Messages 1-4 are labels of toggle buttons in the Mouse Dialog. 
1 左
2 右
3 用手習慣：
4 按鈕 2：
$ _DtMessage 5 is the label of default button.
5 內定值

$ Messages 6,8,9 are labels of the scales.
6 連按兩下
8 加速
9 臨界值

$  MESSAGE 10 IN SET 9 WILL ONLY APPEAR IN THE DT ERRORLOG FILE
10 錯誤：設定指標對應\n

11 傳送
12 調整

$ _DtMessage 13 appears in a Notice dialog after the user selects the 
$ left-handed or right-handed toggle
13 您按一下「確定」之後，右邊與左邊\n\
滑屬按鈕的功能立即交換。從滑鼠視窗\n\
「測試連按兩下」圖形的按鈕 1 與按鈕 3\n\
的位置，即可得知目前用手習慣的設定。\n\

$ _DtMessage 14 appears in a Notice dialog after the user selects the 
$ transfer toggle to change the function of the middle mouse button.
14 如果您按一下「確定」，在下一個階段作業中，\n\
滑鼠按鈕 2 就會被設定成「轉換」(拖曳)。

$ double-click time and presses Ok (and the resource 
$ *WriteXrdbImmediate is True)
15 應用程式重新啟動之後，新的連按\n\
兩下速度就會生效。

$ _DtMessage 16 appears in a Notice dialog after the user selects a new 
$ double-click time and presses Ok (and the resource 
$ *WriteXrdbImmediate is False)
16 在下一個階段作業中，新的連按兩下速度\n\
即可生效。

$ _DtMessage 17 is the text below the mouse icon. Notice the two lines.
17 測試\n連按兩下

$ _DtMessage 18 is the Mouse Dialog title
18 外型式樣管理者 - 滑鼠

$ _DtMessage 19 appears in a Notice dialog after the user selects the 
$ adjust toggle to change the function of the middle mouse button.
19 如果您按一下「確定」，在下一個階段作\n\
業中，滑鼠按鈕 2 就會被設定成「調整」。\n\
按鈕 2 可以用來調整文字與列示選擇。文字\n\
與列示的的傳送動作 (拖曳) 則須使用按鈕 1。\n\

$set 10
$ _DtMessage set 10 is for the SaveRestore functionality
$ Messages 2,4 from set 10 have been deleted from the message catalog

$  MESSAGE 1,3 IN SET 10 WILL ONLY APPEAR IN THE DT ERRORLOG FILE
$ Messages 1,3 make up one error message.  Put together, they say 
$ "<filename> <_DtMessage 2> <_DtMessage 1>
1 檢查檔案許可權。
3 %s 是用來儲存您的階段作業的檔案\n


$set 11
$ _DtMessage set 11 is for the Backdrop dialog

$  MESSAGE 1 IN SET 11 WILL ONLY APPEAR IN THE DT ERRORLOG FILE

$ _DtMessage 1 - could not open a backdrop directory
1 無法開啟目錄 %s。
$ _DtMessage 2 - error opening the backdrop files
2 外型式樣管理者的背景定位部分\n\
無法運作，因為沒有背景定位檔可\n\
用。請檢查 $HOME/.dt/errorlog。


$ _DtMessage 12 is the title of the Backdrop dialog box
12 外型式樣管理者 - 背景定位


$set 12
$ _DtMessage set 12 is for the Screen dialog
$ Message 2 from set 12 has been deleted from the message catalog

$ _DtMessage 1 is the title of the Screen dialog box. 
1 外型式樣管理者 - 螢幕

$ Messages 3-14 are the labels and buttons for the Screen dialog
3 內定值
4 螢幕省電裝置
5 啟動省電裝置
6 各別省電裝置時間設定
7 分鐘
8 螢幕鎖定
$ Message 9 has been removed 
10 開啟
11 關閉
12 螢幕省電裝置：
13 螢幕鎖定：
14 啟動鎖定
15 螢慕自動停歇
16 螢慕自動停歇程式
17 啟動螢慕停歇程式
18 前方面板鎖定
$ Message 19 has been replaced by message 21
19 使用鎖定背景
20 各別背景時間設定
$ Message 21 is a replacement for message 19
21 使用背景鎖定
22 透明鎖定

$set 13
$ _DtMessage set 13 is for the Keyboard dialog
$ _DtMessage 1,5 from set 13 have been deleted from the message catalog

$ _DtMessage 2 through 4 are labels of scales, buttons, and text visible in the
$ Keyboard dialog box.
2 內定值
3 自動重複
4 按鍵音量

$ _DtMessage 6 is the title of the Keyboard dialog box
6 外型式樣管理者 - 鍵盤


$set 14
$ _DtMessage set 14 is for the Color dialog
$ _DtMessage 1-4,9,14-15,17,19,20,22,24 from set 14 have been deleted from the message catalog

$ Messages 5 through 8 are labels and buttons visible in the Color dialog
5 調色盤
6 增加...
7 刪除...
8 修改...

$ _DtMessage 10 is the title above the text area in the Add Palette dialog box
10 新的調色盤名稱：

$ _DtMessage 11 is the title of the Add Palette dialog box.
11 增加調色盤

$ Messages 12 through 14 are error messages.
12 調色盤名稱不能包含\n\
下列字元：\n\n\
<space> * : ( ) [ ] { } < > ! | " / \\
$ LOCALIZERS: FOR 8 AND 16 BIT LANGUAGES YOU MAY WANT TO CHANGE THE MESSAGE 
$ TO SAY THE PALETTE NAME MUST BE 10 BYTES OR LESS
13 調色盤名稱必須在 10 個字元以內。\n

$ _DtMessage 16 is the title of the Delete Palette dialog box.  See screen 4a (2).
16 刪除調色盤

$ _DtMessage 18 appears in an information dialog.
18 無法刪除最後一個調色盤。\n

$ _DtMessage 21 is an error message
21 警告

$ _DtMessage 23 appears in an information dialog
23 要刪除調色盤 '%s' 嗎?\n

$ _DtMessage 25 is an error message that appears when you press the color
$ button and the color server is not running for some reason.
25 外型式樣管理者的顏色部分無法\n\
運作，因為顏色伺服器沒有在執行。\n\
請檢查 $HOME/.dt/errorlog。

$ _DtMessage 26 can be caused by setting the resource *useColorObj to False
$ and then running dtstyle and trying to post the Color Dialog.
26 外型式樣管理者的顏色部分無法\n\
運作，因為資源 'useColorObj' 設定為\n\
False。

$ _DtMessage 27 is for the new ColorUse dialog when you press ok.
27 新的「顏色使用」值在您下一個階段\n\
作業時即會生效。

$ _DtMessage 28 is a warning message that appears on Black and White systems
28 選取的調色板在您下一個階段\n\
作業時即會生效。

$ _DtMessage 29 appears in a warning dialog
29 名為 '%s' 的調色盤已經存在。\n\
這個新調色盤將會重寫舊調色盤。\n\
您想這樣做嗎?

$ _DtMessage 30 is the title of the Color dialog box.
30 外型式樣管理者 - 顏色

$ _DtMessage 31-33 are the ColorUse options
$ 31 High Color
$ 32 Medium Color
$ 33 Low Color
34 黑白
35 內定值

$ Dt Message 36 has been replaced by message 39
$ _DtMessage 36 is the title of the Color Use dialog
$ 36 DT Color Use

$ Dt Message 37 has been replaced by message 40
$ _DtMessage 37 is the Color Use button in the main Color Dialog 
$ 37 DT Color Use...

$ _DtMessage 38 is an error message that appears when you press the color
$ button and there are no palette files to be found
38 外型式樣管理者的顏色部分無法\n\
運作，因為沒有調色盤檔案可用。\n\
請檢查 $HOME/.dt/errorlog。

$ DtMessage 39 is a replacement for DtMessage 36
$ DtMessage 39 is the title of the Color Use dialog
39 要使用的顏色數目：

$ DtMessage 40 is a replacement for DtMessage 37
$ DtMessage 40 is the title of the Color Use dialog
40 顏色數目...

$ _DtMessages 41-43 are replacements for Dtmessages 31-33
$ _DtMessage 41-43 are the ColorUse options
41 更多的桌上管理系統顏色
42 更多的應用程式顏色
43 更多的應用程式顏色

$set 15
$ All of the messages from set 15 have been deleted from the message catalog


$set 16
$ _DtMessage set 16 is for the Color dialog
$ _DtMessage 1-10,13-17 from set 16 have been deleted from the message catalog

$ _DtMessage 11-12 form one error message that appears in an error dialog
11 無法刪除 '%s'。\n
12 請檢查 $HOME/.dt/errorlog，您可從中得到提示。

$  MESSAGE 18-21 IN SET 16 WILL ONLY APPEAR IN THE DT ERRORLOG FILE
18 Warning, Too many directories listed in the resource paletteDirectories,\n\
Maximum number is %d. 
$ _DtMessage 19 - could not open a palette directory
19 Could not open directory %s. 
$ _DtMessage 20 - error opening the palette file
20 Could not open %s. 
$ _DtMessage 21 - error reading a palette file
21 %s is an invalid palette file.


$set 17
$ _DtMessage set 17 is for the Modify Color dialog
$ Messages 1-3,13,16 from set 17 have been deleted from the message catalog

$ Messages 4 through 12 are labels, buttons, and text visible in the
$ Modify Color dialog box
4 顏色範例
5 舊
6 新
7 抓取顏色
8 顏色編輯器
9 色度
$ We recommend that you don't translate R,G,B which stands for Red, Green,
$ Blue unless there are standard single letter acronyms in your language.
10 紅
11 綠
12 藍

$  MESSAGE 14-15 IN SET 17 WILL ONLY APPEAR IN THE DT ERRORLOG FILE
14 Warning, couldn't grab pointer.\n
15 Warning, couldn't grab Keyboard.\n

$ _DtMessage 16 is the title of the Modify Color dialog box.
16 外型式樣管理者 - 修改顏色


$set 18
$ _DtMessage set 18 is for the Window dialog
$ _DtMessage 1 is the title of the Window dialog box
1 外型式樣管理者 - 視窗

$ Messages 2 through 5 are labels on the Window dialog.
2 內定值
3 視窗行為
4 視窗圖像
$ Messages 5-8 have been replaced by messages 13-16 
$ 5 Focus Follows Mouse
$ 6 Click In Window For Focus
$ 7 Raise Window With Focus
$ 8 Opaque Move
9 使用圖像方框
10 置於工作空間中
11 允許主要視窗置於頂層
12 工作空間管理者必須重新啟動，您\n\
所作的變更才會生效。按「確定」可以\n\
重新啟動工作空間管理者，按「取消」\n\
則可重設這些變更。
$ Messages 13-16 are replacements for messages 5-8
13 請指到視窗中使其成為現行視窗
14 請按一下視窗使其成為現行視窗
15 成為現行視窗後提至前端
16 移動時顯示內容

$set 99
$    DO NOT TRANSLATE THESE MESSAGES
$    DO NOT CHANGE THESE MESSAGES
$    DO NOT LOCALIZE THESE MESSAGES
$    These messages are used for the version information.
$quote "
1 "@(#)version_goes_here"
2 "\n@(#)_DtMessage catalog source $XConsortium: dtstyle.msg /main/4 1995/12/08 09:50:58 rswiston $"

