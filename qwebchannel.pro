TEMPLATE = subdirs

SUBDIRS = \
  src \
  examples \
  tests

examples.depends = src
tests.depends = src
