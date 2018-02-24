# source file
file(GLOB DIR_HEADER "*.h" "*.hpp")
file(GLOB DIR_SOURCE "*.c" "*.cpp")
list(APPEND DIR_HEADER ${INCLUDE_DIR}/EventLoop.hpp)

source_group(header FILES ${DIR_HEADER})
source_group(source FILES ${DIR_SOURCE})

# target
add_library(${PROJECT_NAME} STATIC ${DIR_SOURCE} ${DIR_HEADER})