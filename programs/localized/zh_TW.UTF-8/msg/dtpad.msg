$ $XConsortium: dtpad.msg /main/4 1995/12/08 09:50:26 rswiston $
$ *************************************<+>*************************************
$ *****************************************************************************
$ **
$ **  File:        dtpad.msg
$ **
$ **  Project:     DT Text Editor client (sometimes called dtpad)
$ **
$ **  Description:
$ **  -----------
$ **  This file is the source for the message catalog for dtpad
$ **
$ **
$ *****************************************************************************
$ **
$ **     (c) Copyright 1992, 1993 Hewlett-Packard Company
$ **     All Rights reserved
$ **
$ **
$ *****************************************************************************
$ **
$ *****************************************************************************
$ **  (c) Copyright 1993, 1994 Hewlett-Packard Company
$ **  (c) Copyright 1993, 1994 International Business Machines Corp.
$ **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
$ **  (c) Copyright 1993, 1994 Novell, Inc.
$ *****************************************************************************
$ **

$set 2
$ **********************************************
$ **  General messages used in multiple files **
$ **********************************************

$ Label used for all "Help" buttons in all DIALOGS.
$ It is not used for the Help menu.
1 輔助說明


$set 4
$ *****************************************************************
$ **                                                    fileCB.c **
$ ** These are the strings which change between the Open dialog  **
$ ** and the Include dialog.  These two operations use the same  **
$ ** Motif dialog, and simply change these two labels.           **
$ *****************************************************************

$ Title of the Open dialog
1 開啟檔案

$ label for the File text field in the file selection box
$ ***UNUSED***	This label in no longer used and need not be localized.
2 要開啟的檔案：

$ Title of the Include dialog
3 含括檔案

$ Label for the File text field in the file selection box
$ ***UNUSED***	This label is no longer used and need not be localized.
4 要含括的檔案：

$ Error when printing a file and a temporary copy can not be saved.
5 無法建立暫時檔。


$set 5
$ ***************************************************************************
$ **                                                               dtpad.c **
$ **                                                             fileDlg.c **
$ **                                                                       **
$ ** Messages and strings used in dialogs for all operations involving the **
$ ** "File" menu items.                                                    **
$ ***************************************************************************
$ These compose the dialog which appears if the user attempts to exit, open
$ a different file, drop a file, or start a "New" session with unsaved
$ changes to the current file.

$ This is used to build: "Save changes to FILENAME?"
1 要儲存 %s 的修改部分嗎?

$ This is used as-is, whenever the current file is unnamed.
2 現行檔案沒有檔名。\n您要儲存這個檔案嗎?

$ This is used to build a message like "Do you want to save the current
$ copy of FILENAME before you Edit NEWFILENAME?" when a new file is droped
$ on the edit window.
$ ***UNUSED***	Currently, dropped files are INSERTED in the text rather than
$		opened for editing so this message is no longer used and need
$		not be localized.
3 您要先儲存 %s 之後，再編輯 %s 嗎?

$ Used when about to write over an existing file.
$ The "%s" is replaced by the file name.
5 檔案 %s 已存在。\n您想以新編輯的內容\n重寫該檔案嗎?

$ Title of "file already exists" dialog.  Also used for other "Warning" type
$ dialogs.
6 警告

$ Title of "Save As" dialog.
7 另存新檔

$ Labels for add/don't-add newlines radio buttons in the Save As dialog
$ ***UNUSED***	These labels are no longer used and need not be localized.
$		They've been replaced by messages 44 and 45
8 在自動換行的行結尾加上換行字元。
9 不加入換行字元。只保留以 [Return] 產生的\n換行。

$ Labels for the Save Warning dialog. Also used in the confirmation, or
$ Editor Return dialog.
14 否
15 是
$ Title of the dialog - used when user selects "New", "Open", or "Exit" with
$ unsaved changes in the text.
16 要儲存嗎?

$ Title of general error dialog
17 錯誤

$ Title of the resource-specified "confirmation" dialog
$ This dialog is generally specified by programs such as mailers, which
$ want to know if the user wants to send the message.
$ There is no default message for the dialog, since it is _only_ posted if
$ the message string is specified via a resource (dtpad*confirmationString:)
$ This message is only used if the "dtpad*mainTitle:" resource is not set.
$ If this resource _is_ set, the the mainTitle is also used as the title
$ for the exit confirmation dialog.
$ DO NOT LOCALIZE - the user should never see this message
18 Exit Confirmation

$ Title for general information dialog. e.g. Find dialog can't find word in
$ file.
20 資訊

$ Default file name in Save As dialog box.
21 UNTITLED

