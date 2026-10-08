$ $XConsortium: dtprintinfo.msg /main/2 1996/07/24 19:03:15 drk $
$quote "
$set DTPRINTER_SET

$ ----- These are the titles that appear on dialogs and the main window -----
ApplicationName1L  "列印管理者"
ApplicationName2L  "印表機工作"
$ Do not translate ApplicationName3L for this release
ApplicationName3L  "桌面印表機"

$ ----- Common Dialog Buttons -----
YesL     "是"
NoL      "否"
OKL      "確定"
ApplyL   "即刻應用"
CancelL  "取消"
HelpL    "輔助說明"

$ ----- Find dialog -----
FindTitleL        "%s - 尋找列印工作"
MyJobNameL        "我的工作的名稱"
OnlyRootJobsL     "僅有根工作"
IgnoreCaseL       "忽略案例"
ExactMatchL       "正確的符合"
GotoL             "前往"
CancelPrintJobsL  "取消列印工作"
PositionL         "位置"
CloseL            "關閉"
StartFindL        "啟動尋找"
StopFindL         "停止尋找"
SearchL           "\
搜尋 %s...\n\
\n\
請按「停止」來中斷"

$ Find Information dialog messages
NoMatchesL             "無列印工作符合"

NotFoundMessageL       "\
列印工作不在存在，或\n\
印表機被關閉。"

$ ----- Modify Show Dialog -----
ModifyShowTitleL         "%s - 修改顯示列表"
SelectPrintersToBeShowL  "選取欲顯示的印表機"
SelectAllL               "全部選取"
DeselectAllL             "取消全部選取"

$ ----- Set Options Dialog -----
SetOptionsTitleL      "%s - 設定選項"
RepresentationL       "表現方式"
LargeIconL            "大圖像"
SmallIconL            "小圖像"
NameOnlyL             "僅有名稱"
DetailsL              "細節"
ShowLabelsL           "顯示標籤"
JobsToShowL           "欲顯示的工作"
OnlyMineL             "僅有我的"
EveryoneL             "每個人的"
StatusL               "狀態"
ProblemFlagL          "問題旗標"
InformationLineL      "訊息列"
UpdateIntervalL       "更新"
IntervalL             "時間間隔(秒)"

$ ----- Print Job Properties Dialog -----
PrintJobPropsTitleL  "%s - 列印工作特性"
PrinterL             "印表機"
JobName1L            "工作名稱"
SubmittedL           "已送出"
BytesL               "位元組"

Position1L           "位置："
JobNameL             "工作名稱："
JobNumberL           "工作號碼："
OwnerL               "擁有者："
DateL                "日期："
TimeL                "時間："
SizeL                "大小："

$ ----- Printer Properties Dialog -----
PrinterPropsTitleL    "%s - 印表機特性"
IconLabelL            "圖像標籤："
IconGroupTitleL       "圖像："
FindSetL              "尋找集合..."
DescriptionL          "說明："
PrinterStatusL        "狀態："
PrintQueueL           "印表機佇列："
DeviceL               "裝置名稱："
DeviceNL              "裝置 %d："
UpL                   "向上 - 服務中"
DownL                 "向下 - 結束服務"
PrinterOnServerL      "%1$s on %2$s"
DefaultDescriptionL   "這是 %s 印表機佇列。"
$ %1 is the printer name and %2 is the server name, ex： "lp0 on warpspeed"
$ This is the value of the device field when the printer is on a remote system

$ ----- Find Set Dialog -----
FindSetTitleL       "%s - 尋找集合"
IconFoldersL        "圖像檔案夾："
IconTitleL          "圖像"
LoadingIconsL       "載入%2$d個中的 %1$d個圖像..."
$ %1 is a counter and %2 is the total number of icons

$ ----- "Cancel" Print Job Confirmation Dialog -----
CancelTitleL     "%s - 取消 %s"
CancelQuestionL  "您確定要取消%s嗎?"

$ ----- Printer "Status" Dialog -----
PrinterStatusTitleL  "%s - 印表機狀態"
PrinterDownL         "%s印表機結束服務。"
DeviceDownL          "%s裝置結束服務。"

$ ----- Failed Action Dialog -----
FailedActionTitleL  "%s - %s 失敗"
FailedMessageL      "無法即刻應用'%s'動作至\n\
  %s.  請參閱以下原因。\n\n%s"

$ ----- Main Window Information Line Messages -----
GettingHelpL      "取得線上輔助說明..."
SearchingL        "搜尋印表機..."
ShowingMyJobsL    "僅顯示我的工作"
PrintersHiddenL   "%d 部印表機中有 %d部被隱藏"
JobsL             "%d個工作"
UpdatingActionsL  "更新動作資料庫..."
UpdatingL         "更新%s..."
$ Do not translate EditLangL for this release
EditLangL         "編輯於/etc/dt/appconfig/類別/%s"
$ %s is the name of the printer

