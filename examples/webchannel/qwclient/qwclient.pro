TEMPLATE = aux

exampleassets.files += \
    qwclient.js \
    package.json \
    README

exampleassets.path = $$[QT_INSTALL_EXAMPLES]/qwebchannel/qwclient
include(../exampleassets.pri)
