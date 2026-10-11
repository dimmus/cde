# $XConsortium: Classlist.mk /main/3 1996/10/30 14:40:00 drk $
Basic_CLASSES=\
Error \
FolioObject \
BitField \
Iterator \
Hashable \
StringHash \
OString \
FString \
NamedData \
Preferences \
NodeID \
OrderList \
List \
Atomizer \
HashTbl \
OutlineElement \
TOC_Element \
OutlineString \
OutlineList \
VisualInfo \
Buffer \
Long_Lived \
ClientEventHandler \
NodeDisplayData \
NodeViewInfo

Basic_SRCS = \
%reldir%/FolioObject.C \
%reldir%/BitField.C \
%reldir%/OString.C \
%reldir%/FString.C \
%reldir%/OrderList.C \
%reldir%/List.C \
%reldir%/Atomizer.C \
%reldir%/HashTbl.C \
%reldir%/StringHash.C \
%reldir%/OutlineElement.C \
%reldir%/OutlineList.C \
%reldir%/TOC_Element.C \
%reldir%/Buffer.C \
%reldir%/Long_Lived.C \
%reldir%/NodeViewInfo.C
