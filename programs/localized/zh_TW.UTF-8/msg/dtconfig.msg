$ $XConsortium: dtconfig.msg /main/3 1995/11/08 14:05:40 rswiston $
$ *****************************************************************************
$ **
$ **  File:        dtconfig.msg
$ **
$ **  Project:     CDE Session Management (dtconfig)
$ **
$ **  Description:
$ **  -----------
$ **  This file is the source for the message catalog for dtconfig
$ **
$ **  Message set 10: IBM
$ **  Message set 20: Sun
$ **
$ *****************************************************************************

$ *****************************************************************************
$ **  Message set 10: For IBM platforms
$ *****************************************************************************

$set 10
1 只能由高階使用者(superuser)執行。\n
$ 
2 CDE 現已設定成內定使用者介面。 此介面\n\
在系統上每一個使用者的登入時便會顯示。\n\
要看它是否發生效用，您必須關機後再重新開啟系統。\n
$ 
3 命令行現已設定成內定使用者介面。 此介面\n\
在系統上每一個使用者的登入時便會顯示。\n\
要看它是否發生效用，您必須關機後再重新開啟系統。\n
4 CDE 已經設定成內定使用者介面。
$ 
5 命令行已經設定成內定使用者介面。\n
$ 
6 用法: %s\n\
  -e (啟用 dtlogin 的 auto-start 功能)\n\
  -d (取消 dtlogin 的 auto-start 功能)\n\
  -kill (殺掉 dtlogin)\n\
  -reset (重設 dtlogin - 重新讀取配置檔)\n
$ 
7 太多引數 \n
$ 
8  需要一個引數 \n
$ 
9  無法取消 xdm，請參閱 xdm 線上援助頁以取消\n\
xdm 並再試一次。 \n 
$ 
10 Dtlogin 並未執行 \n

$ *****************************************************************************
$ **  Message set 20: For Sun Platforms
$ *****************************************************************************

$set 20
$ 
1  CDE 配置公用程式
$ 
2   %s -d     (取消 auto-start)
3   %s -e     (啟用 auto-start)
4   %s -kill  (殺掉 dtlogin)
5   %s -reset (重設 dtlogin)
6   %s -p     (印表機動作更新)
$ 
$	In above messages 2 through 6, only translate words between
$	left "(" and right ").
$ 
7 %s: 該 script 必須以 root 執行
8 用法: 太多引數
9 用法: 需要一個引數
10 選項失敗
11 無法執行 %s
12 取消失敗
13 無法除去 %s
14 完成 
15 桌上管理系統 auto-start 已取消。
16 啟用失敗
17 找不到 %s
18 無法建立 %s
19 無法建立 %s
20 桌上管理系統 auto-start 已啟用。
21 殺掉 dtlogin 完成。
22 dtlogin 配置資源已重新載入。
23 印表機動作更新已完成
24 用法:
