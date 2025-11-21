install(
    TARGETS r-type_exe
    RUNTIME COMPONENT r-type_Runtime
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
