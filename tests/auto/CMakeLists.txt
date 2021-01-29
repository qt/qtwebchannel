# Generated from auto.pro.

# special case begin
# Qt5 cmake tests
#add_subdirectory(cmake)
# special case end
add_subdirectory(webchannel)
if(TARGET Qt::Quick)
    add_subdirectory(qml)
endif()