$ Error message when a file can not be saved.  The "%1$s" gets replaced by
$ the file name and the "%2$s" gets replaced by the reason why the file could
$ not be saved (messages 31-39).
30 無法儲存 %1$s。\n%2$s

$ Messages 31-39 are reasons why a file could not be saved.
$ Messages 34-39 are substitued in message 30 for "%2$s".
31 無法儲存檔案。\n未指定檔名。

34 該檔案不允許任何人寫入。

35 該檔案是一個目錄。

36 該檔案是一個字元特殊裝置。

37 該檔案是一個區段模態裝置。

$ ***UNUSED***	This message should never appear since currently there is
$		only one supported file format - text.  And this type is
$		hard coded into dtpad.
38 無效的檔案格式。

$ This is a "catch all" when the file can not be saved and none of the above
$ errors apply.
39 理由不明 - 可能是磁碟空間不足。

$ This is displayed when a ToolTalk edit request on a buffer is received,
$ a Save operation is done and there are unsaved changes.
40 要儲存變更部分嗎？

$ This is displayed when a ToolTalk edit request on a file or buffer is
$ received with a document name specified in the request, a Save operation
$ is done and there are unsaved changes.  The "%s" in the message is
$ replaced by the document name (instead of the filename).
41 要將變更部分儲存到 "%s" 嗎？

$ Title of "Copy As" dialog which is substitued for the "Save As" dialog
$ when file name changes are disallowd.  This should match the menu button
$ label specified in message 108 of set 11.
$ ***UNUSED***	This title is no longer used and need not be localized.
42 複製為

$ Title of "Copy To File" dialog which is substitued for the "Save As" dialog
$ when file name changes are disallowd.  This should match the menu button
$ label specified in message 110 of set 11.
43 複製到檔案

$ Labels for add/don't-add newlines radio buttons in the SaveAs and Save dialogs
44 在自動換行的行結尾加入換行字元。
45 不加入換行字元。 只保留以 [Return] 產生的\n換行。
$ 
$ New message as of 5/31/95 (i.e. new since Sample Implementation). This
$ new message is in response to bug DTS CISlx19530.
$ 
46 可能是檔案無法寫入\n或到該檔案的路徑不存在。


$set 6
$ ************************************************************************
$ **                                                           fileIo.c **
$ **                                                                    **
$ ** Error messages generated when there are problems reading a file.   **
$ ************************************************************************

1 該檔案只能讀取。
2 該檔案不存在。
3 無法編輯目錄。
4 無法編輯字元特輸裝置檔案。
5 無法編輯區段模態裝置檔案。
6 由檔案抽出之內建空值
7 無法載入檔案 (記憶體不足)。
8 無法存取檔案 (無法得知其狀態)。
9 無法讀取檔案 (可能無讀取許可權)。
$ Message placed in the status line message area based on the -viewOnly
$ resource and whether or not the file being edited is read only
10 只能察看
11 唯讀


$set 7
$ ************************************
$ **                         main.c **
$ **                        dtpad.c **
$ ** Prefix string for all dialogs. **
$ ************************************
$ bogus?? blank string gotten but otherwise unused in main()
$ DO NOT LOCALIZE string 1
$quote "
1 ""
$quote

$ The name used in the title bar of every window.
$ Be sure to end this message with a space character.
2 文字編輯器 - 

$ Default file name used in the main window title.
$ ***UNUSED***	This message has been replaced by message 6 of this set and
$		need not be localized.
3 (unnamed)

$ Terminal error message displayed to stdout when the desk top services
$ can not be initialized (via DtInitialize())
4 無法起始設定桌上管理系統服務... 結束文字編輯器。

$ Terminal error message displayed to stdout when a connection to the
$ display can not be opend (via XtDisplay())
5 無法開啟顯示器... 結束文字編輯器。

$ Default file name used in the main window title.
$ This should correspond to the default file name used in the "Save As"
$ dialog box - specified in message 21 of set 5.
6 (UNTITLED)
$ 
$ New messages (7,8) since CDE/SI - error gets displayed when user tries to run
$ dtpad -server when and dtpad server process is already running on the
$ $DISPLAY.
7 錯誤: 一個 dtpad 伺服器 %s\n\
已經在服務該顯示器。
8 錯誤: 一個 dtpad 伺服器\n\
已經在服務該顯示器。
$ 
$ Message 9 is also new since CDE/SI - new as of 5/22/95.  It is a usage message
$ for dtpad. Don't translate the '-OPTION' just the text explaining the
$ option.
$ 
9 '%s' 是錯誤的選項。\n\
用法: %s...\n\
   -saveOnClose\n\
\t若有未儲存的更改且要關閉文字編輯器時,\n\
\tDtpad 會自動並靜悄悄地儲存現行文字。\n\n\
   -missingFileWarning\n\
