#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#

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
