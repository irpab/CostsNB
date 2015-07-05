TEMPLATE = app

QT += qml quick widgets

SOURCES += main.cpp \
    costs_nb_qml_proxy.cpp \
    costs_nb_core.cpp \
    jsoncpp.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    costs_nb_qml_proxy.h \
    costs_nb_core.h \
    json/json.h \
    json/json-forwards.h

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
