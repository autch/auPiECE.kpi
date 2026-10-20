cmake_minimum_required(VERSION 3.21)
project(aupiece.kpi LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 配布する .kpi が VC++ ランタイムや libstdc++ の DLL に依存しないようにする
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(libmuslib)

add_library(aupiece SHARED
  auPiECE.cpp
  auPiECE.def
  KpiPmdDecoder.cpp
  KpiPmdDecoder.h
  KpiPmdDecoderModule.cpp
  KpiPmdDecoderModule.h
  kpi_mingw_uuid.h
  version.h
)
target_include_directories(aupiece PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/kpisdk)
target_compile_definitions(aupiece PRIVATE UNICODE _UNICODE NOMINMAX)
target_link_libraries(aupiece PRIVATE muslib)

if(MSVC)
  target_compile_options(aupiece PRIVATE /utf-8 /W3)
else()
  # SDK ヘッダの __declspec(uuid()) は kpi_mingw_uuid.h で補うので、gcc の警告は抑える
  target_compile_options(aupiece PRIVATE -Wall $<$<CXX_COMPILER_ID:GNU>:-Wno-attributes>
    $<$<CXX_COMPILER_ID:Clang>:-fms-extensions>)
  target_link_options(aupiece PRIVATE -static)
endif()

set_target_properties(aupiece PROPERTIES
  OUTPUT_NAME auPiECE
  PREFIX ""
  SUFFIX ".kpi")

# 検証用ホスト。ビルドした .kpi を通した出力を libmuslib の golden と照合する
option(AUPIECE_BUILD_TOOLS "Build kpi-render and the golden tests" ON)
if(AUPIECE_BUILD_TOOLS)
  add_executable(kpi-render tools/kpi-render.cpp)
  target_include_directories(kpi-render PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/kpisdk
    ${CMAKE_CURRENT_SOURCE_DIR}/libmuslib/include)
  target_compile_definitions(kpi-render PRIVATE UNICODE _UNICODE NOMINMAX)
  if(MSVC)
    target_compile_options(kpi-render PRIVATE /utf-8 /W3)
  else()
    target_compile_options(kpi-render PRIVATE -Wall $<$<CXX_COMPILER_ID:GNU>:-Wno-attributes>
      $<$<CXX_COMPILER_ID:Clang>:-fms-extensions>)
    target_link_options(kpi-render PRIVATE -municode -static)
  endif()
  # kpi-render は同じディレクトリの auPiECE.kpi を使うので、出力先をそろえる
  set_target_properties(aupiece kpi-render PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
  add_dependencies(kpi-render aupiece)

  # golden の照合には Node.js が要る。クロスビルドではテストを登録しない
  # (Wine で試すときは tools/golden.mjs に wine を通すラッパーを --cli で渡す)
  enable_testing()
  find_program(NODE_EXECUTABLE node)
  set(muslib_dir ${CMAKE_CURRENT_SOURCE_DIR}/libmuslib)
  if(NOT CMAKE_CROSSCOMPILING)
    file(GLOB fixtures ${muslib_dir}/test/fixtures/*.pmd)
    add_test(NAME seek-check COMMAND kpi-render --seek-check ${fixtures})
    if(NODE_EXECUTABLE)
      add_test(NAME golden-fixtures
        COMMAND ${NODE_EXECUTABLE} tools/golden.mjs check
          --set test --pmd-dir test/fixtures --cli $<TARGET_FILE:kpi-render>
        WORKING_DIRECTORY ${muslib_dir})
    endif()
  endif()
endif()
