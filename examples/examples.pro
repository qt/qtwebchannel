TEMPLATE = subdirs

SUBDIRS += nodejs

qtHaveModule(websockets) {
    SUBDIRS += standalone
}