$ ----- Printer Menu Messages -----
PrinterMenuL         "印表機"
PrinterAcceleratorL  "P"
FindChoiceL          "尋找..."
FindMnemonicL        "F"
FindAcceleratorL     "Ctrl+F"
ExitChoiceL          "結束"
ExitMnemonicL        "x"
ExitAcceleratorL     "Alt+F4"

$ ----- Selected Menu Messages -----
SelectedMenuL           "已選取"
SelectedAcceleratorL    "S"

$ For Print Jobs Only - "Cancel" means to remove a print job from the printer.
$                       This choice pops up the "Cancel" Print Job Confirmation
$                       Dialog. 
CancelChoiceL           "取消"
CancelMnemonicL         "C"
CancelAcceleratorL      "Del"

$ For Printers Only
OpenChoiceL             "開啟"
OpenMnemonicL           "O"
CloseChoiceL            "關閉"
CloseMnemonicL          "C"
HideChoiceL             "隱藏"
HideMnemonicL           "e"
$ Do not translate RenameChoiceL for this release
RenameChoiceL           "重新命名"
$ Do not translate RenameMnemonicL for this release
RenameMnemonicL         "R"
$ Do not translate StartChoiceL for this release
StartChoiceL            "開始排隊"
$ Do not translate StartMnemonicL for this release
StartMnemonicL          "S"
$ Do not translate StopChoiceL for this release
StopChoiceL             "結束排隊"
$ Do not translate StopMnemonicL for this release
StopMnemonicL           "t"
$ Do not translate EnableChoiceL for this release
EnableChoiceL           "啟動列印"
$ Do not translate EnableMnemonicL for this release
EnableMnemonicL         "E"
$ Do not translate DisableChoiceL for this release
DisableChoiceL          "關閉列印"
$ Do not translate DisableMnemonicL for this release
DisableMnemonicL        "D"

$ For Both Printers and Print Jobs
PropertiesChoiceL       "特性..."
PropertiesMnemonicL     "P"
PropertiesAcceleratorL  "Ctrl+Backspace"
HelpChoiceL             "輔助說明"
HelpMnemonicL           "H"

$ ----- View Menu Messages -----
ViewMenuL             "檢視"
ViewAcceleratorL      "V"
SetOptionsChoiceL     "設定選項..."
SetOptionsMnemonicL   "O"
ModifyShowChoiceL     "修改顯示列表..."
ModifyShowMnemonicL   "M"

$ ----- Help Menu Messages -----
$ "Help" Menu uses the "HelpChoiceL" message
$ "Help" Accelerator uses the "HelpMnemonicL" message
OverviewChoiceL     "概觀"
OverviewMnemonicL   "v"
TaskChoiceL         "工作"
TaskMnemonicL       "T"
ReferenceChoiceL    "參考資料"
ReferenceMnemonicL  "R"
OnItemChoiceL       "高於項目"
OnItemMnemonicL     "O"
UsingHelpChoiceL    "使用輔助說明"
UsingHelpMnemonicL  "U"
AboutChoice1L       "關於列印管理者"
AboutChoice2L       "關於列印工作"
$ Do not translate AboutChoice3L for this release
AboutChoice3L       "關於桌面印表機"
AboutMnemonicL      "A"

$ ----- Misc Messages -----
EmptyL            "[ 空的 ]"
FoundNoPrintersL  "沒有找到印表機。"
NotAvailableL     "不可用"
NoDefaultPrinter1L  "沒有系統內設印表機。"
NoDefaultPrinter2L  "\
請參閱使用者引導說明已獲得如何\n\
設定內設印表機LPDEST環境的資訊。"
$ %1 is the name of the program, and %2 is either '-edit' or '-populate'.
RootUserL "%1$s： 您必須是根使用者才能使用%2$s選項。"


$ ----- Command Line Help -----
$ This message is show when the user enters 'dtprintinfo -help'
$ on the command line. The %s is a printer list generated by the program.

CommandLineHelpL "\
dtprintinfo\n\
\n\
    不參照任何選項而啟動dtprintinfo，顯示LPDEST\n\
    印表機或在LPDEST未設定時顯示系統內設。\n\
\n\
dtprintinfo -p <printer>\n\
\n\
    若要看特定的印表機，請使用 -p 選項以及\n\
    下列列印佇列名稱之一：\n\
\n\
%s\
\n\
dtprintinfo -all\n\
\n\
    若要看所有印表機，請使用 -all 選項。\n\
\n\
dtprintinfo -populate\n\
\n\
     -populate 選項僅供根使用者使用。\n\
    該選項對/etc/dt/appconfig/types/LANG\n\
中的所有印表機建立內設印表機動作。  若LANG未定義，\n\
    動作會建立在/etc/dt/appconfig/types/C中。"
