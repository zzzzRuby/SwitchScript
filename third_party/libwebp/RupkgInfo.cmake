set(RUPKG_EXPORT_TARGETS webp webpdemux webpmux)
if (RUPKG_PLATFORM_ANDROID)
    list(APPEND RUPKG_REQUIRED_PACKAGES cpufeatures)
endif()