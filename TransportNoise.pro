QT       += core gui widgets printsupport # <-- Добавили printsupport!
# Модуль printsupport нужен для функций QPrinter внутри QCustomPlot

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

SOURCES += \
    ../parcer/Transport-noise/src/parser.cpp \
    main.cpp \
    mainwindow.cpp

# УДАЛИТЕ СТРОКУ НИЖЕ:
# $$PWD/libs/qcustomplot/qcustomplot.cpp
# Мы больше не компилируем исходники библиотеки вручную

HEADERS += \
    ../parcer/Transport-noise/src/parser.h \
    mainwindow.h

# УДАЛИТЕ СТРОКУ НИЖЕ:
# $$PWD/libs/qcustomplot/qcustomplot.h
# Заголовки подключаются через INCLUDEPATH

FORMS += \
    mainwindow.ui

# --- НАСТРОЙКА ПОДКЛЮЧЕНИЯ ГОТОВОЙ БИБЛИОТЕКИ ---
INCLUDEPATH += $$PWD/libs/qcustomplot # Путь к .h файлам

# Указываем путь к папке с .a файлом.
# Если вы собираете проект в режиме Debug, используйте 'debug', если Release - 'release'.
unix:LIBS += -L$$PWD/libs/qcustomplot/build/Desktop-Debug -lqcustomplot

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
