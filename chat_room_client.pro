QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 console link_pkgconfig


# 设置 .proto 文件路径
PROTOS = $$PWD/chat.proto

# 创建 protobuf headers 编译器
protobuf_decl.name = protobuf headers
protobuf_decl.input = PROTOS
protobuf_decl.output = ${QMAKE_FILE_IN_BASE}.pb.h
protobuf_decl.commands = /usr/local/bin/protoc --cpp_out=. ${QMAKE_FILE_IN} -I${QMAKE_FILE_PATH}
protobuf_decl.variable_out = HEADERS
QMAKE_EXTRA_COMPILERS += protobuf_decl

# 创建 protobuf sources 编译器
protobuf_impl.name = protobuf sources
protobuf_impl.input = PROTOS
protobuf_impl.output = ${QMAKE_FILE_IN_BASE}.pb.cc
protobuf_impl.depends = ${QMAKE_FILE_IN_BASE}.pb.h
protobuf_impl.commands = $$escape_expand(\\n)
protobuf_impl.variable_out = SOURCES
QMAKE_EXTRA_COMPILERS += protobuf_impl

INCLUDEPATH += /usr/local/include

LIBS += -L/usr/local/lib -lprotobuf -lmuduo_net -lmuduo_base -lpthread -lz

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += MUDUO_STD_STRING

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ACAutomaton.cc \
    addfrienddialog.cpp \
    chatclient.cpp \
    codec.cc \
    logindlg.cpp \
    main.cpp \
    registdlg.cpp \
    widget.cpp

HEADERS += \
    ACAutomaton.h \
    addfrienddialog.h \
    chatclient.h \
    codec.h \
    common.h \
    dispatcher.h \
    logindlg.h \
    registdlg.h \
    widget.h

FORMS += \
    addfrienddialog.ui \
    logindlg.ui \
    registdlg.ui \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    src.qrc

RC_ICONS = wechat.ico

DISTFILES += \
    chat.proto

