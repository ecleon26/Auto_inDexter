EXTENSION = auto_index
MODULES = auto_index
DATA = auto_index--1.0.sql

PG_CONFIG = /home/adnan/pg_custom_build/bin/pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)