## -*- Makefile -*-

SUBDIRS = src tab doc 

EXTRA_DIST = cthugha-L.lsm Makefile.pre  cthugha.ini.eg tga2jpeg

dist-hook:
	cp Makefile.pre $(distdir)/Makefile
	ln -s ../pcx $(distdir)/pcx
	ln -s ../map $(distdir)/map
# ln -s ../precompiled $(distdir)/precompiled


## install the pcx and palette
install-data-local: ${noinst_PROGRAMS} ${noinst_DATA}
	${INSTALL} -d ${pkglibdir}/pcx
	${INSTALL} -d ${pkglibdir}/map
	-cp pcx/* ${pkglibdir}/pcx
	-cp map/* ${pkglibdir}/map




