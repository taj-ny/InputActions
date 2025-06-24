find_package(KF6 REQUIRED COMPONENTS
    CoreAddons
    I18n
    KCMUtils
)

set(inputactions_kwin_kcm_SRCS
    kcm.cpp
    kcm.h
    kcm.qrc
)

ki18n_wrap_ui(inputactions_kwin_kcm_SRCS kcm.ui)

add_library(inputactions_kwin_kcm MODULE ${inputactions_kwin_kcm_SRCS})
target_link_libraries(inputactions_kwin_kcm
    KF6::KCMUtils
    KF6::CoreAddons
    KF6::I18n
    Qt6::DBus
)

install(TARGETS inputactions_kwin_kcm DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/effects/configs)