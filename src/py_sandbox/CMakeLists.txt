add_executable(py_sandbox
    main.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/PythonBind/BindApp.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/PythonBind/BindMath.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/PythonBind/BindInput.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/PythonBind/BindInputActions.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/PythonBind/BindRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/PythonBind/BindScene.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/PythonBind/PyEngine.cpp
)

target_include_directories(py_sandbox PRIVATE
    ${CMAKE_SOURCE_DIR}/src/engine/PythonBind
    ${CMAKE_SOURCE_DIR}/src/engine/Private
)

target_link_libraries(py_sandbox
    PRIVATE
    engine
    pybind11::embed
)

target_copy_assets(py_sandbox)
target_copy_scripts(py_sandbox)

set_target_properties(py_sandbox PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
