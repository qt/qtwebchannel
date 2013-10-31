TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
  3rdparty \
  src \
  examples

src.depends = 3rdparty
examples.depends = src