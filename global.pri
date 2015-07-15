#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#

# This file is included by all the other project files and is where
# options or configurations that affect all of the projects can be placed.

OPENRPT_HEADERS = $$(OPENRPT_HEADERS)
! isEmpty( OPENRPT_HEADERS ) {
    OPENRPT_DIR = $$OPENRPT_HEADERS
} else {
    exists(openrpt)    { OPENRPT_DIR = openrpt }
    exists(../openrpt) { OPENRPT_DIR = ../openrpt }
    OPENRPT_DIR_REL=true
}

OPENRPT_BLD = $${OPENRPT_DIR}
exists($${OPENRPT_DIR}-build-desktop) {
  OPENRPT_BLD = $${OPENRPT_DIR}-build-desktop
}

OPENRPT_LIBDIR = $$(OPENRPT_LIBDIR)
isEmpty( OPENRPT_LIBDIR ) {
  OPENRPT_LIBDIR = $${OPENRPT_BLD}/lib
  OPENRPT_LIBDIR_REL=true
}

! exists($${OPENRPT_DIR}) {
  error("Could not set the OPENRPT_DIR qmake variable.")
}

! exists($${OPENRPT_LIBDIR}) {
  error("Could not set the OPENRPT_LIBDIR qmake variable.")
}

CSVIMP_HEADERS = $$(CSVIMP_HEADERS)
isEmpty( CSVIMP_HEADERS ) {
  exists(csvimp)    { CSVIMP_DIR = csvimp    }
  exists(../csvimp) { CSVIMP_DIR = ../csvimp }
  exists(../../csvimp) { CSVIMP_DIR = ../../csvimp }
  CSVIMP_HEADERS = $$CSVIMP_DIR/csvimpcommon
  CSVIMP_HEADERS_REL = true
}

! exists($${CSVIMP_HEADERS}) {
  error("Could not set the CSVIMP_HEADERS qmake variable.")
}

DMTXLIB = -ldmtx

exists($${OPENRPT_LIBDIR}/libdmtx.a)           { DMTXLIB = -ldmtx }
exists($${OPENRPT_LIBDIR}/libDmtx_Library.a)   { DMTXLIB = -lDmtx_Library }
exists($${OPENRPT_LIBDIR}/libdmtx.lib)         { DMTXLIB = -ldmtx }
exists($${OPENRPT_LIBDIR}/libDmtx_Library.lib) { DMTXLIB = -lDmtx_Library }

# global.pri is processed at the top level but the variables are used down 1 level
! isEmpty( OPENRPT_DIR_REL    ) { OPENRPT_DIR    = ../$${OPENRPT_DIR}
                                  OPENRPT_BLD    = ../$${OPENRPT_BLD}    }
! isEmpty( OPENRPT_LIBDIR_REL ) { OPENRPT_LIBDIR = ../$${OPENRPT_LIBDIR} }
! isEmpty( CSVIMP_HEADERS_REL ) { CSVIMP_HEADERS = ../$${CSVIMP_HEADERS} }

INCLUDEPATH += $${OPENRPT_DIR}/common           $${OPENRPT_BLD}/common \
	       $${OPENRPT_DIR}/OpenRPT/renderer $${OPENRPT_BLD}/OpenRPT/renderer \
	       $${OPENRPT_DIR}/OpenRPT/wrtembed $${OPENRPT_BLD}/OpenRPT/wrtembed \
	       $${OPENRPT_DIR}/MetaSQL          $${OPENRPT_BLD}/MetaSQL \
	       $${OPENRPT_DIR}/MetaSQL/tmp      $${OPENRPT_BLD}/MetaSQL/tmp \
	       $${CSVIMP_HEADERS}
INCLUDEPATH =  $$unique(INCLUDEPATH)

XTUPLE_DIR=..
XTUPLE_BLD=$${XTUPLE_DIR}
exists(../qt-client-build-desktop) {
  XTUPLE_BLD=../../qt-client-build-desktop
}

DEPENDPATH  += $${INCLUDEPATH}

CONFIG += release thread

# The packaged version of OpenRPT installs the images to
# /usr/share/openrpt/OpenRPT/images
# Set this variable in the environment to use that location
OPENRPT_IMAGE_DIR = $$(OPENRPT_IMAGE_DIR)
isEmpty( OPENRPT_IMAGE_DIR ) {
  exists("/usr/share/openrpt") {
    OPENRPT_IMAGE_DIR = /usr/share/openrpt/OpenRPT/images
  } else {
    OPENRPT_IMAGE_DIR = $${OPENRPT_DIR}/OpenRPT/images
  }
}

OPENRPTLIBEXT = $${QMAKE_EXTENSION_SHLIB}
XTLIBEXT      = $${QMAKE_EXTENSION_SHLIB}
win32:OPENRPTLIBEXT = a
win32:XTLIBEXT      = a
macx:OPENRPTLIBEXT  = a
macx:XTLIBEXT       = a
isEmpty( OPENRPTLIBEXT ) {
  unix:OPENRPTLIBEXT  = so
}
isEmpty( XTLIBEXT ) {
  unix:XTLIBEXT  = so
}

macx:exists(macx.pri) {
  include(macx.pri)
}

win32:exists(win32.pri) {
  include(win32.pri)
}

unix:exists(unix.pri) {
  include(unix.pri)
}
