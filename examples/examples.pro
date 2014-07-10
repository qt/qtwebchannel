TEMPLATE = subdirs

qtHaveModule(websockets) {
    SUBDIRS += standalone nodejs
}
