if(NOT GUI_ZIP_URL)
  set(GUI_ZIP_URL "https://oxen.rocks/oxen-io/lokinet-gui/dev/lokinet-windows-x64-20220331T180338Z-569f90ad8.zip")
  set(GUI_ZIP_HASH_OPTS EXPECTED_HASH SHA256=316f10489f5907bfa9c74b21f8ef2fdd7b7c7e6a0f5bcedaed2ee5f4004eab52)
endif()

set(TUNTAP_URL "https://build.openvpn.net/downloads/releases/latest/tap-windows-latest-stable.exe")
set(TUNTAP_EXE "${CMAKE_BINARY_DIR}/tuntap-install.exe")
set(BOOTSTRAP_FILE "${PROJECT_SOURCE_DIR}/contrib/bootstrap/mainnet.signed")

file(DOWNLOAD
    ${TUNTAP_URL}
    ${TUNTAP_EXE})

file(DOWNLOAD
    ${GUI_ZIP_URL}
    ${CMAKE_BINARY_DIR}/lokinet-gui.zip
    ${GUI_ZIP_HASH_OPTS})

execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${CMAKE_BINARY_DIR}/lokinet-gui.zip
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

install(DIRECTORY ${CMAKE_BINARY_DIR}/gui DESTINATION share COMPONENT gui)
install(PROGRAMS ${TUNTAP_EXE} DESTINATION bin COMPONENT tuntap)
install(FILES ${BOOTSTRAP_FILE} DESTINATION share COMPONENT lokinet RENAME bootstrap.signed)

set(CPACK_PACKAGE_INSTALL_DIRECTORY "Lokinet")
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/win32-setup/lokinet.ico")
set(CPACK_NSIS_DEFINES "RequestExecutionLevel admin")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)


function(read_nsis_file filename outvar)
  file(STRINGS "${filename}" _outvar)
  list(TRANSFORM _outvar REPLACE "\\\\" "\\\\\\\\")
  list(JOIN _outvar "\\n" out)
  set(${outvar} ${out} PARENT_SCOPE)
endfunction()

read_nsis_file("${CMAKE_SOURCE_DIR}/win32-setup/extra_preinstall.nsis" _extra_preinstall)
read_nsis_file("${CMAKE_SOURCE_DIR}/win32-setup/extra_install.nsis" _extra_install)
read_nsis_file("${CMAKE_SOURCE_DIR}/win32-setup/extra_uninstall.nsis" _extra_uninstall)
read_nsis_file("${CMAKE_SOURCE_DIR}/win32-setup/extra_create_icons.nsis" _extra_create_icons)
read_nsis_file("${CMAKE_SOURCE_DIR}/win32-setup/extra_delete_icons.nsis" _extra_delete_icons)

set(CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS "${_extra_preinstall}")
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${_extra_install}")
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS  "${_extra_uninstall}")
set(CPACK_NSIS_CREATE_ICONS_EXTRA "${_extra_create_icons}")
set(CPACK_NSIS_DELETE_ICONS_EXTRA "${_extra_delete_icons}")

get_cmake_property(CPACK_COMPONENTS_ALL COMPONENTS)
list(REMOVE_ITEM CPACK_COMPONENTS_ALL "Unspecified")
