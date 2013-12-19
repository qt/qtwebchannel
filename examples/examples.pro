TEMPLATE = subdirs

SUBDIRS += standalone

qtHaveModule(quick) {
    SUBDIRS += \
        hybridshell \
        qtobject
}
