include(GNUInstallDirs)

set(PUBLIC_HEADER
)
set(SOURCES
    IPCRecv_Study.cpp
)

link_libraries(mq)
include_directories(${CMAKE_CURRENT_LIST_DIR}
                      ${CMAKE_CURRENT_LIST_DIR}/../../../qnx_interfaces/qnx_slog2/inc/
                        )
						
add_executable(ZStudy3
    ${SOURCES}
)

target_compile_features(ZStudy3 PRIVATE c_std_11)

target_compile_options(ZStudy3
    PRIVATE
        -Werror -Wall -Wextra -Wno-unused-parameter
)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/../config/SLM_ZStudy.xml
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/system/config/slm/)

install(FILES ${PUBLIC_HEADER}
         DESTINATION include)

install(TARGETS ZStudy3 EXPORT ZStudy3-config
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}        COMPONENT bin
)