# Generated from webchannel.pro.

# special case begin
# These are all TEMPLATE = aux
#add_subdirectory(nodejs)
#add_subdirectory(qwclient)
#add_subdirectory(chatclient-html)
# special case end
if(TARGET Qt::WebSockets)
    add_subdirectory(chatserver-cpp)
    # special case begin
    # This one is TEMPLATE = aux too
    #add_subdirectory(chatclient-qml)
    # special case end
endif()
if(TARGET Qt::WebSockets AND TARGET Qt::Widgets)
    add_subdirectory(standalone)
endif()
