TEMPLATE = app

QT += qml quick widgets

SOURCES += main.cpp \
    costs_nb_qml_proxy.cpp \
    costs_nb_core.cpp \
    jsoncpp.cpp \
    utils.cpp \
    categories_to_json_converter.cpp \
    categories_to_backend.cpp \
    restclient.cpp \
    miniz_wrp.cpp \
    miniz.cpp \
    base64.cpp

RESOURCES += qml.qrc

LIBS += -L $$PWD/libcurl/lib/armeabi-v7a -lcurl

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    costs_nb_qml_proxy.h \
    costs_nb_core.h \
    json/json.h \
    json/json-forwards.h \
    utils.h \
    categories_to_json_converter.h \
    categories_to_backend.h \
    restclient-cpp/restclient.h \
    restclient-cpp/meta.h \
    miniz_wrp.h \
    base64.h \
    miniz.h

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