\t每當指定一個不存在或無法存取的檔案時,\n\
\t即顯示一個警告對話框。\n\n\
   -noReadOnlyWarning\n\
\t毎當使用者指定一個没有寫入許可權的檔案時,\n\
\t取消警告對話框顯示。\n\n\
   -noNameChange\n\
\t指示當內文儲存成不同於讀取時的名稱時,\n\
\t不更改和現行內文相關的內定檔案名稱。\n\n\
   -viewOnly\n\
\t不允許在編輯視窗中編輯文字, 基本上是將\n\
\t文字編輯器變成文字察看器。\n\n\
  -statusLine\n\
\t在編輯視窗底端顯示一行狀態行。 該狀態行\n\
\t會顯示文字游標目前所在位置的行號。\n\n\
  -wrapToFit\n\
\t開啟自動換行模態。\n\n\
   -workspaceList WorkspaceList\n\
\t在指定的工作空間中調用現行的文字\n\
\t編輯器, 以顯示編輯視窗。\n\n\
   -session SessionFile\n

10 \t對所有的文字編輯視窗和前次 CDE 關機時發生效用\n\
\t的設定值回復文字編輯器。 當指定此選項時, 所有其\n\
\t他的命令行選項皆會被忽略。\n\n\
   -standAlone\n\
\t強迫現行調用的文字編輯器在其本身的視窗中, \n\
\t進行自己的文字處理, 而獨立於文字編輯器伺\n\
\t服器之外。\n\n\
  -noBlocking\n\
\t在文字編輯器伺服器可處理要求者的編輯要求時,\n\
\t即刻終止文字編輯器要求者的程序。\n\n\
  -server\n\
\t強迫文字編輯器伺服器啟動 (若它尚未執行的話)\n\
\t以處理該顯示器隋後而來的編輯要求。\n\n\
  -exitOnLastClose\n\
\t指定當顯示器上最後一個編輯視窗關閉時,\n\
\t終止文字編輯器伺服器程序。\n\
\t它只配合 -server 選項使用, 因為它只適用\n\
\t於伺服器程序。\n\n\
有關這些選項的詳細資料, 請參閱 dtpad 線上援助頁。\n\n
$ 


$set 10
$ ************************************************************************
$ **                                                          session.c **
$ ** THESE MESSAGES ARE _ONLY_ WRITTEN TO THE ERRORLOG FILE.
$ **
$ ************************************************************************
$ Strings used to build error messages issued when the session save
$ file cannot be read or written.
1 檢查檔案許可權。
2  %s 是用來儲存您階段作業的檔案。


$set 11
$ ***********************************************************
$ **                                              dtpad.c **
$ **                                                       **
$ ** These are all of the menu button labels and mnemonics **
$ ***********************************************************

$ ***UNUSED*** 	Messages 1-14 are no longer used and need not be localized.
$		All of the mnemonics and labels relating to the "Help" menu
$		have been replaced by messages 81-96.

$ *
$ * Mnemonics and labels for the "Help" pulldown menu and items.
$ *
$ Mnemonic and label for the "Help" pulldown menu
1 H
2 輔助說明(H)

$ mnemonic and label for the "Overview" item
3 V
4 概觀(V)...

$ mnemonic and label for the "Tasks" item
5 T
6 工作(T)...

$ nmemonic and label for the "Table of Contents" item are message nos. 71 & 72

$ mnemonic and label for the "Reference" item
7 R
8 參考資料(R)...

$ mnemonic and label for the "On Item" item
9 O
10 關於項目(O)

$ mnemonic and label for "Using Help" item
11 U
12 使用輔助說明(U)...

$ mnemonic and label for "About Text Editor" item
13 A
14 關於文字編輯器(A)...

$ *
$ * Mnemonics and labels for the "File" pulldown menu and items.
$ *
$ mnemonic and label for the "File" pulldown menu
15 F
16 檔案(F)

$ mnemonic and label for the "New" item
17 N
18 新增(N)

$ mnemonic and label for the "Open" item
19 O
20 開啟(O)...

$ mnemonic and label for the "Include" item
21 I
22 含括(I)...

$ mnemonic and label for the "Save" item
$ NOTE: The "Save" label can be either Message 24 or message 73
23 S
24 儲存(S)

$ mnemonic and label for the "Save As" item
25 A
26 另存新檔(A)...

$ mnemonic and label for the "Print" item
27 P
28 列印(P)...

$ mnemonic and label for the "Close" item
29 C
30 關閉(C)

$ *
$ * Mnemonics and labels for the "Edit" pulldown menu and items.
$ *
$ mnemonic and label for the "Edit" pulldown menu
31 E
32 編輯(E)

$ mnemonic and label for the "Undo" menu item
33 U
34 還原(U)

