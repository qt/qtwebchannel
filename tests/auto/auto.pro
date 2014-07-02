TEMPLATE = subdirs

SUBDIRS += cmake webchannel

qtHaveModule(webkit):qtHaveModule(quick) {
    SUBDIRS += qml
}
