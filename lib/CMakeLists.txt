# Function to download resource from git repo
function(FIND_AND_DOWNLOAD_GIT_RESOURCE ResourceName ResourceURL)
   if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${ResourceName})
      message("${ResourceName} not found!")
      execute_process(COMMAND git clone --recursive ${ResourceURL} ${CMAKE_CURRENT_SOURCE_DIR}/${ResourceName})
   else()
      message("${ResourceName} found!")
   endif()
endfunction()

add_subdirectory(stb_image)