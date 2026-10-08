$ +SNOTICE
$ 
$ 	$TOG: libDtMail.msg /main/4 1997/10/31 15:40:33 rafi $
$ 
$ 	RESTRICTED CONFIDENTIAL INFORMATION:
$ 	
$ 	The information in this document is subject to special
$ 	restrictions in a confidential disclosure agreement bertween
$ 	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
$ 	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
$ 	Sun's specific written approval.  This documment and all copies
$ 	and derivative works thereof must be returned or destroyed at
$ 	Sun's request.
$ 
$ 	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
$ 
$ +ENOTICE
$ @(#)libDtMail.msg	1.8 03 Nov 1994
$set 1 libDtMail API set
1 未發生任何錯誤。
2 某個不具有效 Bento 物件的訊息被存取。
3 某個建立時有問題的物件被存取。
4 有物件無法建立。
5 物件找不到值。
6 系統記憶體不足，無法執行此操作。
7 此項操作接收到一個錯誤的引數。
8 您嘗試起始設定一個已起始設定過的物件。
9 要求的檔案不存在。
10 要求的檔案並非具此版可辨識格式的郵箱。
11 DtMail 不存在訊息目錄。
12 錯誤結構無效。
13 這個值是一個內含的儲存，必須先開啟才能讀取。
14 另一個郵件程式具有此檔案夾的寫入許可。
15 某個 ToolTalk 訊息無法處理。
16 您的要求被操作伺服器拒絕。
17 使用者中斷此項操作。
18 某個物件無法存取。
19 要求的檔案無法建立。
20 執行此項操作時發生內部錯誤。
21 現行物件並不支援您所要求的操作。s
22 沒有資料類型與運送類型相符。
23 使用者岔斷轉換。
24 郵件程式庫沒有已製作完成者。
25 指定的程式製作不存在。
26 指定的程式製作無法執行要求的操作。
27 現行的程式製作未支援此項操作。
28 程式庫不認得此資料類型。
29 指定的郵遞位址無法處理。
30 郵件傳遞失敗。
31 MD5 類別記號與訊息內容不相符。
32 有效的執行群組並非 "mail"

$ 
$ Strings from libDtMail/Common/DtMailValuesBuiltin.C

$set 2
1 "%C" 
$ /* NL_COMMENT
$  * The %C is the time and date format, please refer to strftime man page for 
$  * explanation of each format.
$  */
