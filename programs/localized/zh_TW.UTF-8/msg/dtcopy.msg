$ $XConsortium: dtcopy.msg /main/3 1996/07/24 19:02:41 drk $
$ *****************************************************************************
$ **
$ **  File:        dtcopy.msg
$ **
$ **  Project:     CDE File Manager
$ **
$ *****************************************************************************

$set 2
$ ****** Module: main_dtcopy.c ******
3 檔案
4 檔案
5 子檔案夾
6 子檔案夾
7 錯誤： 無
8 錯誤： %d
10 用法： %s [選項 ...] 來源目標\n
11 檔案管理員 -- 檔案夾移動
12 檔案管理員 -- 檔案夾複製
13 因為錯誤，%s 未除去。
14 完成。
15 取消於
$ ** Message 20 - 32
$ ** LongFileOpNames ... message number = op + 20.
20 處理中
21 開啟中
22 開啟中
23 開啟中
24 讀取連結
25 刪除中
26 複製中
27 建立目錄
28 複製連結
29 建立連結
30 工作中
31 是一個複製
32 是一個連結
33 剩餘

$set 3
$ ****** Module: copydialog.c ******
1 繼續
2 暫停
3 移動中：
4 複製中：
5 至檔案夾：
7 這樣將停止複製程序。 任何已複製的\n\
檔案將留在目標檔案夾中。
10 這樣將停止移動程序。 任何已移動的\n\
檔案將留在目標檔案夾中。
11 檔案夾複製 -- 警告
12 檔案夾移動 -- 警告
13 取消複製
14 繼續複製
15 取消移動
16 繼續移動
17 已移動
18 已複製

$set 4
$ ****** Module: errordialog.c ******
1 錯誤 %s\n%s
2 因錯誤而暫停
3 檔案夾複製 -- 錯誤
4 檔案夾移動 -- 錯誤

5 "物件移動 - 錯誤"

6 "物件複製 - 錯誤"

7 "物件垃圾檔 - 錯誤"

8 "不允許您將物件 \n\n\
    %s\n\n\
放入垃圾檔.\n\n\
Use the Change Permissions choice from the object's\n\
popup menu or from the Selected menu to turn on your\n\
Read permission on the object.\n\n\n\
Note: If this object is a folder, you must also have\n\
Read permission for each of the objects inside the\n\
folder before you can put the folder in the trash."


$set 5
$ ****** Module: overwrtdialog.c ******
1 名稱為\n%s\n的檔案已存在檔案夾中\n%s
2 檔案夾移動 -- 檔案重寫
3 取代存在的檔案
4 將存在的檔案重新命名為：
5 將新物件重新命名為：
6 略過這個檔案
7 即刻將此動作應用於接續的名稱衝突中。
8 欲採取的動作：
9 檔案夾複製 -- 檔案重寫
10 %s\n已存在：請選擇另外的名稱。
11 檔案重寫 -- 錯誤
12 %s\n無法重新命名為\n%s\n%s

$set 6
$ ****** Module: utils.c ******
$ ** Button labels.
1 確定
2 暫停
3 取消
4 輔助說明
5 目錄複製輔助說明
6 繼續
7 取消複製
8 取消移動
