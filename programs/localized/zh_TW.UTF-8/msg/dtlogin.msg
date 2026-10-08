
$ "$XConsortium: dtlogin.msg /main/3 1995/11/08 14:07:17 rswiston $"

$ *************************************<+>*************************************
$ *****************************************************************************
$ **
$ **  File:        dtlogin.msg
$ **
$ **  Project:     CDE dtlogin
$ **
$ **  Description:
$ **  -----------
$ **  This file is the source for the message catalog for dtlogin
$ **
$ **
$ *****************************************************************************
$ **
$ **     (c) Copyright 1990 Hewlett-Packard Company
$ **     All Rights reserved
$ **
$ **
$ *****************************************************************************
$ **
$ **
$ **  ------------------------- MODIFICATION RECORD --------------------------
$ *
$ * 
$ **  ----------------------- MODIFICATION RECORD END ------------------------
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

$set 1
$ set 1 is labels
$ 
$ messages 1 through 6 are button labels on the login screen

1 請輸入您的使用者名稱
2 請輸入您的密碼
3 確定
4 重新開始
5 選項
6 輔助說明

$ 
$ messages 7 through 11 are options button menu items
$ 

7 重設登入螢幕
8 命令行登入
9 版本...
10 單視窗階段作業
11 語言

$ 
$ messages 12 through 14 are miscellaneous button labels
$ 
12 開始 DT
13 取消 
14 輔助說明


$ 
$ message 15, 17 are more option button menu items 
$ message 16 is the welcome message
$ 
15 DT Lite 階段作業
16 歡迎使用
17 一般桌上管理系統

$ 
$ message 18-24
$ 
18 * 暫停桌上管理系統登入...\r\n*\r\n
19 * 如需登入提示，請按 [Enter]。\r\n*\r\n
20 * 登入。在您登出之後不久，桌上管理系統登入即將繼續。\r\n*\r\n
21 * 桌上管理系統登入繼續中...\r\n*\r\n
22 * 在顯示器 %1$s 上啟動桌上管理系統登入...\r\n*\r\n
23 * 請先等待桌上管理系統登入螢幕出現之後再行登入。\r\n*\r\n
24 * %1$s 顯示器上無法啟動 X 伺服器...\r\n*\r\n

$ messages 25 is a options button menu item
25 階段作業

$ 
$ 26 replaces 16 as the greeting after a user name has been 
$ entered. The %s is replaced by the user name.
$ 
26 歡迎 %s

$ 
$  Sun specific message catalog id numbers in MC_LABEL_SET messages.
$  (Starting at a random uneven higher number, 73, to allow some room
$  for some shared sample source message additions.
$ 
73 使用者上一次桌上管理系統

$set 2
$ set 2 is error messages
$ 

1 登入不正確，請再試一次。
2 無法變換到起始目錄。
3 抱歉。目前登入的使用者人數已達極限。
$ message 4 is a lengthy message that should be localized to correspond to 
$ the language.  
4 \n\
您必須選擇一個起始密碼，\n\
或者是您的現行密碼已過期。\n\
\n\
設定好密碼之後，請使用新密碼\n\
重新登入一次。
5 登入錯誤，使用者 id 無效。
6 登入錯誤，群組 id 無效。
7 登入錯誤，稽核  id 無效。
8 登入錯誤，稽核旗標無效。
9 目前登入功能被取消。
10 只有根使用者可執行 %1$s\n
11 從 %1$s 重新建構內定的語言清單\n
12 重新掃描配置檔與伺服器檔\n
13 重新讀取配置檔 %1$s\n
14 重新讀取伺服器檔 %1$s\n
15 重新讀取存取檔 %1$s\n
16 dtlogin：常式耗盡記憶體\n


$set 3
$ set 3 is help text
$ 

$ message 1 is generated when the help button on the login screen is clicked.
1 \n\
登入輔助說明：\n\
\n\
請使用登入畫面讓電腦知道您的身份。\n\
\n\
若要登入：\n\
       1) 鍵入您的使用者名稱，然後按 <Enter> 鍵或按一下 [確定]。\n\
          (如果您尚未建立一個使用者，請輸入 "root" 做為\n\
	  使用者名稱\n\
       2) 鍵入您的密碼，然後按 <Enter> 鍵或按一下 [確定]。\n\
\n\
[重新開始]\n\
   消除您的輸入項，以便重新輸入您的使用者名稱\n\
   重試一次。\n\
