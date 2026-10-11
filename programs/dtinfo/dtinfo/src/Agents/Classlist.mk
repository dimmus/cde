# $XConsortium: Classlist.mk /main/8 1996/10/30 14:39:26 drk $

Agents_CLASSES= \
Agent \
Dialog \
ListView \
SearchHistoryListView \
NodeHistoryAgent \
MessageAgent \
IcccmAgent \
AnchorCanvas \
MarkIcon \
MarkCanvas \
NodeWindowAgent \
NodePrintAgent \
BookmarkEdit \
NodeListAgent \
SearchResultsAgent \
MapAgent \
OutlineListView \
SearchScopeList \
SearchScopeAgent \
BookTab \
PrintPanelAgent \
PrefAgent \
GraphicAgent \
LibraryAgent \
HelpAgent \
MarkListView \
MarkChooser \
ScopeMenu \
UrlAgent \
AddLibraryAgent


Agents_SRCS = %reldir%/AgentMotif.C \
%reldir%/MessageAgentMotif.C \
%reldir%/IcccmAgent.C \
%reldir%/AnchorCanvas.C \
%reldir%/MarkIconMotif.C \
%reldir%/MarkCanvas.C \
%reldir%/NodePrintAgent.C \
%reldir%/NodeWindowAgentMotif.C \
%reldir%/NodeListAgentMotif.C \
%reldir%/NodeHistoryAgentMotif.C \
%reldir%/SearchResultsAgentMotif.C \
%reldir%/ListViewMotif.C \
%reldir%/SearchHistoryListViewMotif.C \
%reldir%/MapAgentMotif.C \
%reldir%/OutlineListViewMotif.C \
%reldir%/SearchScopeList.C \
%reldir%/SearchScopeAgentMotif.C \
%reldir%/BookTabMotif.C \
%reldir%/LibraryAgentMotif.C \
%reldir%/BookmarkEditMotif.C \
%reldir%/PrintPanelAgentMotif.C \
%reldir%/PrefAgentMotif.C \
%reldir%/GraphicAgentMotif.C \
%reldir%/HelpAgent.C \
%reldir%/MarkListView.C \
%reldir%/MarkChooser.C \
%reldir%/ScopeMenu.C \
%reldir%/UrlAgentMotif.C \
%reldir%/AddLibraryAgentMotif.C

Agents_MSGS = \
%reldir%/Agent.msg \
%reldir%/AddLibraryAgent.msg \
%reldir%/BookmarkEdit.msg \
%reldir%/GraphicAgent.msg \
%reldir%/LibraryAgent.msg \
%reldir%/MapAgent.msg \
%reldir%/MarkList.msg \
%reldir%/MarkChooser.msg \
%reldir%/MessageAgent.msg \
%reldir%/NodeHistoryAgent.msg \
%reldir%/NodePrintAgent.msg \
%reldir%/NodeWindowAgent.msg \
%reldir%/PrefAgent.msg \
%reldir%/PrintPanelAgent.msg \
%reldir%/SearchHistoryList.msg \
%reldir%/SearchResultsAgent.msg \
%reldir%/SearchScopeAgent.msg \
%reldir%/UrlAgent.msg

