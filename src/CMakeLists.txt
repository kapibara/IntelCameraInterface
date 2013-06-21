
set(EXEC intelCamera)
set(EXEC_TEST testLIC)
set(LIBINTEL libIntelCamera)

set(SOURCE_EXEC main.cpp)

set(SOURCE_TEST testIVSclass.cpp )

set(SOURCE intelvideosource.cpp)

set(HEADERS ../include/intelvideosource.h)

add_executable(${EXEC} ${SOURCE_EXEC} ${SOURCE}  ${HEADERS})
add_executable(${EXEC_TEST} ${SOURCE_TEST} ${SOURCE}  ${HEADERS})
add_library(${LIBINTEL} ${SOURCE} ${HEADERS})

target_link_libraries(${EXEC} ${IntelSDK_LIBS} ${OpenCV_LIBS})
target_link_libraries(${LIBINTEL}  ${IntelSDK_LIBS} ${OpenCV_LIBS})
target_link_libraries(${EXEC_TEST} ${IntelSDK_LIBS} ${OpenCV_LIBS})

install(TARGETS ${EXEC} DESTINATION .)
install(TARGETS ${EXEC_TEST} DESTINATION .)
install(TARGETS ${LIBINTEL} DESTINATION .)

