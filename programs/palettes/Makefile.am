palettedir = $(CDE_INSTALLATION_TOP)/share/palettes

dist_palette_DATA = Alpine.dp Chocolate.dp Grass.dp PBNJ.dp Tundra.dp	\
	Arizona.dp Cinnamon.dp GrayScale.dp Sand.dp Urchin.dp		\
	BeigeRose.dp Clay.dp Lilac.dp SantaFe.dp Wheat.dp Black.dp	\
	Crimson.dp Mustard.dp Savannah.dp WhiteBlack.dp BlackWhite.dp	\
	DarkGold.dp Neptune.dp SeaFoam.dp White.dp Broica.dp		\
	Default.dp NorthernSky.dp SkyRed.dp Cabernet.dp Delphinium.dp	\
	Nutmeg.dp SoftBlue.dp Camouflage.dp Desert.dp Olive.dp		\
	SouthWest.dp Charcoal.dp Golden.dp Orchid.dp Summer.dp

install-data-hook:
	$(RM) $(DESTDIR)$(CDE_INSTALLATION_TOP)/palettes
	cd $(DESTDIR)$(CDE_INSTALLATION_TOP)/ && $(LN_S) share/palettes palettes
