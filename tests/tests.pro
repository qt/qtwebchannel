TEMPLATE = subdirs

SUBDIRS += webchannel

qtHaveModule(quick) {
    SUBDIRS += qml
}
