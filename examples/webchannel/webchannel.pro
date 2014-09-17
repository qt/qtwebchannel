TEMPLATE = subdirs

qtHaveModule(widgets):qtHaveModule(websockets) {
    SUBDIRS += standalone \
}

SUBDIRS += nodejs \
           chatserver-cpp \
           chatclient-html \
           chatclient-qml
