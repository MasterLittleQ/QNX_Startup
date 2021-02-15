include(GNUInstallDirs)

set(PUBLIC_HEADER
)
set(SOURCES
    IPCSend_Study.c
)

link_libraries(mq)
include_directories(${CMAKE_CURRENT_LIST_DIR}
                      ${CMAKE_CURRENT_LIST_DIR}/../../../qnx_interfaces/qnx_slog2/inc/
                        )
						
add_executable(ZStudy2
    ${SOURCES}
)

target_compile_features(ZStudy2 PRIVATE c_std_11)

target_compile_options(ZStudy2
    PRIVATE
        -Werror -Wall -Wextra -Wno-unused-parameter
)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/../config/SLM_ZStudy.xml
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/system/config/slm/)

install(FILES ${PUBLIC_HEADER}
         DESTINATION include)

install(TARGETS ZStudy2 EXPORT ZStudy2-config
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}        COMPONENT bin
)