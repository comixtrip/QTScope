# File generated by kdevelop's qmake manager. 
# ------------------------------------------- 
# Subdir relative project main directory: ./plugins/xyplot
# Target is a library:  

SOURCES += xyplot.cpp 
HEADERS += xyplot.h 
TEMPLATE = lib 
CONFIG += release \
          warn_on \
          qt \
          thread \
          plugin 
INCLUDEPATH = ../../plugins/xyplot \
              ../../src \
              /usr/include/qwt
LIBS += -lqwt \
        ../../src/moc_scopeplotplugin.o \
        ../../src/scopeplotplugin.o 
TARGETDEPS += ../../src/../bin/qtscope 
target.path = /usr/local/lib/qtscope/plugins 
INSTALLS += target 
#The following line was inserted by qt3to4
QT +=  qt3support 
#The following line was inserted by qt3to4
QT +=  
#The following line was inserted by qt3to4
QT +=  