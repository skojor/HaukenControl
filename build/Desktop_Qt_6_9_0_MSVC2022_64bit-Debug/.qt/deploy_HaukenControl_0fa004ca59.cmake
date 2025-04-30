include("C:/Kode/kode/kode/kode/kode/HaukenControl/build/Desktop_Qt_6_9_0_MSVC2022_64bit-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/HaukenControl-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase;qtwebsockets")

qt6_deploy_runtime_dependencies(
    EXECUTABLE C:/Kode/kode/kode/kode/kode/HaukenControl/build/Desktop_Qt_6_9_0_MSVC2022_64bit-Debug/HaukenControl.exe
    GENERATE_QT_CONF
)