\n\
[選項]\n\
   能讓您變更階段作業選項：\n\
\n\
   將滑鼠指標置於 [選項] 按鈕之上，按住 \n\
   滑鼠按鈕 1。拖曳指標作選擇。\n\
   當指標位於某個項目上方，放開\n\
   滑鼠按鈕即可選取該項目。您可以選擇：\n\
\n\
   語言 - 改變階段作業使用的語言\n\
   階段作業 - 選擇一個圖形環境\n\
     OpenWindows 桌上管理系統 - 為 OW 桌上管理系統 \n\
     一般桌上管理系統 - 為一般桌上管理系統環境 (CDE) \n\
     使用者上一次桌上管理系統 - 為上一次登入 \n\
     故障自動保險階段作業 - 為單一 X 視窗環境\n\
   命令行登入 - 轉換成命令行環境\n\
   重設登入螢幕 - 重新啟動 X 伺服器\n\
\n\
使用任一滑鼠按鈕在背景任一處按一下，即可\n\
刷新螢幕。您可以在系統訊息甘擾到螢幕以致\n\
於不容易閱讀時，使用此功能。\n\
\n\
如需更多的輔助說明資訊，請登入後在前方面板的\n\
「輔助說明管理者」圖像 (書上有一個問號者) 上按\n\
一下，即可找到「登入管理者」輔助說明那一冊。\n\

$ message 2 is generated when you attempt to start a system whose id is
$ set to unknown or if it is not configured for networking.
2 \n\
您的工作站名稱與其他工作站重複。因此目前被設定\n\
成「unknown」。建議您替這個工作站再命名一次。\n\
\n\

$ message 3 is generated when the help button on the chooser screen is clicked.
3 \n\
選擇程式輔助說明：\n\
\n\
請使用此選擇程式螢幕選擇要登入的伺服器。\n\
\n\
選擇程式螢幕含有一個列表，每個項目都包括兩個\n\
部分。第一個部分 (伺服器名稱) 含有每一個可用伺\n\
服器的網路名稱。第二個部分 (伺服器資訊)\n\
則依每個伺服器上執行的登入管理者不同，\n\
而有不同的資訊。如果某個伺服器執行\n\
CDE 桌上管理系統，此部份就會提供目前\n\
正在使用此系統的使用者 (或 tty 裝置) 的人數，\n\
以及前 5、10、15 分鐘的負載平均值 (CPU 平均使用量)\n\
等資訊。\n\
\n\
要選擇一個伺服器登入，只要在列示中選取有\n\
您想登入的伺服器名稱的那一行即可。\n\
\n\
作好選擇之後，請按「確定」按鈕。螢幕上就會\n\
出現您所選擇的伺服器的登入螢幕。\n\
\n\
[更新列示]\n\
   這個選項會清除列示，並查詢網路上的伺服\n\
   器，看是否允許登入後，重新建構此列示。\n\
\n\
[選項]\n\
   讓您變更階段作業選項：\n\
\n\
   將滑鼠指標置於 [選項] 按鈕之上，按住 \n\
   滑鼠按鈕 1。拖曳指標作選擇。\n\
   當指標位於某個項目上方，放開\n\
   滑鼠按鈕即可選取該項目。您可以選擇：\n\
\n\
   語言 - 改變階段作業使用的語言\n\
   重設登入螢幕 - 重新啟動 X 伺服器\n\
\n\
使用任一滑鼠按鈕在背景任一處按一下，即可\n\
刷新螢幕。您可以在系統訊息干擾到螢幕以致\n\
於不容易閱讀時，使用此功能。\n\
\n\


$set 4
$ set 4 is log error messages
$ 

