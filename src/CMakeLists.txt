
set(EXEC intelCamera)
set(EXEC_TEST testLIC)
set(EXEC_SYNC syncTest)
set(EXEC_UV decodeUV)
set(LIBINTEL libIntelCamera)


set(SOURCE_EXEC main.cpp)

set(SOURCE_TEST testIVSclass.cpp )

set(SOURCE intelvideosource.cpp)

set(SOURCE_SYNCTEST syncQueueTest.cpp)

set(SOURCE_UV decodeUV.cpp)

set(HEADERS ../include/intelvideosource.h)

add_executable(${EXEC} ${SOURCE_EXEC} ${SOURCE}  ${HEADERS})
add_executable(${EXEC_TEST} ${SOURCE_TEST} ${SOURCE}  ${HEADERS})
add_executable(${EXEC_SYNC} ${SOURCE_SYNCTEST})
add_executable(${EXEC_UV} ${SOURCE_UV})
add_library(${LIBINTEL} ${SOURCE} ${HEADERS})

target_link_libraries(${EXEC} ${IntelSDK_LIBS} ${OpenCV_LIBS})
target_link_libraries(${LIBINTEL}  ${IntelSDK_LIBS} ${OpenCV_LIBS})
target_link_libraries(${EXEC_TEST} ${IntelSDK_LIBS} ${OpenCV_LIBS} ${Qt5Widgets_LIBRARIES} ${Qt5Multimedia_LIBRARIES})
target_link_libraries(${EXEC_SYNC} ${Qt5Widgets_LIBRARIES} ${Qt5Multimedia_LIBRARIES})
target_link_libraries(${EXEC_UV} ${OpenCV_LIBS})

install(TARGETS ${EXEC} DESTINATION .)
install(TARGETS ${EXEC_TEST} DESTINATION .)
install(TARGETS ${LIBINTEL} DESTINATION .)
install(TARGETS ${EXEC_SYNC} DESTINATION .)
install(TARGETS ${EXEC_UV} DESTINATION .)

