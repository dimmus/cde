$ $XConsortium: dthelpgen.msg /main/3 1995/11/08 14:06:29 rswiston $
$ *****************************************************************************
$ 
$ ***** NOTE FOR MESSAGE CATALOG TRANSLATORS *****
$ 
$ There may be three types of messages in this file:
$ 
$  1. Messages that appear in dialogs or are displayed to the user.
$ 
$     These messages are the default and they should ALL BE LOCALIZED.
$     Note that these messages do NOT have any identification (see the
$     comments for type 2 and 3 below).
$ 
$  2. Messages that only appear in the DT error log file ($HOME/.dt/errorlog).
$ 
$     The localization of these messages is OPTIONAL.  These messages are
$     identified by the following:
$ 
$  MESSAGES xx-yy IN SET zz WILL ONLY APPEAR IN THE DT ERRORLOG FILE
$ 
$  3. Messages that should not be localized.
$ 
$     These messages are identified by the following:
$ 
$  DO NOT TRANSLATE or CHANGE or LOCALIZE MESSAGES xx-yy from set zz
$ 
$ ***** END (NOTE FOR MESSAGE CATALOG TRANSLATORS) *****
$ 
$ ******************************************************************************
$ 
$set 1
$ 
$ _DtMessage 1 is the usage message
$ 
$ Localize only the words 'directory', 'name', and 'language'.
$ 'directory' means the directory name in which the resulting files
$ will be placed. 'name' mean what base name to give the files placed
$ in 'directory'. 'language' means which localized versions of help files
$ to look for.
$ 
1 %s -dir <目錄> [-generate] [-file <名稱>] [-lang <語言>]\n

$ 
$ Mesages 2-18 are error messages.
$ 
2 %s: %s 之元素不是目錄\n
3 %s: 目錄 %s 之存取被拒\n要以高階使用者身分再試一次嗎?\n
4 %s: %s 之元素不存在\n
5 %s: 包含 %s 之檔案系統已滿\n
6 %s: 無法存取 %s － 錯誤狀態號碼 %d\n
7 %s: 包含 %s 之檔案系統只能讀取\n
8 %s: 需要根使用者以寫入 %s\n
9 %s: 寫入 %s 不對\n
10 %s: 搜尋路徑是空的\n
11 %s: 欠缺 'title' 資源\n
12 %s: 欠缺 'abstract' 資源\n
13 %s: 欠缺 'volumes' 資源\n
14 %s: 欠缺 'character' 集資源\n
15 %s: 欠缺目標目錄\n
16 %s: 未發現系列檔案\n
17 %s: 未發現容體檔案\n
18 %s: 無法存取目前工作目錄 － 錯誤狀態號碼 %d\n
19 %s: 無法配置記憶體\n
20 %s: 指定的系統語言 %s 不對\n

$ 
$set 2
$ 
$ Specifies the character set used to create this file
$ 
$ This must be localized for the various languages. That is
$ for Japanese shift JIS, it would be 'ja_JP.SJIS'; for Japanese 
$ EUC it would be 'ja_JP.eucJP'. For files written using
$ HP Roman8 character set it would be '<lang+terr>.HP-ROMAN8', etc.
$ The set of allowable locale strings can be found in
$ /usr/dt/config/svc/CDE.lcx
$ 
1 zh_TW.EUC-TW

$ 
$ Specifies the title for the browser.
$ It is used in the body of text displayed to the user.
$ 
2 歡迎使用輔助說明管理員

$ 
$ Specifies the body of text displayed in the browser.
$ 
$ When localizing, DO NOT alter any text between the "<" and ">" pairs.
$ That is - DO NOT localize the tags such as <ABBREV> and <PARAGRAPH>.
$           DO     localize the text between the tags.
$ 
3 <ABBREV>歡迎使用輔助說明管理員</ABBREV>                          \
<PARAGRAPH>下列各項標題均代表\n               \
<ANGLE italic> 一種已在線上輔助說明</>\n                                      \
安裝並註冊的產品系列。 每項標題\n        \
(與圖像) 都是列出該系列中輔助說明的超連結。</>     \
<PARAGRAPH after 0 first 1 left 3 label "<CHAR C.DT-SYMBOL-1><0xB7></>"> \
若要顯示產品系列中可用的輔助說明的列表，請選擇\n  \
其標題 (下標文字) 或圖像。</PARAGRAPH>                        \
<PARAGRAPH after 0 first 1 left 3 label "<CHAR C.DT-SYMBOL-1><0xB7></>"> \
在一產品系列中，請尋找您要檢視的輔助說明，然後\n         \
選擇其標題。</PARAGRAPH>                                           \
<PARAGRAPH first 1 left 3 label "<CHAR C.DT-SYMBOL-1><0xB7></>">        \
如果您在使用輔助說明視窗時需要輔助說明，請按 F1。</PARAGRAPH>

$ 
$ Specifies the Volume Title
$ 
4 輔助說明 － 頂層

$ 
$ Specifies the preamble to the help file if no volumes or family files
$ are found.
$ 
$ When localizing, DO NOT alter any text between the "<" and ">" pairs.
$ That is - DO NOT localize the tags such as <ABBREV> and <PARAGRAPH>.
$           DO     localize the text between the tags.
$ 
5 <ABBREV>歡迎使用輔助說明管理員</ABBREV>                                \
<LINK 0 "Help4Help How-To-Register-Help">                                \
<TYPE serif><WEIGHT bold><SIZE 12><ANGLE italic>                         \
請注意：   沒有已註冊的輔助說明                                          \
</SIZE></WEIGHT></TYPE></></LINK>                                        \
<PARAGRAPH leftindent 3 firstindent 3>                                   \
<WEIGHT bold>沒有產品系列曾註冊其線上輔助說明檔案供瀏覽.</>              \
輔助說明可經由一些應用程式使用，請直接在應用程式中選擇輔助說明命令。</>

$ 
$ Specifies the title to use in the dthelpgen dtksh dialog.
$ 
6 dthelpgen

$ 
$ Specifies the message to display in the dthelpgen dtksh dialog.
$ This message indicates that dthelpgen is building (or rebuilding)
$ the browser information.
$ 
7 產生瀏覽程式資料中。 請稍候。

$ 
$set 3
$ 
$ ******* Text Formatting Templates ********
$ DO NOT TRANSLATE THESE MESSAGES
$ DO NOT CHANGE THESE MESSAGES
$ DO NOT LOCALIZE THESE MESSAGES
$ 
1 <TOPIC charset %s>
2 <TITLE><TYPE serif><WEIGHT bold><SIZE 14>%s</SIZE></WEIGHT></TYPE></TITLE>
3 <PARAGRAPH before 1 first 1 left 1>
4 <CHARACTERSET %s>
5 <TYPE serif><WEIGHT bold><SIZE 12><ANGLE italic>%s</></></></>
6 <PARAGRAPH before 1 first 1 left 1 graphic %s glink %s gtypelink 0>
7 <PARAGRAPH before 1 first 1 left 1 graphic %s glink "%s %s" gtypelink 1>
