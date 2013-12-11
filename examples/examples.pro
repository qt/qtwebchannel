TEMPLATE = subdirs

qtHaveModule(quick) {
    SUBDIRS += \
        hybridshell \
        qtobject
}
