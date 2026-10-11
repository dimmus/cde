# $XConsortium: Classlist.mk /main/11 1996/10/30 14:41:40 drk $
Managers_CLASSES= \
ErrorMgr \
MessageMgr \
StyleSheetMgr \
HistoryList \
NodeMgr \
SearchMgr \
NodeListMgr \
SearchResultsMgr \
MapMgr \
SearchScopeMgr \
LibraryMgr \
GraphicsMgr \
PrefMgr \
PrintMgr \
ClientEventMgr \
ServiceMgr \
SessionMgr \
WorkspaceMgr \
TtIpcMgr \
MarkMgr \
LocalHistoryMgr \
GlobalHistoryMgr \
InputMgrX \
UrlMgr \
EnvMgr

Managers_SRCS = %reldir%/MessageMgr.C \
%reldir%/StyleSheetMgr.C \
%reldir%/SearchMgr.C \
%reldir%/NodeMgr.C \
%reldir%/NodeListMgr.C \
%reldir%/SearchResultsMgr.C \
%reldir%/HistoryList.C \
%reldir%/MapMgr.C \
%reldir%/SearchScopeMgr.C \
%reldir%/LibraryMgr.C \
%reldir%/GraphicsMgr.C \
%reldir%/PrefMgr.C \
%reldir%/PrintMgr.C \
%reldir%/ClientEventMgrX.C \
%reldir%/ServiceMgrX.C \
%reldir%/SessionMgr.C \
%reldir%/WorkspaceMgr.C \
%reldir%/TtIpcMgr.C \
%reldir%/MarkMgr.C \
%reldir%/LocalHistoryMgr.C \
%reldir%/GlobalHistoryMgr.C \
%reldir%/InputMgrX.C \
%reldir%/UrlMgr.C \
%reldir%/CatMgr.C \
%reldir%/WString.C \
%reldir%/EnvMgr.C

Managers_MSGS = \
%reldir%/CatMgr.msg \
%reldir%/EnvMgr.msg \
%reldir%/TtIpcMgr.msg

