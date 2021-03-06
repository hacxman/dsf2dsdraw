# Makefile for libiconv/man

#### Start of system configuration section. ####

# Directories used by "make":
srcdir = .

# Directories used by "make install":
prefix = /usr/local
local_prefix = /usr/local
exec_prefix = ${prefix}
datadir = ${prefix}/share
mandir = ${datadir}/man
docdir = $(datadir)/doc/libiconv
htmldir = $(docdir)

# Programs used by "make":
RM = rm -f


# Programs used by "make install":
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
mkinstalldirs = $(SHELL) ../autoconf/mkinstalldirs

#### End of system configuration section. ####

SHELL = /bin/sh

all :

install : force
	if [ ! -d $(DESTDIR)$(prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(prefix) ; fi
	if [ ! -d $(DESTDIR)$(datadir) ] ; then $(mkinstalldirs) $(DESTDIR)$(datadir) ; fi
	if [ ! -d $(DESTDIR)$(mandir) ] ; then $(mkinstalldirs) $(DESTDIR)$(mandir) ; fi
	if [ ! -d $(DESTDIR)$(mandir)/man1 ] ; then $(mkinstalldirs) $(DESTDIR)$(mandir)/man1 ; fi
	builddir="`pwd`"; cd $(srcdir) && for f in *.1 ; do (cd "$$builddir"; echo $(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(mandir)/man1/$$f ; $(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(mandir)/man1/$$f) ; done
	if [ ! -d $(DESTDIR)$(mandir)/man3 ] ; then $(mkinstalldirs) $(DESTDIR)$(mandir)/man3 ; fi
	builddir="`pwd`"; cd $(srcdir) && for f in *.3 ; do (cd "$$builddir"; echo $(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(mandir)/man3/$$f ; $(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(mandir)/man3/$$f) ; done
	if [ ! -d $(DESTDIR)$(datadir)/doc ] ; then $(mkinstalldirs) $(DESTDIR)$(datadir)/doc ; fi
	if [ ! -d $(DESTDIR)$(docdir) ] ; then $(mkinstalldirs) $(DESTDIR)$(docdir) ; fi
	if [ ! -d $(DESTDIR)$(htmldir) ] ; then $(mkinstalldirs) $(DESTDIR)$(htmldir) ; fi
	builddir="`pwd`"; cd $(srcdir) && for f in *.html ; do (cd "$$builddir"; echo $(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(htmldir)/$$f ; $(INSTALL_DATA) $(srcdir)/$$f $(DESTDIR)$(htmldir)/$$f) ; done

installdirs : force
	if [ ! -d $(DESTDIR)$(prefix) ] ; then $(mkinstalldirs) $(DESTDIR)$(prefix) ; fi
	if [ ! -d $(DESTDIR)$(datadir) ] ; then $(mkinstalldirs) $(DESTDIR)$(datadir) ; fi
	if [ ! -d $(DESTDIR)$(mandir) ] ; then $(mkinstalldirs) $(DESTDIR)$(mandir) ; fi
	if [ ! -d $(DESTDIR)$(mandir)/man1 ] ; then $(mkinstalldirs) $(DESTDIR)$(mandir)/man1 ; fi
	if [ ! -d $(DESTDIR)$(mandir)/man3 ] ; then $(mkinstalldirs) $(DESTDIR)$(mandir)/man3 ; fi
	if [ ! -d $(DESTDIR)$(datadir)/doc ] ; then $(mkinstalldirs) $(DESTDIR)$(datadir)/doc ; fi
	if [ ! -d $(DESTDIR)$(docdir) ] ; then $(mkinstalldirs) $(DESTDIR)$(docdir) ; fi
	if [ ! -d $(DESTDIR)$(htmldir) ] ; then $(mkinstalldirs) $(DESTDIR)$(htmldir) ; fi

uninstall : force
	cd $(srcdir) && for f in *.1 ; do echo $(RM) $(DESTDIR)$(mandir)/man1/$$f ; $(RM) $(DESTDIR)$(mandir)/man1/$$f ; done
	cd $(srcdir) && for f in *.3 ; do echo $(RM) $(DESTDIR)$(mandir)/man3/$$f ; $(RM) $(DESTDIR)$(mandir)/man3/$$f ; done
	cd $(srcdir) && for f in *.html ; do echo $(RM) $(DESTDIR)$(htmldir)/$$f ; $(RM) $(DESTDIR)$(htmldir)/$$f ; done

check :

mostlyclean :

clean :

distclean : force
	$(RM) Makefile

maintainer-clean : distclean

force :

