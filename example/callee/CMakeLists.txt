set(SRC_LIST userservice.cpp ../user.pb.cc)
add_executable(callee ${SRC_LIST})
target_link_libraries(callee myrpc protobuf)