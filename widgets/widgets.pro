include( ../global.pri )
TARGET = xtuplewidgets
TEMPLATE = lib
CONFIG += qt \
          warn_on \
          plugin

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets printsupport sql script designer uitools designer
} else {
  CONFIG += designer uitools
}

# INCLUDEPATH += $$QT_SOURCE_TREE/tools/designer/interfaces ../common .
INCLUDEPATH += ../common \
               . #current directory

DBFILE = widgets.db
LANGUAGE = C++
DEPENDPATH += ../common
dynamic { 
    CONFIG += dll # this is technically redundant as plugin implies dll however it fixes a cross-compile problem
    DESTDIR = $$[QT_INSTALL_PLUGINS]/designer
    QMAKE_LIBDIR = ../lib $$OPENRPT_LIBDIR $$QMAKE_LIBDIR
    LIBS += -lxtuplecommon -lwrtembed -lrenderer -lMetaSQL -lopenrptcommon
    DEFINES += MAKEDLL
    MOC_DIR = tmp/dll
    OBJECTS_DIR = tmp/dll
    UI_DIR = tmp/dll
}
else {
    DESTDIR = ../lib
    CONFIG += staticlib
    MOC_DIR = tmp/lib
    OBJECTS_DIR = tmp/lib
    UI_DIR = tmp/lib
}
HEADERS += plugins/addressclusterplugin.h \
    plugins/alarmsplugin.h \
    plugins/cmheadclusterplugin.h \
    plugins/calendarcomboboxplugin.h \
    plugins/characteristicswidgetplugin.h       \
    plugins/clineeditplugin.h \
    plugins/apopenclusterplugin.h \
    plugins/aropenclusterplugin.h \
    plugins/commentsplugin.h \
    plugins/contactclusterplugin.h \
    plugins/contactwidgetplugin.h \
    plugins/crmacctclusterplugin.h \
    plugins/currclusterplugin.h \
    plugins/currdisplayplugin.h \
    plugins/custclusterplugin.h \
    plugins/customerselectorplugin.h \
    plugins/dateclusterplugin.h \
    plugins/deptclusterplugin.h \
    plugins/dlineeditplugin.h \
    plugins/documentsplugin.h \
    plugins/empclusterplugin.h \
    plugins/empgroupclusterplugin.h \
    plugins/expenseclusterplugin.h \
    plugins/expenselineeditplugin.h \
    plugins/fileclusterplugin.h \
    plugins/filemoveselectorplugin.h \
    plugins/glclusterplugin.h \
    plugins/imageclusterplugin.h \
    plugins/invoiceclusterplugin.h \
    plugins/invoicelineeditplugin.h \
    plugins/incidentclusterplugin.h \
    plugins/itemclusterplugin.h \
    plugins/itemgroupclusterplugin.h \
    plugins/itemlineeditplugin.h \
    plugins/lotserialclusterplugin.h \
    plugins/lotserialseqclusterplugin.h \
    plugins/menubuttonplugin.h \
    plugins/numbergencomboboxplugin.h \
    plugins/orderclusterplugin.h \
    plugins/opportunityclusterplugin.h \
    plugins/parametergroupplugin.h \
    plugins/parameterwidgetplugin.h \
    plugins/periodslistviewplugin.h \
    plugins/planordclusterplugin.h \
    plugins/planordlineeditplugin.h \
    plugins/projectclusterplugin.h \
    plugins/projectlineeditplugin.h \
    plugins/querysetplugin.h \
    plugins/quoteclusterplugin.h \
    plugins/raclusterplugin.h \
    plugins/recurrencewidgetplugin.h \
    plugins/revisionclusterplugin.h \
    plugins/shiftclusterplugin.h \
    plugins/shipmentclusterplugin.h \
    plugins/shiptoclusterplugin.h \
    plugins/shiptoeditplugin.h \
    plugins/usernameclusterplugin.h \
    plugins/usernamelineeditplugin.h \
    plugins/vendorclusterplugin.h \
    plugins/vendorgroupplugin.h \
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
    plugins/xdoccopysetterplugin.h      \
    plugins/xlabelplugin.h \
    plugins/xlineeditplugin.h \
    plugins/xtreewidgetplugin.h \
    plugins/xurllabelplugin.h \
    plugins/xtexteditplugin.h \
    plugins/screenplugin.h \
    plugins/xtreeviewplugin.h \
    plugins/xspinboxplugin.h \
    plugins/xtableviewplugin.h \

