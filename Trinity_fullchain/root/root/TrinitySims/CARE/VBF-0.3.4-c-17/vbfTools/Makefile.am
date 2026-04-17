AM_CPPFLAGS = -I$(top_srcdir)

bin_includedir=$(includedir)/VBF

bin_PROGRAMS = vbfCheck vbfConfig vbfList vbfPed vbfSummary vbfRepTel vbfReindex vbfZeroSup vbfZip
vbfCheck_SOURCES = vbfCheck.cpp
vbfCheck_LDADD = ../VBF/libVBF.la $(XTRA_LDFLAGS) $(XTRA_LIBS)

vbfConfig_SOURCES = vbfConfig.c
vbfConfig_LDADD =  ../VBF/libVBF.la $(XTRA_LDFLAGS) $(XTRA_LIBS)
vbfConfig_CFLAGS = -DPREFIX="\"$(prefix)\""\
	-DINCLUDE_DIR="\"$(DESTDIR)$(includedir)\""\
	-DLIB_DIR="\"$(DESTDIR)$(libdir)\""\
	-DXTRA_LDFLAGS="\"$(XTRA_LDFLAGS)\""\
	-DXTRA_CPPFLAGS="\"$(XTRA_CPPFLAGS)\""\
	-DXTRA_LIBS="\"$(XTRA_LIBS)\""\
	-DCXXFLAGS="\"$(CXXFLAGS)\""\
	-DCPPFLAGS="\"$(CPPFLAGS)\""\
	-DLDFLAGS="\"$(LDFLAGS)\""\
	-DLIBS="\"$(LIBS)\""\
	-DPACKAGE_STRING="\"VBF\ $(VERSION)\""\
	$(AM_CFLAGS)

vbfList_SOURCES = vbfList.cpp
vbfList_LDADD =  ../VBF/libVBF.la $(XTRA_LDFLAGS) $(XTRA_LIBS)

vbfPed_SOURCES = vbfPed.cpp
vbfPed_LDADD = ../VBF/libVBF.la $(XTRA_LDFLAGS) $(XTRA_LIBS)

vbfSummary_SOURCES = vbfSummary.cpp
vbfSummary_LDADD = ../VBF/libVBF.la $(XTRA_LDFLAGS) $(XTRA_LIBS)

vbfRepTel_SOURCES = vbfRepTel.cpp
vbfRepTel_LDADD = ../VBF/libVBF.la $(XTRA_LDFLAGS) $(XTRA_LIBS)

vbfReindex_SOURCES = vbfReindex.cpp
vbfReindex_LDADD = ../VBF/libVBF.la $(XTRA_LDFLAGS) $(XTRA_LIBS)

vbfZip_SOURCES = vbfZip.cpp
vbfZip_LDADD = ../VBF/libVBF.la $(XTRA_LDFLAGS) $(XTRA_LIBS)

vbfZeroSup_SOURCES = vbfZeroSup.cpp
vbfZeroSup_LDADD = ../VBF/libVBF.la $(XTRA_LDFLAGS) $(XTRA_LIBS)
