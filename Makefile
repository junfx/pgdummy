# $PostgreSQL: pgsql/contrib/plpgsql_check/Makefile

MODULE_big = pg_dummy
OBJS = pg_dummy.o
DATA = pg_dummy--1.0.sql
EXTENSION = pg_dummy

REGRESS = init pg_dummy

ifdef NO_PGXS
subdir = contrib/pg_dummy
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
else
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
endif

override CFLAGS += -Wextra

