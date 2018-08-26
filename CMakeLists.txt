cmake_minimum_required( VERSION 2.6 )

project( wiznet-config-cli )

include_directories(
        ${BOOST_INCLUDES}
)

add_subdirectory(src)
