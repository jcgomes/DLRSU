#-------------------------------------------------
#
# Project created by QtCreator 2016-01-04T00:53:46
#
#-------------------------------------------------

QT += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DLRSU
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    lixoes.cpp \
    municipios.cpp \
    municipios_cad.cpp \
    niveis_impacto.cpp \
    sobre.cpp \
    valor_situacao.cpp \
    lixoes_cad.cpp \
    lixoes_cen.cpp

VERSION = 0.1

HEADERS += mainwindow.h \
    globais.h \
    lixoes.h \
    municipios.h \
    municipios_cad.h \
    niveis_impacto.h \
    sobre.h \
    valor_situacao.h \
    lixoes_cad.h \
    lixoes_cen.h

FORMS += mainwindow.ui \
    lixoes.ui \
    municipios.ui \
    municipios_cad.ui \
    niveis_impacto.ui \
    sobre.ui \
    valor_situacao.ui \
    lixoes_cad.ui \
    lixoes_cen.ui

RESOURCES = \
    resources.qrc

#RC_FILE = resources.rc