1 存取檔案 "%1$s"，主機 "%2$s" 無法找到\n
2 ReadHostEntry\n
3 存取檔案 %1$2s，顯示器 %1$2s 為不明的顯示器\n
4 無法開啟存取控制檔 %1$s，因此無法答應 XDMCP 要求\n
5 網域/操作系統確認錯誤：%1$s (%2$s/%3$s)
6 無法開啟伺服器確認檔 %1$s\n
7 無法寫入伺服器確認檔 %1$s\n
8 saveAddr
9 獲取網路介面配置時有問題
10 setAuthNumber
11 無法鎖定授權檔 %1$s 或備份檔 %2$s\n
12 無法將授權檔移到適當位置\n
13 無法建立/鎖定 pid 檔案 %1$s\n
14 無法鎖定 pid 檔案 %1$s，因為另一個 Dtlogin 正在執行 (pid %2$d)\n
15 無法存取伺服器檔案 %1$s\n
16 NewDisplay
17 無法開啟 errorLogFile %1$s\n
18 TrimErrorLog()：錯誤日誌檔的檔案大小限制最大為 200Kb。\n
19 伺服器檔案缺少顯示器名稱\n
20 %1$s 缺少顯示器類型\n
21 主機名稱 %1$s 無效。DISPLAY 設定為 ':0'\n
22 顯示器 %2$s 的顯示器類型 %1$s 無法被接受\n
23 虛擬重設逾時。\n
24 GetResource
25 沒有空間可供引數重新配置\n
26 無法開啟配置檔 %1$s\n
27 命令行有多餘的引數：
28 StartServer()：沒有引數\n
29 伺服器 %1$s 無法執行\n
30 StartServer() 產生子程序失敗，暫停中\n
31 伺服器意外中斷\n
32 XOpenDisplay(%1$s) 嘗試 #%2$d 中止，正在中斷。\n
33 開啟伺服器嘗試 #%1$d 失敗，因為 %2$s，放棄再嘗試\n
34 嚴重 IO 錯誤 %d (%1$s)\n
35 X 錯誤\n
36 階段作業啟始錯誤\n
37 警告：顯示器 %1$s 無法保障安全\n
38 警告：顯示器 %1$s 的鍵盤無法保障安全\n
39 BLS - 無法獲得戶頭資訊。\n
40 BLS - 無法獲得受保護的戶頭資訊。\n
41 BLS - 無法驗證敏感程度。\n
42 BLS - 無法變更為使用者：%1$s。\n
43 AFS - 啟動階段作業時 setgroups() 失敗\n
44 使用者 %s Setuid 失敗，errno = %d\n
45 使用者 %2$s 沒有啟始目錄 %1$s，使用 /\n
46 階段作業執行失敗 %1$s\n
47 階段作業沒有命令/引數\n
48 無法啟動 %s 的階段作業，產生子程序失敗\n
49 Dtlogin 無法清除客戶機\n
50 無法執行 %1$s\n
51 無法產生子程序以執行 %1$s\n
52 不適當的確認名稱 "%1$s"。使用 AFS、Kerberos 或內定名稱。\n
53 %2$s 無法執行 Dtgreet %1$s \n
54 無法產生子程序以執行 Dtgreet %1$s\n
55 %1$d 埠建立插座失敗\n
56 結合插座位址 %1$d 錯誤，errno = %2$d\n
57 makeEnv
58 setEnv
59 parseArgs
60 %1$s 目錄的掃描時間超過 %2$d 秒。結束掃描中...\n
61 %s 屬於 %1$d 個以上的群組，%2$s 會被忽略\n
62 audwrite() 錯誤：呼叫者並非高階使用者\n
63 audwrite() 錯誤：稽核記錄中有無效的事件號碼。\n
64 audwrite() 錯誤：errno = %1$d\n
65 AFS 確認失敗：%1$s\n
66 %1$s 的戶頭已到期。請與您的系統管理員連絡。\n
67 伺服器中止  %1$s\n
68 無法開啟標誌位元映射檔：%1$s\n
69 無法讀取 "%1$s" 訊息目錄。使用內定語言。
70 顯示器 %1$s 無法啟始設定，伺服器可能被抓取。\n
71 (%1$s)：登入 matte 寬度超過顯示器寬度，使用內定值。matte 的單位為圖像單元。\n
72 (%1$s) 登入 matte 高度超過顯示器高度，使用內定值。matte 的單位為圖像單元。\n

$set 5
$ set 5 is for other stuff specific to chooser.
1 請選擇一個伺服器登入：
$ 1 is the title/instructions for the chooser dialog
2 伺服器名稱                    伺服器資訊
$                                ^
$                                | This needs to start in this column
$ 2 is the headings for the list
3  更新列示 

$set 99
$    DO NOT TRANSLATE THESE MESSAGES
$    DO NOT CHANGE THESE MESSAGES
$    DO NOT LOCALIZE THESE MESSAGES
$    These messages are used for the version information.
$quote "
1 "@(#)version_goes_here"
2 "\n@(#)_DtMessage catalog source $XConsortium: dtlogin.msg /main/3 1995/11/08 14:07:17 rswiston $"

