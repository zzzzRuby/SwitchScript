if (RUPKG_PLATFORM_AVR)
    set(RUPKG_REQUIRED_PACKAGES lufa)
else()
    set(RUPKG_REQUIRED_PACKAGES Qt tinyxml2 opencv)
endif()

set(RUPKG_SUPPORT_PLATFORMS windows linux macos avr)