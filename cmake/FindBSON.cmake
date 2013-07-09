#
# BSON
# FROM https://github.com/mongodb/mongo-c-driver
#

FIND_PATH(BSON_INCLUDE_DIR bson.h ${BSON_ROOT}/src /usr/local/include /usr/include)

if(WIN64)
  FIND_LIBRARY(BSON_LIBRARY bson ${BSON_ROOT}/lib/x64 ${BSON_ROOT})
else(WIN64)  
  FIND_LIBRARY(BSON_LIBRARY bson ${BSON_ROOT} /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64)
endif(WIN64)

# Report the found libraries, quit with fatal error if any required library has not been found.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(BSON DEFAULT_MSG BSON_LIBRARY BSON_INCLUDE_DIR)