$ mnemonic and label for the "Cut" menu item
35 t
36 剪取(t)

$ mnemonic and label for the "Copy" menu item
37 C
38 複製(C)

$ mnemonic and label for the "Paste" menu item
39 P
40 黏貼(P)

$ mnemonic and label for the "Clear" menu item
41 e
42 清除(e)

$ mnemonic and label for the "Delete" menu item
43 D
44 刪除(D)

$ ***UNUSED***	Messages 45-50 are no longer used and need not be localized.
$		They have been replaced by messages 101-106 of this set.
$ mnemonic and label for the "Select All" menu item
$ 45 A
$ 46 Select All

$ mnemonic and label for the "Find/Change" menu item
$ 47 F
$ 48 Find/Change...

$ mnemonic and label for the Check Spelling menu item
$ 49 S
$ 50 Check Spelling...

$ *
$ * Mnemonics and labels for the "Format" pulldown menu and items.
$ *
$ mnemonic and label for the "Format" pulldown menu
51 r
52 格式(r)

$ mnemonic and label for the "Settings..." menu item
53 S
54 設定(S)...

$ mnemonic and label for the "Paragraph" menu item
55 P
56 段落(P)

$ mnemonic and label for the "All" menu item
57 l
58 全部(l)

$ *
$ * Mnemonics and labels for the "Options" pulldown menu and items.
$ *
$ mnemonic and label for the "Format" pulldown menu
59 O
60 選項(O)

$ mnemonic and label for the "Overstrike" toggle button
61 O
62 覆寫模態(O)

$ mnemonic and label for the "Word Wrap" toggle button
$ ***UNUSED***	This label and mnemonic are no longer used and need not be
$		localized.  They've been replaced by messages 111 and 112.
63 W
64 自動換行(W)

$ mnemonic and label for the "Status Line" toggle button
65 S
66 狀態行(S)

$ *
$ * Late additions.
$ *

$ nmemonic and label for the "Table of Contents" item of the "Help" menu
$ ***UNUSED*** 	This nmemonic and label are no longer used and need not be
$		localized.  They've been replaced by messages 87 and 88.
71 C
72 目錄(C)...

$ label for the "Save" item in the "File" pulldown menu
$ NOTE: This label is used in place of message 24 when text has been
$       changed but has not been saved
73 必須儲存(S)


$ *
$ * Mnemonics and labels for the "Help" pulldown menu and items.
$ *
$ Mnemonic and label for the "Help" pulldown menu
81 H
82 輔助說明(H)

$ mnemonic and label for the "Overview" item
83 v
84 概觀(v)

$ mnemonic and label for the "Tasks" item
85 T
86 工作(T)

$ nmemonic and label for the "Table of Contents" item
87 C
88 目錄(C)

$ mnemonic and label for the "Reference" item
89 R
90 參考資料(R)

$ mnemonic and label for the "On Item" item
91 O
92 關於項目(O)

$ mnemonic and label for "Using Help" item
93 U
94 使用輔助說明(U)

$ mnemonic and label for "About Text Editor" item
95 A
96 關於文字編輯器(A)


$ *
$ * Changes for "Edit" pulldown menu messages 45-50.
$ *

$ mnemonic and label for the "Select All" menu item
101 S
102 全部選取(S)

$ mnemonic and label for the "Find/Change" menu item
103 F
104 尋找(F)/更改...

$ mnemonic and label for the Check Spelling menu item
105 k
106 拼字檢查(k)...

$ mnemonic and label for the "Copy As" File menu item
$ (should match the dialog title specified in message 42 of set 5)
$ ***UNUSED***	This label is no longer used and need not be localized.
107 A
108 複製為(A)...
$ mnemonic and label for the "Copy To File" File menu item
$ (should match the dialog title specified in message 42 of set 5)
109 F
110 複製到檔案(F)...

$ mnemonic and label for the "Wrap To Fit" toggle button
111 W
112 自動換行(W)


$ ***********************************************************
$ **                                              helpCB.c **
$ **                                                       **
$ ** Messages and strings associated with online help.     **
$ **                                                       **
$ ***********************************************************
$set 12
$ Message used for "Help - On Item" if the user clicks outside application
1 您必須在文字編輯器中選取一個項目。


$ ***********************************************************
$ **                                        ttMsgSupport.c **
$ **                                                       **
$ ** Messages and strings associated with ToolTalk.        **
$ **                                                       **
$ ***********************************************************
$set 13
1 檢查點儲存失敗。
2 檢查點回應失敗。

$ NEW message after the SAMPLE IMPLEMENTATION.... more errors/warnings can
$ be detected while reading in a file via ToolTalk.
3 無法設定文字編輯器的內容。 (原因不明).
