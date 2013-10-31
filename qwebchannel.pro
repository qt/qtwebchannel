TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
  src \
  examples

examples.depends = src