include( ../global.pri )

TARGET      = xtuplewidgets

TEMPLATE    =  lib
CONFIG      += qt warn_on designer plugin
#INCLUDEPATH += $$QT_SOURCE_TREE/tools/designer/interfaces ../common .
INCLUDEPATH += ../common .
DBFILE      =  widgets.db
LANGUAGE    =  C++

DEPENDPATH += ../common

dynamic {
  CONFIG += dll # this is technically redundant as plugin implies dll however it fixes a cross-compile problem
  DESTDIR = $$[QT_INSTALL_PLUGINS]/designer

  LIBS += -L../lib -L../$$OPENRPT_DIR/lib -lxtuplecommon -lwrtembed -lrenderer -lcommon
  DEFINES += MAKEDLL

  MOC_DIR = tmp/dll
  OBJECTS_DIR = tmp/dll
  UI_DIR = tmp/dll
} else {
  DESTDIR = ../lib
  CONFIG += staticlib

  MOC_DIR = tmp/lib
  OBJECTS_DIR = tmp/lib
  UI_DIR = tmp/lib
}

HEADERS += \
           plugins/addressclusterplugin.h \
           plugins/alarmsplugin.h \
           plugins/calendarcomboboxplugin.h \
           plugins/clineeditplugin.h \
           plugins/cmclusterplugin.h \
           plugins/commentsplugin.h \
           plugins/contactclusterplugin.h \
           plugins/crmacctclusterplugin.h \
           plugins/currclusterplugin.h \
           plugins/currdisplayplugin.h \
           plugins/custclusterplugin.h \
           plugins/custinfoplugin.h \
           plugins/dateclusterplugin.h \
           plugins/deptclusterplugin.h \
           plugins/dlineeditplugin.h \
           plugins/documentsplugin.h \
           plugins/empclusterplugin.h \
           plugins/empgroupclusterplugin.h \
           plugins/expenseclusterplugin.h \
           plugins/expenselineeditplugin.h \
           plugins/fileclusterplugin.h \
           plugins/glclusterplugin.h \
           plugins/imageclusterplugin.h \
           plugins/invoiceclusterplugin.h \
           plugins/invoicelineeditplugin.h \
           plugins/incidentclusterplugin.h \
           plugins/itemclusterplugin.h \
           plugins/itemlineeditplugin.h \
           plugins/lotserialclusterplugin.h \
           plugins/orderclusterplugin.h \
           plugins/opportunityclusterplugin.h \
           plugins/parametergroupplugin.h \
           plugins/periodslistviewplugin.h \
           plugins/planordclusterplugin.h \
           plugins/planordlineeditplugin.h \
           plugins/poclusterplugin.h \
           plugins/polineeditplugin.h \
           plugins/projectclusterplugin.h \
           plugins/projectlineeditplugin.h \
           plugins/raclusterplugin.h\
           plugins/revisionclusterplugin.h\
           plugins/shiftclusterplugin.h \
           plugins/shipmentclusterplugin.h \
           plugins/shiptoclusterplugin.h \
           plugins/shiptoeditplugin.h \
           plugins/soclusterplugin.h \
           plugins/solineeditplugin.h \
           plugins/toclusterplugin.h \
           plugins/usernameclusterplugin.h \
           plugins/usernamelineeditplugin.h \
           plugins/vendorclusterplugin.h \
           plugins/vendorgroupplugin.h \
           plugins/vendorinfoplugin.h \
           plugins/vendorlineeditplugin.h \
           plugins/warehousegroupplugin.h \
           plugins/wcomboboxplugin.h \
           plugins/woclusterplugin.h \
           plugins/wolineeditplugin.h \
           plugins/womatlclusterplugin.h \
           plugins/workcenterclusterplugin.h \
           plugins/workcenterlineeditplugin.h \
           plugins/xcheckboxplugin.h \
           plugins/xcomboboxplugin.h \
           plugins/xlabelplugin.h \
           plugins/xlineeditplugin.h \
           plugins/xtreewidgetplugin.h \
           plugins/xurllabelplugin.h \
           plugins/xtexteditplugin.h \
	   plugins/screenplugin.h\
           plugins/xtreeviewplugin.h \
           plugins/xspinboxplugin.h \

