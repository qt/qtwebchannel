# These are all TEMPLATE = aux
#qt_internal_add_example(nodejs)
#qt_internal_add_example(qwclient)
#qt_internal_add_example(chatclient-html)

if(TARGET Qt::WebSockets)
    qt_internal_add_example(chatserver-cpp)

    # This one is TEMPLATE = aux too
    #qt_internal_add_example(chatclient-qml)

endif()
if(TARGET Qt::WebSockets AND TARGET Qt::Widgets)
    qt_internal_add_example(standalone)
endif()
