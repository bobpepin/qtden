CONFIG += debug
CONFIG += qt 

#DEFINES = __STDC__ QWT_DLL
DEFINES = __STDC__ 

HEADERS = dendrogram.h vectorplot.h vectorplotwidget.h umatrixplot.h 
HEADERS += heatmapplot.h heatmapwidget.h stat.h dataset.h metabdata.h
HEADERS += param_window.h datamodels.h metabanalysis.h metabresultwidget.h
HEADERS += metabcsv.h umatrixwidget.h resulttablewidget.h

SOURCES = qtden.cc dendrogram.cc vectorplot.cc vectorplotwidget.cc 
SOURCES += umatrixplot.cc heatmapplot.cc heatmapwidget.cc
SOURCES += lrn.cc stat.cc dataset.cc metabdata.cc
SOURCES += param_window.cc datamodels.cc metabanalysis.cc metabresultwidget.cc
SOURCES += metabcsv.cc umatrixwidget.cc resulttablewidget.cc

RESOURCES = qtden.qrc

INCLUDEPATH += ../libesom-1.0/ ../qwt-5.2/src
LIBS += -L$$OUT_PWD/../libesom-1.0/debug -lesom -L$$OUT_PWD/../cprob/debug -lcprob -L$$OUT_PWD/../qwt-5.2/lib -lqwtd -lWinmm
#LIBS += -lcprob -L$$OUT_PWD/../cprob/debug -L$$OUT_PWD/../qwt-5.2/lib -lqwtd5

