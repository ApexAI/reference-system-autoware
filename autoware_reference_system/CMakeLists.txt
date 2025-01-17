cmake_minimum_required(VERSION 3.5)
project(autoware_reference_system)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake_auto REQUIRED)
ament_auto_find_build_dependencies()

# find_package(reference_system REQUIRED)

# Single Threaded Executor
ament_auto_add_executable(autoware_default_singlethreaded
  src/ros2/executor/autoware_default_singlethreaded.cpp
)

# Static Single Threaded Executor
ament_auto_add_executable(autoware_default_staticsinglethreaded
  src/ros2/executor/autoware_default_staticsinglethreaded.cpp
)

# Multi Threaded Executor
ament_auto_add_executable(autoware_default_multithreaded
  src/ros2/executor/autoware_default_multithreaded.cpp
)

# Add new executors to test here
#ament_auto_add_executable(autoware_default_custom
#  src/ros2/executor/autoware_default_custom.cpp
#)

if(${BUILD_TESTING})
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

  find_package(ros_testing REQUIRED)

  # run target for n seconds
  function(test_reference_system_autoware target timeout)
    set(TEST_EXECUTABLE ${CMAKE_CURRENT_BINARY_DIR}/${target})
    set(TEST_EXECUTABLE_NAME ${target}_${rmw_implementation})
    set(TIMEOUT ${timeout})
    set(RMW_IMPLEMENTATION ${rmw_implementation})

    # replaces all @var@ and ${var} within input file
    configure_file(
      test/test_${PROJECT_NAME}.py
      test_${target}_${rmw_implementation}.py
      @ONLY
    )

    add_ros_test(
      ${CMAKE_CURRENT_BINARY_DIR}/test_${target}_${rmw_implementation}.py
      TIMEOUT 10 # seconds
    )

    if(TARGET ${target})
      ament_target_dependencies(${target}
        "rclcpp" "reference_interfaces" "reference_system")
    endif()
  endfunction()

  # get available rmw implementations
  find_package(rmw_implementation_cmake REQUIRED)
  get_available_rmw_implementations(rmws_available)

  # loop over each rmw implmentation
  foreach(rmw_implementation ${rmws_available})
    find_package("${rmw_implementation}" REQUIRED)

    # add each exe to test
    test_reference_system_autoware(autoware_default_singlethreaded 10)
    test_reference_system_autoware(autoware_default_staticsinglethreaded 10)
    test_reference_system_autoware(autoware_default_multithreaded 10)
    # Add new exe's here to test
    #test_reference_system_autoware(autoware_default_custom)

  endforeach()
endif()

ament_auto_package(
  INSTALL_TO_SHARE test
)
