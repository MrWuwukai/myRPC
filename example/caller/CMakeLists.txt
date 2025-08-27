set(SRC_LIST userclient.cpp ../user.pb.cc)
add_executable(caller ${SRC_LIST})
target_link_libraries(caller myrpc protobuf)