SOURCES += widgets.cpp \
    addressCluster.cpp \
    alarmMaint.cpp \
    alarms.cpp \
    apopencluster.cpp \
    aropencluster.cpp \
    calendarTools.cpp \
    characteristicAssignment.cpp        \
    characteristicswidget.cpp           \
    cmheadcluster.cpp \
    comment.cpp \
    comments.cpp \
    contactCluster.cpp \
    contactemail.cpp \
    contactWidget.cpp \
    crmacctCluster.cpp \
    crmCluster.cpp \
    currCluster.cpp \
    custCluster.cpp \
    customerselector.cpp \
    datecluster.cpp \
    deptCluster.cpp \
    docAttach.cpp \
    documents.cpp \
    editwatermark.cpp   \
    empcluster.cpp \
    empgroupcluster.cpp \
    expensecluster.cpp \
    filecluster.cpp \
    filemoveselector.cpp \
    filterManager.cpp \
    filterSave.cpp \
    glCluster.cpp \
    imageAssignment.cpp \
    imagecluster.cpp \
    imageview.cpp \
    incidentCluster.cpp \
    invoiceCluster.cpp \
    invoiceLineEdit.cpp \
    itemAliasList.cpp \
    itemCluster.cpp \
    itemgroupcluster.cpp \
    lotserialCluster.cpp \
    lotserialseqcluster.cpp \
    menubutton.cpp \
    numbergencombobox.cpp \
    opportunitycluster.cpp \
    ordercluster.cpp \
    parametergroup.cpp \
    parameterwidget.cpp \
    plCluster.cpp \
    projectCluster.cpp \
    projectCopy.cpp \
    queryitem.cpp \
    queryset.cpp \
    quoteList.cpp \
    quotecluster.cpp \
    racluster.cpp \
    recurrencewidget.cpp \
    revisionCluster.cpp \
    salesOrderList.cpp \
    screen.cpp \
    shiftCluster.cpp \
    shipmentCluster.cpp \
    shiptoCluster.cpp \
    transferOrderList.cpp \
    usernameCluster.cpp \
    vendorcluster.cpp \
    vendorgroup.cpp \
    virtualCluster.cpp \
    warehouseCluster.cpp \
    warehousegroup.cpp \
    woCluster.cpp \
    workcenterCluster.cpp \
    xcheckbox.cpp \
    xcombobox.cpp \
    xdatawidgetmapper.cpp \
    xdoccopysetter.cpp \
    xdoublevalidator.cpp \
    xitemdelegate.cpp \
    xlabel.cpp \
    xlineedit.cpp \
    xlistbox.cpp \
    xspinbox.cpp \
    xsqltablemodel.cpp \
    xtableview.cpp \
    xtextedit.cpp \
    xtreeview.cpp \
    xtreewidget.cpp \
    xtreewidgetprogress.cpp \
    xurllabel.cpp \

HEADERS += widgets.h \
    xtupleplugin.h \
    guiclientinterface.h \
    addresscluster.h \
    alarmMaint.h \
    alarms.h \
    apopencluster.h \
    aropencluster.h \
    calendarTools.h \
    characteristicAssignment.h  \
    characteristicswidget.h     \
    cmheadcluster.h \
    comment.h \
    comments.h \
    contactcluster.h \
    contactemail.h \
    contactwidget.h \
    crmacctcluster.h \
    crmcluster.h \
    currcluster.h \
    custcluster.h \
    customerselector.h \
    datecluster.h \
    dcalendarpopup.h \
    deptcluster.h \
    docAttach.h \
    documents.h \
    editwatermark.h     \
    empcluster.h \
    empgroupcluster.h \
    expensecluster.h \
    filecluster.h \
    filemoveselector.h \
    filterManager.h \
    filtersave.h \
    glcluster.h \
    imageAssignment.h \
    imagecluster.h \
    imageview.h \
    incidentcluster.h \
    invoicecluster.h \
    invoicelineedit.h \
    itemAliasList.h \
    itemcluster.h \
    itemgroupcluster.h \
    lotserialCluster.h \
    lotserialseqcluster.h \
    menubutton.h        \
    numbergencombobox.h \
    opportunitycluster.h \
    ordercluster.h \
    parametergroup.h \
    parameterwidget.h \
    plCluster.h \
    projectcluster.h \
    projectCopy.h \
    queryitem.h \
    queryset.h \
    quoteList.h \
    quotecluster.h \
    racluster.h \
    recurrencewidget.h \
    revisioncluster.h \
    salesOrderList.h \
    screen.h \
    shiftcluster.h \
    shipmentcluster.h \
    shiptocluster.h \
    transferOrderList.h \
    usernamecluster.h \
    vendorcluster.h \
    vendorgroup.h \
    virtualCluster.h \
    warehouseCluster.h \
    warehousegroup.h \
    woCluster.h \
    workcentercluster.h \
    xcheckbox.h \
    xcombobox.h \
    xcomboboxprivate.h \
    xdatawidgetmapper.h \
    xdoccopysetter.h \
    xdoublevalidator.h \
    xitemdelegate.h \
    xlabel.h \
    xlineedit.h \
    xlistbox.h \
    xspinbox.h \
    xsqltablemodel.h \
    xtableview.h \
    xtextedit.h \
    xtreeview.h \
    xtreewidget.h \
    xtreewidgetprogress.h \
    xurllabel.h \

FORMS += alarmMaint.ui \
    alarms.ui \
    characteristicAssignment.ui         \
    characteristicswidget.ui            \
    comment.ui                          \
    contactemail.ui \
    customerselector.ui \
    docAttach.ui \
    documents.ui \
    editwatermark.ui    \
    filemoveselector.ui \
    filterManager.ui \
    filterSave.ui \
    imageAssignment.ui \
    imageview.ui \
    menubutton.ui \
    parameterwidget.ui \
    queryitem.ui \
    queryset.ui \
    recurrencewidget.ui \
    vendorgroup.ui      \
    womatlcluster.ui    \
    xdoccopysetter.ui

RESOURCES += widgets.qrc

QT += sql script
