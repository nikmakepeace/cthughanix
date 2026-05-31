## -*- makefile -*-

## table cmd programs
cmd_huricn_SOURCES = cmd_huricn.c imath.c
cmd_huricn_LDADD = -lm

cmd_smoke_SOURCES = cmd_smoke.c imath.c
cmd_smoke_LDADD = -lm

cmd_space_SOURCES = cmd_space.c imath.c
cmd_space_LDADD = -lm

cmd_gentable_SOURCES = cmd_gentable.c imath.c
cmd_gentable_LDADD = -lm

cmd_bighalfwheel_SOURCES = cmd_bighalfwheel.c imath.c
cmd_bighalfwheel_LDADD = -lm

cmd_downspiral_SOURCES = cmd_downspiral.c imath.c
cmd_downspiral_LDADD = -lm

cmd_randswirls_SOURCES = cmd_randswirls.c
cmd_randswirls_LDADD = -lm

cmdRead_SOURCES = cmdRead.cc
cmdRead_LDADD = -lm


# call this noinst, because it gets installed manually
noinst_PROGRAMS = cmd_huricn cmd_smoke cmd_space cmd_gentable 		\
		  cmd_bighalfwheel cmd_downspiral cmdRead		\
		  cmd_randswirls

noinst_DATA = 	randswirls.cmd						\
		bighalfwheel.cmd  hurricane.cmd     space-fast.cmd	\
		downspiral.cmd    rotate.cmd        space-slow.cmd	\
		gentable.cmd      smoke.cmd         yin-yang.cmd	\
		cmdRead.cmd



## install the thing
install-data-local: ${noinst_PROGRAMS} ${noinst_DATA}
	${INSTALL} -d ${pkglibdir}/tab
	-cp ${noinst_PROGRAMS}	${pkglibdir}/tab
	-cp ${noinst_DATA}	${pkglibdir}/tab


EXTRA_DIST = 	randswirls.cmd						\
		bighalfwheel.cmd  hurricane.cmd     space-fast.cmd	\
		downspiral.cmd    rotate.cmd        space-slow.cmd	\
		gentable.cmd      smoke.cmd         yin-yang.cmd	\
		cmdRead.cmd


dist-hook:
	-ln -s ../precompiled $(distdir)/../precompiled
	cp -p $(pkglibdir)/tab/cmd_huricn	$(distdir)/../precompiled/tab/cmd_huricn
	cp -p $(pkglibdir)/tab/cmd_smoke 	$(distdir)/../precompiled/tab/cmd_smoke
	cp -p $(pkglibdir)/tab/cmd_space 	$(distdir)/../precompiled/tab/cmd_space
	cp -p $(pkglibdir)/tab/cmd_gentable	$(distdir)/../precompiled/tab/cmd_gentable
	cp -p $(pkglibdir)/tab/cmd_bighalfwheel	$(distdir)/../precompiled/tab/cmd_bighalfwheel
	cp -p $(pkglibdir)/tab/cmd_downspiral	$(distdir)/../precompiled/tab/cmd_downspiral
	cp -p $(pkglibdir)/tab/cmd_randswirls	$(distdir)/../precompiled/tab/cmd_randswirls
