# to be included from component/po/CMakeLists.txt using include(Translations)
# requires setting GETTEXT_DOMAIN variable

find_package(Gettext)

set(COMPONENT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
file(GLOB_RECURSE POT_SOURCES RELATIVE ${COMPONENT_PATH} ${COMPONENT_PATH}/*.cpp ${COMPONENT_PATH}/*.hpp)

# target to refresh pot file from current sources
add_custom_target(${GETTEXT_DOMAIN}-pot
    COMMENT "Generating fresh ${GETTEXT_DOMAIN}.pot file from sources"
    COMMAND ${XGETTEXT_COMMAND} --output=po/${GETTEXT_DOMAIN}.pot ${POT_SOURCES}
    WORKING_DIRECTORY ${COMPONENT_PATH}
    )
add_dependencies(gettext-potfiles ${GETTEXT_DOMAIN}-pot)


# convert all *.po files to *.gmo and install them
if (GETTEXT_FOUND)
    # this process unfortunately reformats .po files so work on a copy
    file(GLOB POS ${CMAKE_CURRENT_SOURCE_DIR}/*.po)
    file(COPY ${POS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(GLOB POS_BIN ${CMAKE_CURRENT_BINARY_DIR}/*.po)
    GETTEXT_CREATE_TRANSLATIONS(${GETTEXT_DOMAIN}.pot ALL ${POS_BIN})
endif()