SOURCES    += widgets.cpp \
              addressCluster.cpp contactCluster.cpp crmacctCluster.cpp \
              xlabel.cpp xlineedit.cpp xcheckbox.cpp xcombobox.cpp \
              xlistbox.cpp \
              cmCluster.cpp \
              custCluster.cpp \
              itemCluster.cpp itemList.cpp itemSearch.cpp itemAliasList.cpp \
              warehouseCluster.cpp warehousegroup.cpp \
              woCluster.cpp woList.cpp \
              filecluster.cpp glCluster.cpp accountList.cpp accountSearch.cpp \
              imagecluster.cpp \
              invoiceLineEdit.cpp incidentCluster.cpp \
              ordercluster.cpp \
              opportunitycluster.cpp \
              poCluster.cpp purchaseOrderList.cpp \
              plCluster.cpp plannedOrderList.cpp \
              vendorcluster.cpp \
              vendorgroup.cpp \
              soCluster.cpp salesOrderList.cpp \
              shiptoCluster.cpp shipToList.cpp \
              toCluster.cpp \
              transferOrderList.cpp \
              calendarTools.cpp \
              parametergroup.cpp \
              comment.cpp comments.cpp \
              xurllabel.cpp \
              currCluster.cpp usernameCluster.cpp usernameList.cpp \
              workcenterCluster.cpp \
              projectCluster.cpp \
              expensecluster.cpp \
              datecluster.cpp \
              virtualCluster.cpp deptCluster.cpp shiftCluster.cpp \
              xtreewidget.cpp \
              lotserialCluster.cpp \
              shipmentCluster.cpp \
              racluster.cpp \
              revisionCluster.cpp \
	      xdatawidgetmapper.cpp \
              xtextedit.cpp \
              empcluster.cpp empgroupcluster.cpp \
	      xsqltablemodel.cpp \
              xtreeview.cpp \
	      screen.cpp \
              documents.cpp \
              imageview.cpp \
              imageAssignment.cpp \
              file.cpp \
              alarms.cpp alarmMaint.cpp \
              invoiceCluster.cpp \
              xspinbox.cpp \

HEADERS    += widgets.h \
              addresscluster.h contactcluster.h crmacctcluster.h \
              xlabel.h xlineedit.h xcheckbox.h xcombobox.h \
              xlistbox.h \
              cmcluster.h \
              custcluster.h \
              itemcluster.h itemList.h itemSearch.h itemAliasList.h \
              warehouseCluster.h warehousegroup.h \
              woCluster.h woList.h \
              filecluster.h glcluster.h accountList.h accountSearch.h \
              imagecluster.h \
              invoicelineedit.h incidentcluster.h \
              ordercluster.h \
              opportunitycluster.h \
              pocluster.h purchaseOrderList.h \
              plCluster.h plannedOrderList.h \
              vendorcluster.h \
              vendorgroup.h \
              socluster.h salesOrderList.h \
              shiptocluster.h shipToList.h \
              tocluster.h \
              transferOrderList.h \
              calendarTools.h \
              parametergroup.h \
              comment.h comments.h \
              xurllabel.h \
              currcluster.h usernamecluster.h usernameList.h \
              workcentercluster.h \
              projectcluster.h \
              expensecluster.h \
              datecluster.h \
              virtualCluster.h deptcluster.h shiftcluster.h \
              xtreewidget.h \
              lotserialCluster.h \
              shipmentcluster.h \
              racluster.h \
              revisioncluster.h \
              xdatawidgetmapper.h \
              xtextedit.h \
              dcalendarpopup.h\
              empcluster.h empgroupcluster.h\
              xsqltablemodel.h \
              xtreeview.h \
              screen.h \
              documents.h \
              imageview.h \
              imageAssignment.h \
              file.h \
              alarms.h alarmMaint.h \
              invoicecluster.h \
              xspinbox.h \

FORMS += accountSearch.ui \
         documents.ui \
         imageview.ui \
         imageAssignment.ui \
         file.ui \
         alarms.ui alarmMaint.ui \
         vendorgroup.ui \
         womatlcluster.ui


RESOURCES += widgets.qrc

#The following line was inserted by qt3to4
QT +=  sql
#QT += qt3support 
