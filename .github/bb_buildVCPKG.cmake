function(bb_buildVCPKG)
  set(vcpkgPath "${CMAKE_BINARY_DIR}/vcpkg")
  # clone if it doesn't exist
  if(NOT EXISTS ${vcpkgPath})
    execute_process(COMMAND git clone https://github.com/microsoft/vcpkg.git
                            ${vcpkgPath} RESULT_VARIABLE GIT_CLONE_RESULT)
    if(NOT GIT_CLONE_RESULT EQUAL "0")
      message(
        FATAL_ERROR
          "git clone https://github.com/microsoft/vcpkg.git ${vcpkgPath} failed with ${GIT_CLONE_RESULT}, please checkout vcpkg"
      )
    endif()
    # todo checkout known commit: ff5afbab34834eaf1aebba808c47fea4fd606276
  endif()
  # bootstrap vcpkg
  if(WIN32)
    set(bootstrapExtension ".bat")
    set(triplet "x64-windows-static")
  else() # TODO add MacOS support
    set(bootstrapExtension ".sh")
    set(triplet "x64-linux")
  endif(WIN32)

  execute_process(
    COMMAND bootstrap-vcpkg${bootstrapExtension}
    WORKING_DIRECTORY ${vcpkgPath}
    RESULT_VARIABLE VCPKG_BOOTSTRAP_RESULT)
  if(NOT VCPKG_BOOTSTRAP_RESULT EQUAL "0")
    message(
      FATAL_ERROR
        "${vcpkgPath}/bootstrap-vcpkg failed with ${VCPKG_BOOTSTRAP_RESULT}, please check vcpkg"
    )
  endif()
  # install dependencies vcpkg
  set(vcpkg_install_args
      "install;zstd;lz4;tomlplusplus;fmt;stb;doctest;--triplet;${triplet}")
  execute_process(
    COMMAND vcpkg ${vcpkg_install_args}
    WORKING_DIRECTORY ${vcpkgPath}
    RESULT_VARIABLE VCPKG_INSTALL_RESULT)
  if(NOT VCPKG_INSTALL_RESULT EQUAL "0")
    message(
      FATAL_ERROR
        "${vcpkgPath}/vcpkg.exe install failed with ${VCPKG_INSTALL_RESULT}, please check vcpkg"
    )
  endif()
  set(vcpkgPath
      ${vcpkgPath}
      PARENT_SCOPE)
  set(vcpkgTriplet
      ${triplet}
      PARENT_SCOPE)
endfunction(bb_buildVCPKG)
