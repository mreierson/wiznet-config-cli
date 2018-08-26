set(MAIN_SRC
	main.cpp
	wiznet.cpp
)

include_directories(
	"${CMAKE_SOURCE_DIR}/src"
	${BOOST_INCLUDES}
)

set(Boost_USE_STATIC_LIBS ON)
set(BOOST_LIBRARY_DIRS /usr/local/lib/)
find_package(Boost REQUIRED COMPONENTS thread program_options system)

set(BOOST_LIBRARIES ${Boost_LIBRARIES} z)

add_executable(${PROJECT_NAME} ${MAIN_SRC})

target_link_libraries(${PROJECT_NAME}
	${BOOST_LIBRARIES}
	pthread
	dl
)
