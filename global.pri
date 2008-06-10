#
# This file is included by all the other project files
# and is where options or configurations that affect all
# of the projects can be place.
#

#
# This is the relative directory path to the openrpt project.
#
exists(../../../openrpt) {
    OPENRPT_DIR = ../../../openrpt
}
exists(../../openrpt) {
    OPENRPT_DIR = ../../openrpt
}
exists(../openrpt) {
    OPENRPT_DIR = ../openrpt
}

! exists($${OPENRPT_DIR}) {
    error("Could not set the OPENRPT_DIR qmake variable.")
}

INCLUDEPATH += ../$${OPENRPT_DIR}/common ../$${OPENRPT_DIR}/OpenRPT/renderer ../$${OPENRPT_DIR}/OpenRPT/wrtembed ../$${OPENRPT_DIR}/MetaSQL


CONFIG += release thread
#CONFIG += debug

QT += qt3support
macx {
  CONFIG += x86 ppc
}
