## Build the dfiles and hfiles components for each of the class
## modules.  Moved into this file to keep Makefile.am in dtinfo/src
## somewhat morereadable

# Support
include Support/Classlist.mk
Support/Support.d: Support/Classlist.mk
	cd Support && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles d Support $(Support_CLASSES)

Support/Support.h: Support/Classlist.mk
	cd Support && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles h Support $(Support_CLASSES)


# UAS
include UAS/Classlist.mk
UAS/UAS.d: UAS/Classlist.mk
	cd UAS && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles d UAS $(UAS_CLASSES)

UAS/UAS.h: UAS/Classlist.mk
	cd UAS && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles h UAS $(UAS_CLASSES)


# Basic
include Basic/Classlist.mk
Basic/Basic.d: Basic/Classlist.mk
	cd Basic && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles d Basic $(Basic_CLASSES)

Basic/Basic.h: Basic/Classlist.mk
	cd Basic && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles h Basic $(Basic_CLASSES)


# OliasSearch
include OliasSearch/Classlist.mk
OliasSearch/OliasSearch.d: OliasSearch/Classlist.mk
	cd OliasSearch && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles d OliasSearch $(OliasSearch_CLASSES)

OliasSearch/OliasSearch.h: OliasSearch/Classlist.mk
	cd OliasSearch && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles h OliasSearch $(OliasSearch_CLASSES)


# Marks
include Marks/Classlist.mk
Marks/Marks.d: Marks/Classlist.mk
	cd Marks && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles d Marks $(Marks_CLASSES)

Marks/Marks.h: Marks/Classlist.mk
	cd Marks && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles h Marks $(Marks_CLASSES)


# Graphics
include Graphics/Classlist.mk
Graphics/Graphics.d: Graphics/Classlist.mk
	cd Graphics && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles d Graphics $(Graphics_CLASSES)

Graphics/Graphics.h: Graphics/Classlist.mk
	cd Graphics && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles h Graphics $(Graphics_CLASSES)


# Preferences
include Preferences/Classlist.mk
Preferences/Preferences.d: Preferences/Classlist.mk
	cd Preferences && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles d Preferences $(Preferences_CLASSES)

Preferences/Preferences.h: Preferences/Classlist.mk
	cd Preferences && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles h Preferences $(Preferences_CLASSES)


# Managers
include Managers/Classlist.mk
Managers/Managers.d: Managers/Classlist.mk
	cd Managers && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles d Managers $(Managers_CLASSES)

Managers/Managers.h: Managers/Classlist.mk
	cd Managers && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles h Managers $(Managers_CLASSES)


# Other
include Other/Classlist.mk
Other/Other.d: Other/Classlist.mk
	cd Other && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles d Other $(Other_CLASSES)

Other/Other.h: Other/Classlist.mk
	cd Other && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles h Other $(Other_CLASSES)


# Query
include Query/Classlist.mk
Query/Query.d: Query/Classlist.mk
	cd Query && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles d Query $(Query_CLASSES)

Query/Query.h: Query/Classlist.mk
	cd Query && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles h Query $(Query_CLASSES)


# Agents
include Agents/Classlist.mk
Agents/Agents.d: Agents/Classlist.mk
	cd Agents && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles d Agents $(Agents_CLASSES)

Agents/Agents.h: Agents/Classlist.mk
	cd Agents && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles h Agents $(Agents_CLASSES)


# OnlineRender
include OnlineRender/Classlist.mk
OnlineRender/OnlineRender.d: OnlineRender/Classlist.mk
	cd OnlineRender && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles d OnlineRender $(OnlineRender_CLASSES)

OnlineRender/OnlineRender.h: OnlineRender/Classlist.mk
	cd OnlineRender && $(top_builddir)/../programs/dtinfo/tools/misc/dfiles h OnlineRender $(OnlineRender_CLASSES)
