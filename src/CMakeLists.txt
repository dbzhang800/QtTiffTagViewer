
find_package(Qt6 REQUIRED COMPONENTS Widgets)

file(GLOB PROJECT_SOURCES *.cpp *.h *ui *.qrc *.rc)

qt_add_executable(tagviewer
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

set_target_properties(tagviewer PROPERTIES OUTPUT_NAME "QtTiffTagViewer")
target_compile_definitions(tagviewer PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(tagviewer PRIVATE Qt::Widgets)


include(GNUInstallDirs)
install(TARGETS tagviewer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(tagviewer)
