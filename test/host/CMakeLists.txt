include_directories ( "include" )

set(TEST test_MeshMessageState)

set(TEST_SOURCE_DIR "test/host")

set(TEST_SOURCE_FILES ${SOURCE_DIR}/protocol/mesh/cs_MeshMessageState.cpp)
set(SOURCE_FILES ${TEST_SOURCE_DIR}/${TEST}.cpp ${TEST_SOURCE_FILES}) 
add_executable(${TEST} ${SOURCE_FILES})
add_test(NAME ${TEST} COMMAND ${TEST})


set(TEST test_MeshMessageCounter)

set(TEST_SOURCE_DIR "test/host")

set(TEST_SOURCE_FILES ${SOURCE_DIR}/protocol/mesh/cs_MeshMessageCounter.cpp)
set(SOURCE_FILES ${TEST_SOURCE_DIR}/${TEST}.cpp ${TEST_SOURCE_FILES}) 
add_executable(${TEST} ${SOURCE_FILES})
add_test(NAME ${TEST} COMMAND ${TEST})