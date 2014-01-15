TEMPLATE = subdirs

SUBDIRS += webchannel

qtHaveModule(webkit):qtHaveModule(quick) {
    SUBDIRS += qml
}
