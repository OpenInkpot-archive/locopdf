MAINTAINERCLEANFILES = Makefile.in

EDC= $(wildcard *.edc)
EDJ = $(EDC:%.edc=%.edj)

EXTRA_DIST = $(EDC)
CLEANFILES = $(EDJ)

themedir = $(datadir)/eoi/themes/azbooka
theme_DATA = $(EDJ)

all: $(EDJ)
%.edj: %.edc
	edje_cc -id ${srcdir} -o $@ $<

