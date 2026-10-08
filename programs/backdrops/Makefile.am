backdropdir = $(CDE_INSTALLATION_TOP)/share/backdrops

dist_backdrop_DATA = Ankh.bm Crochet.pm Lattice.pm Paver.pm	\
SkyLight.pm Background.bm Foreground.bm Leaves.pm Pebbles.pm	\
Sprinkles.pm BrickWall.bm InlayColor.pm PinStripe.pm SunLogo.pm	\
Concave.pm InlayPlain.pm NoBackdrop.bm RakedSand.bm Toronto.bm	\
Convex.pm KnitLight.pm NoBackdrop.pm RicePaper.pm WaterDrops.pm	\
Corduroy.pm LatticeBig.pm OldChars.pm SkyDark.pm Wooly.pm

install-data-hook:
	$(RM) $(DESTDIR)$(CDE_INSTALLATION_TOP)/backdrops
	cd $(DESTDIR)$(CDE_INSTALLATION_TOP)/ && $(LN_S) share/backdrops backdrops

