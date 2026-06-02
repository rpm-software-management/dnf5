if (WITH_SYSTEMD)
  add_subdirectory("systemd")
  add_subdirectory("tmpfiles.d")
endif()
