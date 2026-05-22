add_library(common INTERFACE)
target_include_directories(common INTERFACE include)
target_compile_features(common INTERFACE cxx_std_20)
