include( ../global.pri )

TARGET   = xtuple
CONFIG   += qt warn_on assistant uitools designer
TEMPLATE = app

INCLUDEPATH += ../common ../widgets ../widgets/tmp/lib .
DEPENDPATH  += ../common ../widgets ../widgets/tmp/lib
LIBS        += -L../lib -L../$${OPENRPT_DIR}/lib -lxtuplecommon -lxtuplewidgets -lwrtembed -lcommon -lrenderer -lQtDesignerComponents
#LIBS        += -L../../payflowpro/win32/lib -lpfpro

win32 {
  #LIBS += -lshell32
  RC_FILE = rcguiclient.rc
  OBJECTS_DIR = win_obj
}

unix {
  OBJECTS_DIR = unx_obj
}

macx {
  RC_FILE = images/icons.icns
  #PRECOMPILED_HEADER = stable.h
  OBJECTS_DIR = osx_obj
  QMAKE_INFO_PLIST = Info.plist
}

DESTDIR     = ../bin
MOC_DIR     = moc
UI_DIR      = ui

FORMS        = absoluteCalendarItem.ui accountNumber.ui accountNumbers.ui                             \
               accountingPeriod.ui accountingPeriods.ui accountingYearPeriod.ui                       \
               accountingYearPeriods.ui addPoComment.ui                                               \
               address.ui addresses.ui                                                                \
               adjustmentTrans.ui assignClassCodeToPlannerCode.ui assignItemToPlannerCode.ui          \
               allocateReservations.ui                                                                \
               apAccountAssignment.ui apAccountAssignments.ui                                         \
               applyAPCreditMemo.ui apOpenItem.ui apCreditMemoApplication.ui                          \
               applyDiscount.ui                                                                       \
               arAccountAssignment.ui arAccountAssignments.ui                                         \
               applyARCreditMemo.ui arOpenItem.ui arCreditMemoApplication.ui                          \
               archRestoreSalesHistory.ui arWorkBench.ui apWorkBench.ui                               \
               assignLotSerial.ui                                                                     \
               ../common/batchManager.ui ../common/batchItem.ui                                       \
               bankAccounts.ui bankAccount.ui                                                         \
               bankAdjustment.ui bankAdjustmentEditList.ui                                            \
               bankAdjustmentTypes.ui bankAdjustmentType.ui                                           \
               bboms.ui bbomItem.ui bbom.ui                                                           \
               bom.ui bomItem.ui bomList.ui boo.ui booItem.ui booItemList.ui booList.ui buyCard.ui    \
               booitemImage.ui budgets.ui                                                             \
               calendars.ui calendar.ui                                                               \
               cashReceipt.ui cashReceiptsEditList.ui cashReceiptItem.ui cashReceiptMiscDistrib.ui    \
               changePoitemQty.ui changeWoQty.ui changeQtyToDistributeFromBreeder.ui                  \
               characteristic.ui characteristics.ui characteristicAssignment.ui                       \
               characteristicPrice.ui check.ui checkFormat.ui checkFormats.ui                         \
               classCodes.ui classCode.ui closePurchaseOrder.ui closeWo.ui                            \
               commentType.ui commentTypes.ui company.ui companies.ui configureAccountingSystem.ui    \
               configureCC.ui configureSO.ui configureIM.ui                                           \
               configureGL.ui configureMS.ui configurePD.ui configurePO.ui                            \
               configureWO.ui configureBackup.ui configureCRM.ui                                      \
               configureIE.ui configureEncryption.ui                                                  \
               confirmAchOK.ui                                                                        \
               contact.ui contacts.ui copyPurchaseOrder.ui copyBudget.ui                              \
               copyBOM.ui copyBOO.ui copyItem.ui copyPlannedSchedule.ui copySalesOrder.ui             \
               copyTransferOrder.ui                                                                   \
               costCategory.ui costCategories.ui countries.ui country.ui                              \
               countTagList.ui correctOperationsPosting.ui correctProductionPosting.ui                \
               createBufferStatusByItem.ui createBufferStatusByPlannerCode.ui                         \
               createCycleCountTags.ui                                                                \
               createCountTagsByParameterList.ui createCountTagsByItem.ui                             \
               createItemSitesByClassCode.ui                                                          \
               createPlannedOrdersByItem.ui createPlannedOrdersByPlannerCode.ui                       \
               createRecurringInvoices.ui                                                             \
               woMaterialItem.ui createLotSerial.ui creditMemo.ui                                     \
               creditMemoItem.ui creditMemoEditList.ui                                                \
               crmaccount.ui crmaccounts.ui                                                           \
               currencies.ui currency.ui currencyConversion.ui currencyConversions.ui                 \
               currencySelect.ui                                                                      \
               customCommands.ui customCommand.ui customCommandArgument.ui                            \
               customer.ui customerGroups.ui customerGroup.ui creditCard.ui                           \
               customerFormAssignment.ui customerFormAssignments.ui                                   \
               customers.ui customerTypes.ui customerType.ui customerTypeList.ui                      \
               databaseInformation.ui deletePlannedOrder.ui deletePlannedOrdersByPlannerCode.ui       \
               deliverEmail.ui									      \
               deliverInvoice.ui deliverPurchaseOrder.ui deliverSalesOrder.ui                         \
               department.ui departments.ui                                                           \
               distributeBreederProduction.ui                         distributeInventory.ui          \
               distributeToLocation.ui                                                                \
               dspAllocations.ui dspAPApplications.ui                                                 \
               dspAPOpenItemsByVendor.ui dspAROpenItemsByCustomer.ui                                  \
               dspAROpenItems.ui                                                                      \
               dspARApplications.ui dspBacklogByCustomer.ui dspBacklogByParameterList.ui              \
               dspBacklogByItem.ui dspBacklogBySalesOrder.ui                                          \
               dspBankrecHistory.ui                                                                   \
               billingEditList.ui dspBillingSelections.ui                                             \
               dspBookingsByCustomer.ui dspBookingsByCustomerGroup.ui dspBookingsByItem.ui            \
               dspBookingsByProductCategory.ui dspBookingsBySalesRep.ui dspBookingsByShipTo.ui        \
               dspBreederDistributionVarianceByItem.ui dspBreederDistributionVarianceByWarehouse.ui   \
               dspBriefEarnedCommissions.ui                                                           \
               dspBriefSalesHistoryByCustomer.ui dspBriefSalesHistoryByCustomerType.ui                \
               dspBriefSalesHistoryBySalesRep.ui                                                      \
               dspCapacityBufferStatusByWorkCenter.ui                                                 \
               dspCapacityUOMsByClassCode.ui dspCapacityUOMsByProductCategory.ui                      \
               dspCheckRegister.ui dspCashReceipts.ui                                                 \
               dspCostedSingleLevelBOM.ui dspCostedSummarizedBOM.ui dspCostedIndentedBOM.ui           \
               dspCountSlipsByWarehouse.ui dspCountSlipEditList.ui dspCountTagEditList.ui             \
               dspCountTagsByItem.ui dspCountTagsByWarehouse.ui dspCountTagsByClassCode.ui            \
               dspCustomerARHistory.ui dspCustomerInformation.ui dspCustomerInformationExport.ui      \
               dspCustomersByCharacteristic.ui dspCustomersByCustomerType.ui                          \
               dspInvoiceRegister.ui                                                                  \
               dspDetailedInventoryHistoryByLocation.ui dspDetailedInventoryHistoryByLotSerial.ui     \
               dspDepositsRegister.ui                                                                 \
               dspFinancialReport.ui dspFrozenItemSites.ui                                            \
               dspEarnedCommissions.ui dspExpediteExceptionsByPlannerCode.ui                          \
               dspExpiredInventoryByClassCode.ui                                                      \
               dspGLSeries.ui dspGLTransactions.ui                                                    \
               dspIndentedBOM.ui dspIndentedWhereUsed.ui dspInvalidBillsOfMaterials.ui                \
               dspInventoryAvailabilityByItem.ui dspInventoryAvailabilityByParameterList.ui           \
               dspInventoryAvailabilityBySalesOrder.ui dspInventoryAvailabilityBySourceVendor.ui      \
               dspInventoryAvailabilityByWorkOrder.ui dspInventoryAvailabilityByCustomerType.ui       \
               dspInventoryBufferStatusByParameterList.ui                                             \
               dspInventoryHistoryByItem.ui dspInventoryHistoryByOrderNumber.ui                       \
               dspInventoryHistoryByParameterList.ui dspIncidentsByCRMAccount.ui                      \
               dspInventoryLocator.ui dspInvoiceInformation.ui                                        \
               dspItemCostDetail.ui dspItemCostsByClassCode.ui dspItemCostSummary.ui                  \
               dspItemCostHistory.ui                                                                  \
               dspItemsByCharacteristic.ui dspItemsByClassCode.ui dspItemsByProductCategory.ui        \
               dspItemsWithoutItemSources.ui dspItemSitesByItem.ui dspItemSitesByParameterList.ui     \
               dspItemSourcesByItem.ui dspItemSourcesByVendor.ui dspJobCosting.ui                     \
               dspLaborVarianceByBOOItem.ui dspLaborVarianceByItem.ui                                 \
               dspLaborVarianceByWorkCenter.ui dspLaborVarianceByWorkOrder.ui                         \
               dspMaterialUsageVarianceByBOMItem.ui dspMaterialUsageVarianceByItem.ui                 \
               dspMaterialUsageVarianceByComponentItem.ui                                             \
               dspMaterialUsageVarianceByWorkOrder.ui dspMaterialUsageVarianceByWarehouse.ui          \
               dspMPSDetail.ui dspMRPDetail.ui openReturnAuthorizations.ui                            \
               openSalesOrders.ui openVouchers.ui dspOrders.ui dspPendingAvailability.ui              \
               dspPendingBOMChanges.ui dspPlannedOrdersByItem.ui dspPlannedOrdersByPlannerCode.ui     \
               dspPlannedRevenueExpensesByPlannerCode.ui dspPoHistory.ui                              \
               dspPoDeliveryDateVariancesByItem.ui dspPoDeliveryDateVariancesByVendor.ui              \
               dspPoItemsByBufferStatus.ui dspWoBufferStatusByParameterList.ui                        \
               dspPoItemsByDate.ui dspPoItemsByItem.ui dspPoItemsByVendor.ui                          \
               dspPoItemReceivingsByDate.ui dspPoItemReceivingsByItem.ui                              \
               dspPoItemReceivingsByVendor.ui                                                         \
               dspPoPriceVariancesByItem.ui dspPoPriceVariancesByVendor.ui dspPoReturnsByVendor.ui    \
               dspPOsByDate.ui dspPOsByVendor.ui                                                      \
               dspPurchaseReqsByItem.ui dspPurchaseReqsByPlannerCode.ui                               \
               dspQOHByParameterList.ui dspQOHByItem.ui dspQOHByLocation.ui                           \
               dspQuotesByCustomer.ui dspQuotesByItem.ui dspOrderActivityByProject.ui                 \
               dspOperationsByWorkCenter.ui dspPartiallyShippedOrders.ui                              \
               dspPricesByItem.ui dspPricesByCustomer.ui dspPricesByCustomerType.ui                   \
               dspFreightPricesByCustomer.ui dspFreightPricesByCustomerType.ui                        \
               dspReorderExceptionsByPlannerCode.ui dspRoughCutByWorkCenter.ui                        \
               dspReservations.ui                                                                     \
               dspRunningAvailability.ui                                                              \
               dspSalesHistoryByBilltoName.ui dspSalesHistoryByCustomer.ui                            \
               dspSalesHistoryByItem.ui dspSalesHistoryByParameterList.ui                             \
               dspSalesHistoryBySalesrep.ui dspSalesHistoryByShipTo.ui                                \
               dspSalesOrderStatus.ui dspSalesOrdersByItem.ui dspSalesOrdersByCustomer.ui             \
               dspSalesOrdersByCustomerPO.ui dspSalesOrdersByParameterList.ui                         \
               dspSequencedBOM.ui dspSingleLevelBOM.ui dspSingleLevelWhereUsed.ui                     \
               dspShipmentsByDate.ui dspShipmentsBySalesOrder.ui dspShipmentsByShipment.ui            \
               dspSlowMovingInventoryByClassCode.ui                                                   \
               dspStandardJournalHistory.ui dspStandardOperationsByWorkCenter.ui maintainShipping.ui  \
               dspSubstituteAvailabilityByItem.ui dspSummarizedBacklogByWarehouse.ui                  \
               dspSummarizedBankrecHistory.ui                                                         \
               dspSummarizedBOM.ui dspSummarizedGLTransactions.ui                                     \
               dspSummarizedSalesByCustomer.ui dspSummarizedSalesByCustomerByItem.ui                  \
               dspSummarizedSalesByCustomerType.ui dspSummarizedSalesByCustomerTypeByItem.ui          \
               dspSummarizedSalesByItem.ui                                                            \
               dspSummarizedSalesBySalesRep.ui dspSummarizedTaxableSales.ui                           \
               dspSummarizedSalesHistoryByShippingZone.ui                                             \
               dspTimePhasedAvailability.ui dspTimePhasedAvailableCapacityByWorkCenter.ui             \
               dspTimePhasedBookingsByCustomer.ui dspTimePhasedBookingsByItem.ui                      \
               dspTimePhasedBookingsByProductCategory.ui                                              \
               dspTimePhasedCapacityByWorkCenter.ui dspTimePhasedDemandByPlannerCode.ui               \
               dspTimePhasedLoadByWorkCenter.ui                                                       \
               dspTimePhasedOpenARItems.ui dspTimePhasedOpenAPItems.ui                                \
               dspTimePhasedRoughCutByWorkCenter.ui dspTimePhasedPlannedREByPlannerCode.ui            \
               dspTimePhasedProductionByPlannerCode.ui dspTimePhasedProductionByItem.ui               \
               dspTimePhasedSalesByCustomer.ui dspTimePhasedSalesByCustomerGroup.ui                   \
               dspTimePhasedSalesByCustomerByItem.ui dspTimePhasedSalesByItem.ui                      \
               dspTimePhasedSalesByProductCategory.ui                                                 \
               dspTimePhasedUsageStatisticsByItem.ui dspTrialBalances.ui                              \
               dspTodoByUserAndIncident.ui                                                            \
               dspUnbalancedQOHByClassCode.ui                                                         \
               dspUsageStatisticsByItem.ui dspUsageStatisticsByClassCode.ui                           \
               dspUsageStatisticsByItemGroup.ui dspUsageStatisticsByWarehouse.ui                      \
               dspUndefinedManufacturedItems.ui                                                       \
               dspUninvoicedReceivings.ui uninvoicedShipments.ui unpostedPurchaseOrders.ui            \
               dspUnusedPurchasedItems.ui  dspVendorAPHistory.ui dspValidLocationsByItem.ui           \
               dspVoucherRegister.ui  dspWoEffortByUser.ui dspWoEffortByWorkOrder.ui                  \
               dspWoHistoryByClassCode.ui dspWoHistoryByItem.ui dspWoHistoryByNumber.ui               \
               dspWoMaterialsByItem.ui dspWoMaterialsByWorkOrder.ui                                   \
               dspWoOperationBufrStsByWorkCenter.ui                                                   \
               dspWoOperationsByWorkCenter.ui dspWoOperationsByWorkOrder.ui                           \
               dspWoScheduleByItem.ui dspWoScheduleByParameterList.ui dspWoScheduleByWorkOrder.ui     \
               dspWoSoStatusMismatch.ui dspWoSoStatus.ui                                              \
               duplicateAccountNumbers.ui                                                             \
               ediForm.ui ediFormDetail.ui ediProfile.ui ediProfiles.ui                               \
               editICMWatermark.ui countSlip.ui countTag.ui                                           \
               employee.ui employees.ui empGroup.ui empGroups.ui                                      \
               enterMiscCount.ui                                                                      \
               enterPoitemReceipt.ui enterPoReceipt.ui enterPoitemReturn.ui enterPoReturn.ui          \
               errorLog.ui eventManager.ui                                                            \
               expenseCategories.ui expenseCategory.ui expenseTrans.ui                                \
               explodeWo.ui                                                                           \
	       externalShipping.ui externalShippingList.ui failedPostList.ui                          \
               financialLayout.ui financialLayoutItem.ui financialLayoutGroup.ui financialLayouts.ui  \
               financialLayoutSpecial.ui                          financialLayoutColumns.ui           \
               firmPlannedOrder.ui firmPlannedOrdersByPlannerCode.ui fixACL.ui fixSerial.ui           \
               freightBreakdown.ui freightClass.ui freightClasses.ui getGLDistDate.ui                 \
               getLotInfo.ui glSeries.ui glSeriesItem.ui glTransaction.ui glTransactionDetail.ui      \
               group.ui groups.ui                                                                     \
               honorific.ui honorifics.ui                                                             \
               hotkey.ui hotkeys.ui image.ui imageList.ui implodeWo.ui                                \
               images.ui invoiceList.ui issueWoMaterialBatch.ui                                       \
               importXML.ui                                                                           \
               incident.ui incidentWorkbench.ui                                                       \
               incidentCategory.ui incidentCategories.ui incidentPriority.ui incidentPriorities.ui    \
               incidentSeverity.ui incidentSeverities.ui incidentResolution.ui incidentResolutions.ui \
               issueWoMaterialItem.ui issueToShipping.ui issueLineToShipping.ui                       \
               item.ui itemAlias.ui itemCost.ui itemAvailabilityWorkbench.ui                          \
               itemGroups.ui itemGroup.ui itemImages.ui                                               \
               itemListPrice.ui items.ui                                                              \
               itemPricingSchedule.ui itemPricingScheduleItem.ui itemPricingSchedules.ui              \
               itemSite.ui itemSites.ui itemSourceSearch.ui                                           \
               itemSource.ui itemSources.ui itemSourceList.ui itemSourcePrice.ui itemSubstitute.ui    \
               itemtax.ui itemUOM.ui                                                                  \
               forms.ui form.ui labelForm.ui labelForms.ui laborRate.ui laborRates.ui                 \
               forwardUpdateAccounts.ui                                                               \
               sysLocale.ui locales.ui location.ui locations.ui                                       \
               lotSerial.ui lotSerialRegistration.ui maintainBudget.ui                                \
               maintainItemCosts.ui massExpireComponent.ui massReplaceComponent.ui                    \
               materialReceiptTrans.ui miscVoucher.ui miscCheck.ui                                    \
               invoice.ui invoiceItem.ui                                                              \
               opportunity.ui opportunityList.ui                                                      \
               opportunitySource.ui opportunitySources.ui opportunityStage.ui opportunityStages.ui    \
               opportunityType.ui opportunityTypes.ui                                                 \
               package.ui packages.ui                                                                 \
               packingListBatch.ui                                                                    \
               plannedOrder.ui plannerCodes.ui plannerCode.ui                                         \
               plannedSchedules.ui plannedSchedule.ui plannedScheduleItem.ui                          \
               poLiabilityDistrib.ui postCheck.ui postChecks.ui postCashReceipts.ui                   \
               postCostsByClassCode.ui postCostsByItem.ui                                             \
               postCountSlips.ui postCountTags.ui postBillingSelections.ui                            \
               postCreditMemos.ui postInvoices.ui                                                     \
               postPurchaseOrder.ui postPurchaseOrdersByAgent.ui postPoReturnCreditMemo.ui            \
               postOperations.ui postGLTransactionsToExternal.ui                                      \
               postProduction.ui postMiscProduction.ui                                                \
               postStandardJournal.ui postStandardJournalGroup.ui                                     \
               postVouchers.ui prepareCheckRun.ui priceList.ui                                        \
               pricingScheduleAssignment.ui pricingScheduleAssignments.ui                             \
               printAnnodizingPurchaseRequests.ui                                                     \
               printCheck.ui printChecks.ui printChecksReview.ui                                      \
               printCreditMemo.ui printCreditMemos.ui reprintCreditMemos.ui                           \
               printInvoice.ui printInvoices.ui printInvoicesByShipvia.ui reprintInvoices.ui          \
               printPackingList.ui                                                                    \
               printItemLabelsByClassCode.ui printLabelsByInvoice.ui printLabelsBySo.ui               \
               printLabelsByOrder.ui printLabelsByTo.ui printPackingListBatchByShipvia.ui printPoForm.ui \
               printProductionEntrySheet.ui printPurchaseOrder.ui                                     \
               printPurchaseOrdersByAgent.ui printSASpecialCalendarForm.ui printSoForm.ui             \
               printRaForm.ui                                                                         \
               printShippingForm.ui printShippingForms.ui                                             \
               printStatementByCustomer.ui printStatementsByCustomerType.ui                           \
               printVendorForm.ui                                                                     \
               printWoForm.ui printWoPickList.ui printWoRouting.ui printWoTraveler.ui                 \
               printJournal.ui                                                                        \
               productCategory.ui productCategories.ui profitCenter.ui profitCenters.ui               \
               project.ui projects.ui purchaseOrder.ui purchaseOrderItem.ui                           \
               prospect.ui prospects.ui                                                               \
               purchaseRequest.ui purgeClosedWorkOrders.ui purgeCreditMemos.ui purgeInvoices.ui       \
               purgePostedCounts.ui purgePostedCountSlips.ui                                          \
               purgeShippingRecords.ui quotes.ui                                                      \
               rate.ui reasonCode.ui reasonCodes.ui reassignLotSerial.ui recallOrders.ui              \
               rejectCodes.ui rejectCode.ui                                                           \
               reassignClassCodeByClassCode.ui reassignCustomerTypeByCustomerType.ui                  \
               reassignProductCategoryByProductCategory.ui reconcileBankaccount.ui                    \
               registration.ui                                                                        \
               releasePlannedOrdersByPlannerCode.ui releaseWorkOrdersByPlannerCode.ui                 \
               relativeCalendarItem.ui relocateInventory.ui editOwners.ui                             \
               reports.ui reprioritizeWo.ui                                                           \
               reschedulePoitem.ui  rescheduleSoLineItems.ui rescheduleWo.ui                          \
               reserveSalesOrderItem.ui                                                               \
               resetQOHBalances.ui returnAuthCheck.ui returnAuthorization.ui                          \
               returnAuthorizationItem.ui returnAuthItemLotSerial.ui                                  \
               returnAuthorizationWorkbench.ui returnWoMaterialBatch.ui returnWoMaterialItem.ui       \
               reverseGLSeries.ui                                                                     \
               runMPSByPlannerCode.ui                                                                 \
               sales.ui sale.ui salesAccount.ui salesAccounts.ui salesCategories.ui salesCategory.ui  \
               salesOrder.ui salesOrderInformation.ui salesOrderItem.ui                               \
               salesReps.ui salesRep.ui                                                               \
               salesHistoryInformation.ui scrapTrans.ui scrapWoMaterialFromWIP.ui                     \
               scriptEditor.ui scripts.ui                                                             \
               searchForCRMAccount.ui searchForContact.ui searchForItem.ui searchForEmp.ui            \
               selectBankAccount.ui selectBillingQty.ui selectOrderForBilling.ui                      \
               selectedPayments.ui selectPayment.ui selectPayments.ui                                 \
               selectShippedOrders.ui shift.ui shifts.ui shipOrder.ui shippingInformation.ui          \
               shippingForm.ui shippingForms.ui shipTo.ui                                             \
               shippingChargeType.ui shippingChargeTypes.ui                                           \
               shippingZones.ui shippingZone.ui                                                       \
               shipVias.ui shipVia.ui siteType.ui siteTypes.ui splitReceipt.ui                        \
               standardOperations.ui standardOperation.ui standardJournal.ui standardJournals.ui      \
               standardJournalGroup.ui standardJournalGroupItem.ui standardJournalGroups.ui           \
               standardJournalItem.ui subaccount.ui subaccounts.ui subAccntTypes.ui subAccntType.ui   \
               submitAction.ui submitReport.ui substituteList.ui summarizeInvTransByClassCode.ui      \
               syncCompanies.ui                                                                       \
               systemMessage.ui                                                                       \
               taxAuthorities.ui taxAuthority.ui taxCodes.ui taxCode.ui taxDetail.ui                  \
               taxBreakdown.ui                                                                        \
               taxRegistration.ui taxRegistrations.ui                                                 \
               taxSelection.ui taxSelections.ui                                                       \
               taxType.ui taxTypes.ui                                                                 \
               task.ui thawItemSitesByClassCode.ui                                                    \
               termses.ui terms.ui todoList.ui todoItem.ui todoListCalendar.ui                        \
               transactionInformation.ui transferOrder.ui transferOrders.ui                           \
               transferOrderItem.ui                                                                   \
               transferTrans.ui                                                                       \
               transformTrans.ui                                                                      \
               uiform.ui uiforms.ui                                                                   \
               unappliedARCreditMemos.ui unappliedAPCreditMemos.ui                                    \
               unpostedCreditMemos.ui                                                                 \
               unpostedGLTransactions.ui unpostedInvoices.ui                                          \
               unpostedPoReceipts.ui unpostedGlSeries.ui                                              \
               uom.ui uoms.ui uomConv.ui                                                              \
               updateABCClass.ui updateActualCostsByClassCode.ui updateActualCostsByItem.ui           \
               updateCreditStatusByCustomer.ui updateCycleCountFrequency.ui                           \
               updateItemSiteLeadTimes.ui updateListPricesByProductCategory.ui                        \
               updateLateCustCreditStatus.ui                                                          \
               updateOUTLevelByItem.ui updateOUTLevels.ui updateOUTLevelsByClassCode.ui               \
               updatePricesByProductCategory.ui  updatePricesByPricingSchedule.ui                     \
               updateReorderLevels.ui   							      \
               users.ui user.ui userList.ui userPreferences.ui                                        \
               userCostingElement.ui costingElements.ui                                               \
               vendor.ui vendors.ui vendorWorkBench.ui                                                \
               vendorAddress.ui vendorAddressList.ui                                                  \
               vendorType.ui vendorTypes.ui viewCheckRun.ui voidChecks.ui                             \
               voucher.ui voucheringEditList.ui voucherItemDistrib.ui voucherItem.ui                  \
               voucherMiscDistrib.ui                                                                  \
               warehouses.ui warehouse.ui warehouseZone.ui                                            \
               whseCalendar.ui whseCalendars.ui whseWeek.ui                                           \
               workCenter.ui workCenters.ui                                                           \
               workOrder.ui workOrderMaterials.ui woTimeClock.ui wotc.ui                              \
               woOperation.ui workOrderOperations.ui                                                  \
               zeroUncountedCountTagsByWarehouse.ui                                                   \
               externalCCTransaction.ui                                                               \
               idleShutdown.ui xdateinputdialog.ui xsltMap.ui

HEADERS      = version.h inputManager.h guiclient.h timeoutHandler.h                                  \
               ../common/format.h SaveSizePositionEventFilter.h                                       \
               menuProducts.h menuInventory.h menuSchedule.h menuPurchase.h                           \
               menuManufacture.h menuCRM.h menuSales.h menuAccounting.h menuSystem.h                  \
               taxCache.h xmessagebox.h                                                               \
               absoluteCalendarItem.h accountNumber.h accountNumbers.h                                \
               accountingPeriod.h accountingPeriods.h accountingYearPeriod.h accountingYearPeriods.h  \
               addPoComment.h address.h addresses.h                                                   \
               adjustmentTrans.h assignClassCodeToPlannerCode.h assignItemToPlannerCode.h             \
               allocateReservations.h                                                                 \
               apAccountAssignment.h apAccountAssignments.h                                           \
               applyAPCreditMemo.h apOpenItem.h apCreditMemoApplication.h                             \
               applyDiscount.h                                                                        \
               arAccountAssignment.h arAccountAssignments.h                                           \
               applyARCreditMemo.h arOpenItem.h arCreditMemoApplication.h                             \
               archRestoreSalesHistory.h arWorkBench.h apWorkBench.h                                  \
               assignLotSerial.h                                                                      \
               ../common/batchManager.h ../common/batchItem.h                                         \
               bankAccounts.h bankAccount.h                                                           \
               bankAdjustment.h bankAdjustmentEditList.h                                              \
               bankAdjustmentTypes.h bankAdjustmentType.h                                             \
               bboms.h bbomItem.h bbom.h                                                              \
               bom.h bomItem.h bomList.h boo.h booItem.h booItemList.h booList.h buyCard.h            \
               booitemImage.h budgets.h                                                               \
               calendars.h calendar.h                                                                 \
               cashReceipt.h cashReceiptsEditList.h cashReceiptItem.h cashReceiptMiscDistrib.h        \
               changePoitemQty.h changeWoQty.h changeQtyToDistributeFromBreeder.h                     \
               characteristic.h characteristics.h characteristicAssignment.h                          \
               characteristicPrice.h check.h checkFormat.h checkFormats.h                             \
               classCodes.h classCode.h closePurchaseOrder.h closeWo.h                                \
               commentType.h commentTypes.h company.h companies.h configureAccountingSystem.h         \
               configureCC.h configureSO.h configureIM.h                                              \
               configureGL.h configureMS.h configurePD.h configurePO.h                                \
               configureWO.h configureBackup.h configureCRM.h                                         \
               configureIE.h configureEncryption.h                                                    \
               confirmAchOK.h                                                                         \
               contact.h contacts.h copyPurchaseOrder.h copyBudget.h                                  \
               copyBOM.h copyBOO.h copyItem.h copyPlannedSchedule.h copySalesOrder.h                  \
               copyTransferOrder.h                                                                    \
               costCategory.h costCategories.h countries.h country.h                                  \
               countTagList.h correctOperationsPosting.h correctProductionPosting.h                   \
               createBufferStatusByItem.h createBufferStatusByPlannerCode.h                           \
               createCycleCountTags.h                                                                 \
               createCountTagsByParameterList.h createCountTagsByItem.h                               \
               createItemSitesByClassCode.h                                                           \
               createPlannedOrdersByItem.h createPlannedOrdersByPlannerCode.h                         \
               createRecurringInvoices.h                                                              \
               woMaterialItem.h createLotSerial.h creditMemo.h                                        \
               creditMemoItem.h creditMemoEditList.h                                                  \
               crmaccount.h crmaccounts.h                                                             \
               currencies.h currency.h currencyConversion.h currencyConversions.h                     \
               currencySelect.h                                                                       \
               customCommands.h customCommand.h customCommandArgument.h                               \
               customer.h custCharacteristicDelegate.h  customerGroups.h customerGroup.h creditCard.h \
               customerFormAssignment.h customerFormAssignments.h                                     \
               customers.h customerTypes.h customerType.h customerTypeList.h                          \
               databaseInformation.h deletePlannedOrder.h deletePlannedOrdersByPlannerCode.h          \
               deliverEmail.h									      \
               deliverInvoice.h deliverPurchaseOrder.h deliverSalesOrder.h                            \
               department.h departments.h                                                             \
               distributeBreederProduction.h                        distributeInventory.h             \
               distributeToLocation.h                                                                 \
               dspAllocations.h dspAPApplications.h                                                   \
               dspAPOpenItemsByVendor.h dspAROpenItemsByCustomer.h                                    \
               dspAROpenItems.h                                                                       \
               dspARApplications.h dspBacklogByCustomer.h dspBacklogByParameterList.h                 \
               dspBacklogByItem.h dspBacklogBySalesOrder.h                                            \
               dspBankrecHistory.h                                                                    \
               billingEditList.h dspBillingSelections.h                                               \
               dspBookingsByCustomer.h dspBookingsByCustomerGroup.h dspBookingsByItem.h               \
               dspBookingsByProductCategory.h dspBookingsBySalesRep.h dspBookingsByShipTo.h           \
               dspBreederDistributionVarianceByItem.h dspBreederDistributionVarianceByWarehouse.h     \
               dspBriefEarnedCommissions.h                                                            \
               dspBriefSalesHistoryByCustomer.h dspBriefSalesHistoryByCustomerType.h                  \
               dspBriefSalesHistoryBySalesRep.h dspCapacityBufferStatusByWorkCenter.h                 \
               dspCapacityUOMsByClassCode.h dspCapacityUOMsByProductCategory.h                        \
               dspCheckRegister.h dspCashReceipts.h                                                   \
               dspCostedSingleLevelBOM.h dspCostedSummarizedBOM.h dspCostedIndentedBOM.h              \
               dspCountSlipsByWarehouse.h dspCountSlipEditList.h dspCountTagEditList.h                \
               dspCountTagsByItem.h dspCountTagsByWarehouse.h dspCountTagsByClassCode.h               \
               dspCustomerARHistory.h dspCustomerInformation.h dspCustomerInformationExport.h         \
               dspCustomersByCharacteristic.h dspCustomersByCustomerType.h                            \
               dspDetailedInventoryHistoryByLocation.h dspDetailedInventoryHistoryByLotSerial.h       \
               dspDepositsRegister.h                                                                  \
               dspFinancialReport.h dspFrozenItemSites.h                                              \
               dspEarnedCommissions.h dspExpediteExceptionsByPlannerCode.h                            \
               dspExpiredInventoryByClassCode.h                                                       \
               dspGLSeries.h dspGLTransactions.h                                                      \
               dspIndentedBOM.h dspIndentedWhereUsed.h dspInvalidBillsOfMaterials.h                   \
               dspInventoryAvailabilityByItem.h dspInventoryAvailabilityByParameterList.h             \
               dspInventoryAvailabilityBySalesOrder.h dspInventoryAvailabilityBySourceVendor.h        \
               dspInventoryAvailabilityByWorkOrder.h dspInventoryAvailabilityByCustomerType.h         \
               dspInventoryBufferStatusByParameterList.h                                              \
               dspInventoryHistoryByItem.h dspInventoryHistoryByOrderNumber.h                         \
               dspInventoryHistoryByParameterList.h dspIncidentsByCRMAccount.h                        \
               dspInventoryLocator.h dspInvoiceInformation.h                                          \
               dspInvoiceRegister.h                                                                   \
               dspItemCostDetail.h dspItemCostsByClassCode.h dspItemCostSummary.h                     \
               dspItemCostHistory.h                                                                   \
               dspItemsByCharacteristic.h dspItemsByClassCode.h dspItemsByProductCategory.h           \
               dspItemsWithoutItemSources.h dspItemSitesByItem.h dspItemSitesByParameterList.h        \
               dspItemSourcesByItem.h dspItemSourcesByVendor.h dspJobCosting.h                        \
               dspLaborVarianceByBOOItem.h dspLaborVarianceByItem.h                                   \
               dspLaborVarianceByWorkCenter.h dspLaborVarianceByWorkOrder.h                           \
               dspMaterialUsageVarianceByBOMItem.h dspMaterialUsageVarianceByItem.h                   \
               dspMaterialUsageVarianceByComponentItem.h                                              \
               dspMaterialUsageVarianceByWorkOrder.h dspMaterialUsageVarianceByWarehouse.h            \
               dspMPSDetail.h dspMRPDetail.h openReturnAuthorizations.h                               \
               openSalesOrders.h openVouchers.h dspOrders.h dspPendingAvailability.h                  \
               dspPendingBOMChanges.h dspPlannedOrdersByItem.h dspPlannedOrdersByPlannerCode.h        \
               dspPlannedRevenueExpensesByPlannerCode.h dspPoHistory.h                                \
               dspPoDeliveryDateVariancesByItem.h dspPoDeliveryDateVariancesByVendor.h                \
               dspPoItemsByDate.h dspPoItemsByItem.h dspPoItemsByVendor.h                             \
               dspPoItemReceivingsByDate.h dspPoItemReceivingsByItem.h                                \
               dspPoItemReceivingsByVendor.h dspPoItemsByBufferStatus.h                               \
               dspPoPriceVariancesByItem.h dspPoPriceVariancesByVendor.h dspPoReturnsByVendor.h       \
               dspPOsByDate.h dspPOsByVendor.h                                                        \
               dspPurchaseReqsByItem.h dspPurchaseReqsByPlannerCode.h                                 \
               dspQOHByParameterList.h dspQOHByItem.h dspQOHByLocation.h                              \
               dspQuotesByCustomer.h dspQuotesByItem.h dspOrderActivityByProject.h                    \
               dspOperationsByWorkCenter.h dspPartiallyShippedOrders.h                                \
               dspPricesByItem.h dspPricesByCustomer.h dspPricesByCustomerType.h                      \
               dspFreightPricesByCustomer.h dspFreightPricesByCustomerType.h                          \
               dspReorderExceptionsByPlannerCode.h dspRoughCutByWorkCenter.h                          \
               dspReservations.h                                                                      \
               dspRunningAvailability.h                                                               \
               dspSalesHistoryByBilltoName.h dspSalesHistoryByCustomer.h                              \
               dspSalesHistoryByItem.h dspSalesHistoryByParameterList.h                               \
               dspSalesHistoryBySalesrep.h dspSalesHistoryByShipTo.h                                  \
               dspSalesOrderStatus.h dspSalesOrdersByItem.h dspSalesOrdersByCustomer.h                \
               dspSalesOrdersByCustomerPO.h dspSalesOrdersByParameterList.h                           \
               dspSequencedBOM.h dspSingleLevelBOM.h dspSingleLevelWhereUsed.h                        \
               dspShipmentsByDate.h dspShipmentsBySalesOrder.h dspShipmentsByShipment.h               \
               dspSlowMovingInventoryByClassCode.h                                                    \
               dspStandardJournalHistory.h dspStandardOperationsByWorkCenter.h maintainShipping.h     \
               dspSubstituteAvailabilityByItem.h dspSummarizedBacklogByWarehouse.h                    \
               dspSummarizedBankrecHistory.h                                                          \
               dspSummarizedBOM.h dspSummarizedGLTransactions.h                                       \
               dspSummarizedSalesByCustomer.h dspSummarizedSalesByCustomerByItem.h                    \
               dspSummarizedSalesByCustomerType.h dspSummarizedSalesByCustomerTypeByItem.h            \
               dspSummarizedSalesByItem.h                                                             \
               dspSummarizedSalesBySalesRep.h dspSummarizedTaxableSales.h                             \
               dspSummarizedSalesHistoryByShippingZone.h                                              \
               dspTimePhasedAvailability.h dspTimePhasedAvailableCapacityByWorkCenter.h               \
               dspTimePhasedBookingsByCustomer.h dspTimePhasedBookingsByItem.h                        \
               dspTimePhasedBookingsByProductCategory.h                                               \
               dspTimePhasedCapacityByWorkCenter.h dspTimePhasedDemandByPlannerCode.h                 \
               dspTimePhasedLoadByWorkCenter.h                                                        \
               dspTimePhasedOpenARItems.h dspTimePhasedOpenAPItems.h                                  \
               dspTimePhasedRoughCutByWorkCenter.h dspTimePhasedPlannedREByPlannerCode.h              \
               dspTimePhasedProductionByPlannerCode.h dspTimePhasedProductionByItem.h                 \
               dspTimePhasedSalesByCustomer.h dspTimePhasedSalesByCustomerGroup.h                     \
               dspTimePhasedSalesByCustomerByItem.h dspTimePhasedSalesByItem.h                        \
               dspTimePhasedSalesByProductCategory.h                                                  \
               dspTimePhasedUsageStatisticsByItem.h dspTrialBalances.h                                \
               dspTodoByUserAndIncident.h                                                             \
               dspUnbalancedQOHByClassCode.h                                                          \
               dspUsageStatisticsByItem.h dspUsageStatisticsByClassCode.h                             \
               dspUsageStatisticsByItemGroup.h dspUsageStatisticsByWarehouse.h                        \
               dspUndefinedManufacturedItems.h                                                        \
               dspUninvoicedReceivings.h uninvoicedShipments.h unpostedPurchaseOrders.h               \
               dspUnusedPurchasedItems.h  dspVendorAPHistory.h dspValidLocationsByItem.h              \
               dspVoucherRegister.h dspWoBufferStatusByParameterList.h                                \
               dspWoEffortByUser.h dspWoEffortByWorkOrder.h                                           \
               dspWoHistoryByClassCode.h dspWoHistoryByItem.h dspWoHistoryByNumber.h                  \
               dspWoMaterialsByItem.h dspWoMaterialsByWorkOrder.h                                     \
               dspWoOperationBufrStsByWorkCenter.h                                                    \
               dspWoOperationsByWorkCenter.h dspWoOperationsByWorkOrder.h                             \
               dspWoScheduleByItem.h dspWoScheduleByParameterList.h dspWoScheduleByWorkOrder.h        \
               dspWoSoStatusMismatch.h dspWoSoStatus.h                                                \
               duplicateAccountNumbers.h                                                              \
               ediForm.h ediFormDetail.h ediProfile.h ediProfiles.h                                   \
               editICMWatermark.h countSlip.h countTag.h                                              \
               employee.h employees.h empGroup.h  empGroups.h                                         \
               enterMiscCount.h                                                                       \
               enterPoitemReceipt.h enterPoReceipt.h enterPoitemReturn.h enterPoReturn.h              \
               errorLog.h eventManager.h                                                              \
               expenseCategories.h expenseCategory.h expenseTrans.h                                   \
               explodeWo.h                                                                            \
	       externalShipping.h externalShippingList.h failedPostList.h                             \
               financialLayout.h financialLayoutItem.h financialLayoutGroup.h financialLayouts.h      \
               financialLayoutSpecial.h                         financialLayoutColumns.h              \
               firmPlannedOrder.h firmPlannedOrdersByPlannerCode.h fixACL.h fixSerial.h               \
               freightBreakdown.h freightClass.h freightClasses.h getGLDistDate.h                     \
               getLotInfo.h glSeries.h glSeriesItem.h glTransaction.h glTransactionDetail.h           \
               group.h groups.h                                                                       \
               honorific.h honorifics.h                                                               \
               hotkey.h hotkeys.h image.h imageList.h implodeWo.h                                     \
               images.h invoiceList.h issueWoMaterialBatch.h                                          \
               importXML.h                                                                            \
               incident.h incidentWorkbench.h                                                         \
               incidentCategory.h incidentCategories.h incidentPriority.h incidentPriorities.h        \
               incidentSeverity.h incidentSeverities.h incidentResolution.h incidentResolutions.h     \
               issueWoMaterialItem.h issueToShipping.h issueLineToShipping.h                          \
               item.h itemAlias.h itemCost.h itemAvailabilityWorkbench.h                              \
               itemCharacteristicDelegate.h                                                           \
               itemGroups.h itemGroup.h itemImages.h                                                  \
               itemListPrice.h items.h                                                                \
               itemPricingSchedule.h itemPricingScheduleItem.h itemPricingSchedules.h                 \
               itemSite.h itemSites.h itemSourceSearch.h                                              \
               itemSource.h itemSources.h itemSourceList.h itemSourcePrice.h itemSubstitute.h         \
               itemtax.h itemUOM.h                                                                    \
               forms.h form.h labelForm.h labelForms.h laborRate.h laborRates.h                       \
               forwardUpdateAccounts.h                                                                \
               sysLocale.h locales.h location.h locations.h                                           \
               lotSerial.h lotSerialRegistration.h maintainBudget.h                                   \
               maintainItemCosts.h massExpireComponent.h massReplaceComponent.h                       \
               materialReceiptTrans.h miscVoucher.h miscCheck.h                                       \
               mqlutil.h                                                                              \
               invoice.h invoiceItem.h                                                                \
               opportunity.h opportunityList.h                                                        \
               opportunitySource.h opportunitySources.h opportunityStage.h opportunityStages.h        \
               opportunityType.h opportunityTypes.h                                                   \
               package.h packages.h                                                                   \
               packingListBatch.h                                                                     \
               plannedOrder.h plannerCodes.h plannerCode.h                                            \
               plannedSchedules.h plannedSchedule.h plannedScheduleItem.h                             \
               poitemTableModel.h poitemTableView.h poLiabilityDistrib.h                              \
               postCheck.h postChecks.h postCashReceipts.h                                            \
               postCostsByClassCode.h postCostsByItem.h                                               \
               postCountSlips.h postCountTags.h postBillingSelections.h                               \
               postCreditMemos.h postInvoices.h                                                       \
               postPurchaseOrder.h postPurchaseOrdersByAgent.h postPoReturnCreditMemo.h               \
               postOperations.h postGLTransactionsToExternal.h                                        \
               postProduction.h postMiscProduction.h                                                  \
               postStandardJournal.h postStandardJournalGroup.h                                       \
               postVouchers.h prepareCheckRun.h priceList.h                                           \
               pricingScheduleAssignment.h pricingScheduleAssignments.h                               \
               printAnnodizingPurchaseRequests.h                                                      \
               printCheck.h printChecks.h printChecksReview.h                                         \
               printCreditMemo.h printCreditMemos.h reprintCreditMemos.h                              \
               printInvoice.h printInvoices.h printInvoicesByShipvia.h reprintInvoices.h              \
               printPackingList.h                                                                     \
               printItemLabelsByClassCode.h printLabelsByInvoice.h printLabelsBySo.h                  \
               printLabelsByOrder.h printLabelsByTo.h printPackingListBatchByShipvia.h printPoForm.h  \
               printProductionEntrySheet.h printPurchaseOrder.h                                       \
               printPurchaseOrdersByAgent.h printSASpecialCalendarForm.h printSoForm.h                \
               printRaForm.h                                                                          \
               printShippingForm.h printShippingForms.h                                               \
               printStatementByCustomer.h printStatementsByCustomerType.h                             \
               printVendorForm.h                                                                      \
               printWoForm.h printWoPickList.h printWoRouting.h printWoTraveler.h                     \
               printJournal.h                                                                         \
               productCategory.h productCategories.h profitCenter.h profitCenters.h                   \
               project.h projects.h purchaseOrder.h purchaseOrderItem.h                               \
               prospect.h prospects.h                                                                 \
               purchaseRequest.h purgeClosedWorkOrders.h purgeCreditMemos.h purgeInvoices.h           \
               purgePostedCounts.h purgePostedCountSlips.h                                            \
               purgeShippingRecords.h quotes.h                                                        \
               rate.h reasonCode.h reasonCodes.h reassignLotSerial.h recallOrders.h                   \
               rejectCodes.h rejectCode.h                                                             \
               reassignClassCodeByClassCode.h reassignCustomerTypeByCustomerType.h                    \
               reassignProductCategoryByProductCategory.h reconcileBankaccount.h                      \
               registration.h                                                                         \
               releasePlannedOrdersByPlannerCode.h releaseWorkOrdersByPlannerCode.h                   \
               relativeCalendarItem.h relocateInventory.h editOwners.h                                \
               reports.h reprioritizeWo.h                                                             \
               reschedulePoitem.h  rescheduleSoLineItems.h rescheduleWo.h                             \
               reserveSalesOrderItem.h                                                                \
               resetQOHBalances.h returnAuthCheck.h returnAuthorization.h returnAuthorizationItem.h   \
	       returnAuthItemLotSerial.h                                                              \
               returnAuthorizationWorkbench.h returnWoMaterialBatch.h returnWoMaterialItem.h          \
               reverseGLSeries.h                                                                      \
               runMPSByPlannerCode.h                                                                  \
               sales.h sale.h salesAccount.h salesAccounts.h salesCategories.h salesCategory.h        \
               salesOrder.h salesOrderInformation.h salesOrderItem.h                                  \
               salesReps.h salesRep.h                                                                 \
               salesHistoryInformation.h scrapTrans.h scrapWoMaterialFromWIP.h                        \
               scriptEditor.h scripts.h                                                               \
               scriptquery.h scripttoolbox.h                                                          \
               searchForCRMAccount.h searchForContact.h searchForItem.h searchForEmp.h                \
               selectBankAccount.h selectBillingQty.h selectOrderForBilling.h                         \
               selectedPayments.h selectPayment.h selectPayments.h                                    \
               selectShippedOrders.h shift.h shifts.h shipOrder.h shippingInformation.h               \
               shippingForm.h shippingForms.h shipTo.h                                                \
               shippingChargeType.h shippingChargeTypes.h                                             \
               shippingZones.h shippingZone.h                                                         \
               shipVias.h shipVia.h siteType.h siteTypes.h splitReceipt.h                             \
               standardOperations.h standardOperation.h standardJournal.h standardJournals.h          \
               standardJournalGroup.h standardJournalGroupItem.h standardJournalGroups.h              \
               standardJournalItem.h subaccount.h subaccounts.h subAccntTypes.h subAccntType.h        \
               submitAction.h submitReport.h substituteList.h summarizeInvTransByClassCode.h          \
               syncCompanies.h                                                                        \
               systemMessage.h                                                                        \
               taxAuthorities.h taxAuthority.h taxCodes.h taxCode.h taxDetail.h                       \
               taxBreakdown.h                                                                         \
               taxRegistration.h taxRegistrations.h                                                   \
               taxSelection.h taxSelections.h                                                         \
               taxType.h taxTypes.h                                                                   \
               task.h thawItemSitesByClassCode.h                                                      \
               termses.h terms.h                                                                      \
               todoList.h todoItem.h todoListCalendar.h todoCalendarControl.h                         \
               toitemTableModel.h toitemTableView.h                                                   \
               transactionInformation.h transferOrder.h transferOrders.h                              \
               transferOrderItem.h                                                                    \
               transferTrans.h                                                                        \
               transformTrans.h                                                                       \
               uiform.h uiforms.h                                                                     \
               unappliedARCreditMemos.h unappliedAPCreditMemos.h                                      \
               unpostedCreditMemos.h                                                                  \
               unpostedGLTransactions.h unpostedInvoices.h                                            \
               unpostedPoReceipts.h unpostedGlSeries.h                                                \
               uom.h uoms.h uomConv.h                                                                 \
               updateABCClass.h updateActualCostsByClassCode.h updateActualCostsByItem.h              \
               updateCreditStatusByCustomer.h updateCycleCountFrequency.h                             \
               updateItemSiteLeadTimes.h updateListPricesByProductCategory.h                          \
               updateLateCustCreditStatus.h                                                           \
               updateOUTLevelByItem.h updateOUTLevels.h updateOUTLevelsByClassCode.h                  \
               updatePricesByProductCategory.h  updatePricesByPricingSchedule.h                       \
               updateReorderLevels.h								      \
               users.h user.h userList.h userPreferences.h                                            \
               userCostingElement.h costingElements.h                                                 \
               vendor.h vendors.h vendorWorkBench.h                                                   \
               vendorAddress.h vendorAddressList.h                                                    \
               vendorType.h vendorTypes.h viewCheckRun.h voidChecks.h                                 \
               voucher.h voucheringEditList.h voucherItemDistrib.h voucherItem.h                      \
               voucherMiscDistrib.h                                                                   \
               warehouses.h warehouse.h warehouseZone.h                                               \
               whseCalendar.h whseCalendars.h whseWeek.h workCenter.h workCenters.h                   \
               workOrder.h workOrderMaterials.h woTimeClock.h wotc.h                                  \
               woOperation.h workOrderOperations.h                                                    \
               zeroUncountedCountTagsByWarehouse.h                                                    \
	       creditcardprocessor.h authorizedotnetprocessor.h verisignprocessor.h                   \
	       yourpayprocessor.h externalccprocessor.h externalCCTransaction.h                       \
               xmainwindow.h xdialog.h xuiloader.h xwidget.h                                          \
               idleShutdown.h xdateinputdialog.h xsltMap.h                                            \
               xTupleDesigner.h xTupleDesignerActions.h


SOURCES      = main.cpp inputManager.cpp guiclient.cpp timeoutHandler.cpp                             \
               menuProducts.cpp menuInventory.cpp menuSchedule.cpp menuPurchase.cpp                   \
               menuManufacture.cpp menuCRM.cpp menuSales.cpp menuAccounting.cpp menuSystem.cpp        \
               taxCache.cpp xmessagebox.cpp                                                           \
               absoluteCalendarItem.cpp accountNumber.cpp accountNumbers.cpp                          \
               accountingPeriod.cpp accountingPeriods.cpp accountingYearPeriod.cpp accountingYearPeriods.cpp \
               addPoComment.cpp address.cpp addresses.cpp                                             \
               adjustmentTrans.cpp assignClassCodeToPlannerCode.cpp assignItemToPlannerCode.cpp       \
               allocateReservations.cpp                                                               \
               apAccountAssignment.cpp apAccountAssignments.cpp                                       \
               applyAPCreditMemo.cpp apOpenItem.cpp apCreditMemoApplication.cpp                       \
               applyDiscount.cpp                                                                      \
               arAccountAssignment.cpp arAccountAssignments.cpp                                       \
               applyARCreditMemo.cpp arOpenItem.cpp arCreditMemoApplication.cpp                       \
               archRestoreSalesHistory.cpp arWorkBench.cpp apWorkBench.cpp                            \
               assignLotSerial.cpp                                                                    \
               ../common/batchManager.cpp ../common/batchItem.cpp                                     \
               bankAccounts.cpp bankAccount.cpp                                                       \
               bankAdjustment.cpp bankAdjustmentEditList.cpp                                          \
               bankAdjustmentTypes.cpp bankAdjustmentType.cpp                                         \
               bboms.cpp bbomItem.cpp bbom.cpp                                                        \
               bom.cpp bomItem.cpp bomList.cpp boo.cpp booItem.cpp booItemList.cpp booList.cpp buyCard.cpp \
               booitemImage.cpp budgets.cpp                                                           \
               calendars.cpp calendar.cpp                                                             \
               cashReceipt.cpp cashReceiptsEditList.cpp cashReceiptItem.cpp cashReceiptMiscDistrib.cpp \
               changePoitemQty.cpp changeWoQty.cpp changeQtyToDistributeFromBreeder.cpp               \
               characteristic.cpp characteristics.cpp characteristicAssignment.cpp                    \
               characteristicPrice.cpp check.cpp checkFormat.cpp checkFormats.cpp                     \
               classCodes.cpp classCode.cpp closePurchaseOrder.cpp closeWo.cpp                        \
               commentType.cpp commentTypes.cpp company.cpp companies.cpp configureAccountingSystem.cpp \
               configureCC.cpp configureSO.cpp configureIM.cpp                                        \
               configureGL.cpp configureMS.cpp configurePD.cpp configurePO.cpp                        \
               configureWO.cpp configureBackup.cpp configureCRM.cpp                                   \
               configureIE.cpp configureEncryption.cpp                                                \
               confirmAchOK.cpp                                                                       \
               contact.cpp contacts.cpp copyPurchaseOrder.cpp copyBudget.cpp                          \
               copyBOM.cpp copyBOO.cpp copyItem.cpp copyPlannedSchedule.cpp copySalesOrder.cpp        \
               copyTransferOrder.cpp                                                                  \
               costCategory.cpp costCategories.cpp countries.cpp country.cpp                          \
               countTagList.cpp correctOperationsPosting.cpp correctProductionPosting.cpp             \
               createBufferStatusByItem.cpp createBufferStatusByPlannerCode.cpp                       \
               createCycleCountTags.cpp                                                               \
               createCountTagsByParameterList.cpp createCountTagsByItem.cpp                           \
               createItemSitesByClassCode.cpp                                                         \
               createPlannedOrdersByItem.cpp createPlannedOrdersByPlannerCode.cpp                     \
               createRecurringInvoices.cpp                                                            \
               woMaterialItem.cpp createLotSerial.cpp creditMemo.cpp                                  \
               creditMemoItem.cpp creditMemoEditList.cpp                                              \
               crmaccount.cpp crmaccounts.cpp                                                         \
               currencies.cpp currency.cpp currencyConversion.cpp currencyConversions.cpp             \
               currencySelect.cpp                                                                     \
               customCommands.cpp customCommand.cpp customCommandArgument.cpp                         \
               customer.cpp custCharacteristicDelegate.cpp  customerGroups.cpp customerGroup.cpp      \
               creditCard.cpp                                                                         \
               customerFormAssignment.cpp customerFormAssignments.cpp                                 \
               customers.cpp customerTypes.cpp customerType.cpp customerTypeList.cpp                  \
               databaseInformation.cpp deletePlannedOrder.cpp deletePlannedOrdersByPlannerCode.cpp    \
               deliverEmail.cpp									      \
               deliverInvoice.cpp deliverPurchaseOrder.cpp deliverSalesOrder.cpp                      \
               department.cpp departments.cpp                                                         \
               distributeBreederProduction.cpp                          distributeInventory.cpp       \
               distributeToLocation.cpp                                                               \
               dspAllocations.cpp dspAPApplications.cpp                                               \
               dspAPOpenItemsByVendor.cpp dspAROpenItemsByCustomer.cpp                                \
               dspAROpenItems.cpp                                                                     \
               dspARApplications.cpp dspBacklogByCustomer.cpp dspBacklogByParameterList.cpp           \
               dspBacklogByItem.cpp dspBacklogBySalesOrder.cpp                                        \
               dspBankrecHistory.cpp                                                                  \
               billingEditList.cpp dspBillingSelections.cpp                                           \
               dspBookingsByCustomer.cpp dspBookingsByCustomerGroup.cpp dspBookingsByItem.cpp         \
               dspBookingsByProductCategory.cpp dspBookingsBySalesRep.cpp dspBookingsByShipTo.cpp     \
               dspBreederDistributionVarianceByItem.cpp dspBreederDistributionVarianceByWarehouse.cpp \
               dspBriefEarnedCommissions.cpp                                                          \
               dspBriefSalesHistoryByCustomer.cpp dspBriefSalesHistoryByCustomerType.cpp              \
               dspBriefSalesHistoryBySalesRep.cpp                                                     \
               dspCapacityUOMsByClassCode.cpp dspCapacityUOMsByProductCategory.cpp                    \
               dspCheckRegister.cpp dspCashReceipts.cpp                                               \
               dspCostedSingleLevelBOM.cpp dspCostedSummarizedBOM.cpp dspCostedIndentedBOM.cpp        \
               dspCountSlipsByWarehouse.cpp dspCountSlipEditList.cpp dspCountTagEditList.cpp          \
               dspCountTagsByItem.cpp dspCountTagsByWarehouse.cpp dspCountTagsByClassCode.cpp         \
               dspCustomerARHistory.cpp dspCustomerInformation.cpp dspCustomerInformationExport.cpp   \
               dspCustomersByCharacteristic.cpp dspCustomersByCustomerType.cpp                        \
               dspInvoiceRegister.cpp                                                                 \
               dspDetailedInventoryHistoryByLocation.cpp dspDetailedInventoryHistoryByLotSerial.cpp   \
               dspDepositsRegister.cpp                                                                \
               dspFinancialReport.cpp dspFrozenItemSites.cpp                                          \
               dspEarnedCommissions.cpp dspExpediteExceptionsByPlannerCode.cpp                        \
               dspExpiredInventoryByClassCode.cpp                                                     \
               dspGLSeries.cpp dspGLTransactions.cpp                                                  \
               dspIndentedBOM.cpp dspIndentedWhereUsed.cpp dspInvalidBillsOfMaterials.cpp             \
               dspInventoryAvailabilityByItem.cpp dspInventoryAvailabilityByParameterList.cpp         \
               dspInventoryAvailabilityBySalesOrder.cpp dspInventoryAvailabilityBySourceVendor.cpp    \
               dspInventoryAvailabilityByWorkOrder.cpp dspInventoryAvailabilityByCustomerType.cpp     \
               dspInventoryHistoryByItem.cpp dspInventoryHistoryByOrderNumber.cpp                     \
               dspInventoryHistoryByParameterList.cpp dspIncidentsByCRMAccount.cpp                    \
               dspInventoryLocator.cpp dspInvoiceInformation.cpp                                      \
               dspItemCostDetail.cpp dspItemCostsByClassCode.cpp dspItemCostSummary.cpp               \
               dspItemCostHistory.cpp                                                                 \
               dspItemsByCharacteristic.cpp dspItemsByClassCode.cpp dspItemsByProductCategory.cpp     \
               dspItemsWithoutItemSources.cpp dspItemSitesByItem.cpp dspItemSitesByParameterList.cpp  \
               dspItemSourcesByItem.cpp dspItemSourcesByVendor.cpp dspJobCosting.cpp                  \
               dspLaborVarianceByBOOItem.cpp dspLaborVarianceByItem.cpp                               \
               dspLaborVarianceByWorkCenter.cpp dspLaborVarianceByWorkOrder.cpp                       \
               dspMaterialUsageVarianceByBOMItem.cpp dspMaterialUsageVarianceByItem.cpp               \
               dspMaterialUsageVarianceByComponentItem.cpp                                            \
               dspMaterialUsageVarianceByWorkOrder.cpp dspMaterialUsageVarianceByWarehouse.cpp        \
               dspMPSDetail.cpp dspMRPDetail.cpp openReturnAuthorizations.cpp                         \
               openSalesOrders.cpp openVouchers.cpp dspOrders.cpp dspPendingAvailability.cpp          \
               dspPendingBOMChanges.cpp dspPlannedOrdersByItem.cpp dspPlannedOrdersByPlannerCode.cpp  \
               dspPlannedRevenueExpensesByPlannerCode.cpp dspPoHistory.cpp                            \
               dspPoDeliveryDateVariancesByItem.cpp dspPoDeliveryDateVariancesByVendor.cpp            \
               dspPoItemsByDate.cpp dspPoItemsByItem.cpp dspPoItemsByVendor.cpp                       \
               dspPoItemReceivingsByDate.cpp dspPoItemReceivingsByItem.cpp                            \
               dspPoItemReceivingsByVendor.cpp                                                        \
               dspPoPriceVariancesByItem.cpp dspPoPriceVariancesByVendor.cpp dspPoReturnsByVendor.cpp \
               dspPOsByDate.cpp dspPOsByVendor.cpp                                                    \
               dspPurchaseReqsByItem.cpp dspPurchaseReqsByPlannerCode.cpp                             \
               dspQOHByParameterList.cpp dspQOHByItem.cpp dspQOHByLocation.cpp                        \
               dspQuotesByCustomer.cpp dspQuotesByItem.cpp dspOrderActivityByProject.cpp              \
               dspOperationsByWorkCenter.cpp dspPartiallyShippedOrders.cpp                            \
               dspPricesByItem.cpp dspPricesByCustomer.cpp dspPricesByCustomerType.cpp                \
               dspFreightPricesByCustomer.cpp dspFreightPricesByCustomerType.cpp                      \
               dspReorderExceptionsByPlannerCode.cpp dspRoughCutByWorkCenter.cpp                      \
               dspReservations.cpp                                                                    \
               dspRunningAvailability.cpp                                                             \
               dspSalesHistoryByBilltoName.cpp dspSalesHistoryByCustomer.cpp                          \
               dspSalesHistoryByItem.cpp dspSalesHistoryByParameterList.cpp                           \
               dspSalesHistoryBySalesrep.cpp dspSalesHistoryByShipTo.cpp                              \
               dspSalesOrderStatus.cpp dspSalesOrdersByItem.cpp dspSalesOrdersByCustomer.cpp          \
               dspSalesOrdersByCustomerPO.cpp dspSalesOrdersByParameterList.cpp                       \
               dspSequencedBOM.cpp dspSingleLevelBOM.cpp dspSingleLevelWhereUsed.cpp                  \
               dspShipmentsByDate.cpp dspShipmentsBySalesOrder.cpp dspShipmentsByShipment.cpp         \
               dspSlowMovingInventoryByClassCode.cpp                                                  \
               dspStandardJournalHistory.cpp dspStandardOperationsByWorkCenter.cpp maintainShipping.cpp \
               dspSubstituteAvailabilityByItem.cpp dspSummarizedBacklogByWarehouse.cpp                \
               dspSummarizedBankrecHistory.cpp                                                        \
               dspSummarizedBOM.cpp dspSummarizedGLTransactions.cpp                                   \
               dspSummarizedSalesByCustomer.cpp dspSummarizedSalesByCustomerByItem.cpp                \
               dspSummarizedSalesByCustomerType.cpp dspSummarizedSalesByCustomerTypeByItem.cpp        \
               dspSummarizedSalesByItem.cpp                                                           \
               dspSummarizedSalesBySalesRep.cpp dspSummarizedTaxableSales.cpp                         \
               dspSummarizedSalesHistoryByShippingZone.cpp                                            \
               dspTimePhasedAvailability.cpp dspTimePhasedAvailableCapacityByWorkCenter.cpp           \
               dspTimePhasedBookingsByCustomer.cpp dspTimePhasedBookingsByItem.cpp                    \
               dspTimePhasedBookingsByProductCategory.cpp                                             \
               dspTimePhasedCapacityByWorkCenter.cpp dspTimePhasedDemandByPlannerCode.cpp             \
               dspTimePhasedLoadByWorkCenter.cpp                                                      \
               dspTimePhasedOpenARItems.cpp dspTimePhasedOpenAPItems.cpp                              \
               dspTimePhasedRoughCutByWorkCenter.cpp dspTimePhasedPlannedREByPlannerCode.cpp          \
               dspTimePhasedProductionByPlannerCode.cpp dspTimePhasedProductionByItem.cpp             \
               dspTimePhasedSalesByCustomer.cpp dspTimePhasedSalesByCustomerGroup.cpp                 \
               dspTimePhasedSalesByCustomerByItem.cpp dspTimePhasedSalesByItem.cpp                    \
               dspTimePhasedSalesByProductCategory.cpp                                                \
               dspTimePhasedUsageStatisticsByItem.cpp dspTrialBalances.cpp                            \
               dspTodoByUserAndIncident.cpp                                                           \
               dspUnbalancedQOHByClassCode.cpp                                                        \
               dspUsageStatisticsByItem.cpp dspUsageStatisticsByClassCode.cpp                         \
               dspUsageStatisticsByItemGroup.cpp dspUsageStatisticsByWarehouse.cpp                    \
               dspUndefinedManufacturedItems.cpp                                                      \
               dspUninvoicedReceivings.cpp uninvoicedShipments.cpp unpostedPurchaseOrders.cpp         \
               dspUnusedPurchasedItems.cpp  dspVendorAPHistory.cpp dspValidLocationsByItem.cpp        \
               dspVoucherRegister.cpp  dspWoEffortByUser.cpp dspWoEffortByWorkOrder.cpp               \
               dspWoHistoryByClassCode.cpp dspWoHistoryByItem.cpp dspWoHistoryByNumber.cpp            \
               dspWoMaterialsByItem.cpp dspWoMaterialsByWorkOrder.cpp                                 \
               dspWoOperationsByWorkCenter.cpp dspWoOperationsByWorkOrder.cpp                         \
               dspWoScheduleByItem.cpp dspWoScheduleByParameterList.cpp dspWoScheduleByWorkOrder.cpp  \
               dspWoSoStatusMismatch.cpp dspWoSoStatus.cpp                                            \
               dspCapacityBufferStatusByWorkCenter.cpp dspInventoryBufferStatusByParameterList.cpp    \
               dspPoItemsByBufferStatus.cpp dspWoBufferStatusByParameterList.cpp                      \
               dspWoOperationBufrStsByWorkCenter.cpp                                                  \
               duplicateAccountNumbers.cpp                                                            \
               ediForm.cpp ediFormDetail.cpp ediProfile.cpp ediProfiles.cpp                           \
               editICMWatermark.cpp countSlip.cpp countTag.cpp                                        \
               employee.cpp employees.cpp empGroup.cpp empGroups.cpp                                  \
               enterMiscCount.cpp                                                                     \
               enterPoitemReceipt.cpp enterPoReceipt.cpp enterPoitemReturn.cpp enterPoReturn.cpp      \
               errorLog.cpp eventManager.cpp                                                          \
               expenseCategories.cpp expenseCategory.cpp expenseTrans.cpp                             \
               explodeWo.cpp                                                                          \
	       externalShipping.cpp externalShippingList.cpp failedPostList.cpp                       \
               financialLayout.cpp financialLayoutItem.cpp financialLayoutGroup.cpp financialLayouts.cpp \
               financialLayoutSpecial.cpp                           financialLayoutColumns.cpp        \
               firmPlannedOrder.cpp firmPlannedOrdersByPlannerCode.cpp fixACL.cpp fixSerial.cpp       \
               freightBreakdown.cpp freightClass.cpp freightClasses.cpp getGLDistDate.cpp             \
               getLotInfo.cpp glSeries.cpp glSeriesItem.cpp glTransaction.cpp glTransactionDetail.cpp \
               group.cpp groups.cpp                                                                   \
               honorific.cpp honorifics.cpp                                                           \
               hotkey.cpp hotkeys.cpp image.cpp imageList.cpp implodeWo.cpp                           \
               images.cpp invoiceList.cpp issueWoMaterialBatch.cpp                                    \
               importXML.cpp                                                                          \
               incident.cpp incidentWorkbench.cpp                                                     \
               incidentCategory.cpp incidentCategories.cpp incidentPriority.cpp incidentPriorities.cpp \
               incidentSeverity.cpp incidentSeverities.cpp                                            \
               incidentResolution.cpp incidentResolutions.cpp                                         \
               issueWoMaterialItem.cpp issueToShipping.cpp issueLineToShipping.cpp                    \
               item.cpp itemAlias.cpp itemCost.cpp itemAvailabilityWorkbench.cpp                      \
               itemCharacteristicDelegate.cpp                                                         \
               itemGroups.cpp itemGroup.cpp itemImages.cpp                                            \
               itemListPrice.cpp items.cpp                                                            \
               itemPricingSchedule.cpp itemPricingScheduleItem.cpp itemPricingSchedules.cpp           \
               itemSite.cpp itemSites.cpp itemSourceSearch.cpp                                        \
               itemSource.cpp itemSources.cpp itemSourceList.cpp itemSourcePrice.cpp itemSubstitute.cpp \
               itemtax.cpp itemUOM.cpp                                                                \
               forms.cpp form.cpp labelForm.cpp labelForms.cpp laborRate.cpp laborRates.cpp           \
               forwardUpdateAccounts.cpp                                                              \
               sysLocale.cpp locales.cpp location.cpp locations.cpp                                   \
               lotSerial.cpp lotSerialRegistration.cpp maintainBudget.cpp                             \
               maintainItemCosts.cpp massExpireComponent.cpp massReplaceComponent.cpp                 \
               materialReceiptTrans.cpp miscVoucher.cpp miscCheck.cpp                                 \
               mqlutil.cpp                                                                            \
               invoice.cpp invoiceItem.cpp                                                            \
               opportunity.cpp opportunityList.cpp                                                    \
               opportunitySource.cpp opportunitySources.cpp opportunityStage.cpp opportunityStages.cpp\
               opportunityType.cpp opportunityTypes.cpp                                               \
               package.cpp packages.cpp                                                               \
               packingListBatch.cpp                                                                   \
               plannedOrder.cpp plannerCodes.cpp plannerCode.cpp                                      \
               plannedSchedules.cpp plannedSchedule.cpp plannedScheduleItem.cpp                       \
               poitemTableModel.cpp poitemTableView.cpp poLiabilityDistrib.cpp                        \
               postCheck.cpp postChecks.cpp postCashReceipts.cpp                                      \
               postCostsByClassCode.cpp postCostsByItem.cpp                                           \
               postCountSlips.cpp postCountTags.cpp postBillingSelections.cpp                         \
               postCreditMemos.cpp postInvoices.cpp                                                   \
               postPurchaseOrder.cpp postPurchaseOrdersByAgent.cpp postPoReturnCreditMemo.cpp         \
               postOperations.cpp postGLTransactionsToExternal.cpp                                    \
               postProduction.cpp postMiscProduction.cpp                                              \
               postStandardJournal.cpp postStandardJournalGroup.cpp                                   \
               postVouchers.cpp prepareCheckRun.cpp priceList.cpp                                     \
               pricingScheduleAssignment.cpp pricingScheduleAssignments.cpp                           \
               printAnnodizingPurchaseRequests.cpp                                                    \
               printCheck.cpp printChecks.cpp printChecksReview.cpp                                   \
               printCreditMemo.cpp printCreditMemos.cpp reprintCreditMemos.cpp                        \
               printInvoice.cpp printInvoices.cpp printInvoicesByShipvia.cpp reprintInvoices.cpp      \
               printPackingList.cpp                                                                   \
               printItemLabelsByClassCode.cpp printLabelsByInvoice.cpp printLabelsBySo.cpp            \
               printLabelsByOrder.cpp printLabelsByTo.cpp printPackingListBatchByShipvia.cpp printPoForm.cpp \
               printProductionEntrySheet.cpp printPurchaseOrder.cpp                                   \
               printPurchaseOrdersByAgent.cpp printSASpecialCalendarForm.cpp printSoForm.cpp          \
               printRaForm.cpp                                                                        \
               printShippingForm.cpp printShippingForms.cpp                                           \
               printStatementByCustomer.cpp printStatementsByCustomerType.cpp                         \
               printVendorForm.cpp                                                                    \
               printWoForm.cpp printWoPickList.cpp printWoRouting.cpp printWoTraveler.cpp             \
               printJournal.cpp                                                                       \
               productCategory.cpp productCategories.cpp profitCenter.cpp profitCenters.cpp           \
               project.cpp projects.cpp purchaseOrder.cpp purchaseOrderItem.cpp                       \
               prospect.cpp prospects.cpp                                                             \
               purchaseRequest.cpp purgeClosedWorkOrders.cpp purgeCreditMemos.cpp purgeInvoices.cpp   \
               purgePostedCounts.cpp purgePostedCountSlips.cpp                                        \
               purgeShippingRecords.cpp quotes.cpp                                                    \
               rate.cpp reasonCode.cpp reasonCodes.cpp reassignLotSerial.cpp recallOrders.cpp         \
               rejectCodes.cpp rejectCode.cpp                                                         \
               reassignClassCodeByClassCode.cpp reassignCustomerTypeByCustomerType.cpp                \
               reassignProductCategoryByProductCategory.cpp reconcileBankaccount.cpp                  \
               registration.cpp                                                                       \
               releasePlannedOrdersByPlannerCode.cpp releaseWorkOrdersByPlannerCode.cpp               \
               relativeCalendarItem.cpp relocateInventory.cpp editOwners.cpp                          \
               reports.cpp reprioritizeWo.cpp                                                         \
               reschedulePoitem.cpp  rescheduleSoLineItems.cpp rescheduleWo.cpp                       \
               reserveSalesOrderItem.cpp                                                              \
               resetQOHBalances.cpp returnAuthCheck.cpp returnAuthorization.cpp                       \
               returnAuthorizationItem.cpp returnAuthItemLotSerial.cpp                                \
               returnAuthorizationWorkbench.cpp returnWoMaterialBatch.cpp returnWoMaterialItem.cpp    \
               reverseGLSeries.cpp                                                                    \
               runMPSByPlannerCode.cpp                                                                \
               sales.cpp sale.cpp salesAccount.cpp salesAccounts.cpp salesCategories.cpp salesCategory.cpp \
               salesOrder.cpp salesOrderInformation.cpp salesOrderItem.cpp                            \
               salesReps.cpp salesRep.cpp                                                             \
               salesHistoryInformation.cpp scrapTrans.cpp scrapWoMaterialFromWIP.cpp                  \
               scriptEditor.cpp scripts.cpp                                                           \
               scriptquery.cpp scripttoolbox.cpp                                                      \
               searchForCRMAccount.cpp searchForContact.cpp searchForItem.cpp searchForEmp.cpp        \
               selectBankAccount.cpp selectBillingQty.cpp selectOrderForBilling.cpp                   \
               selectedPayments.cpp selectPayment.cpp selectPayments.cpp                              \
               selectShippedOrders.cpp shift.cpp shifts.cpp shipOrder.cpp shippingInformation.cpp     \
               shippingForm.cpp shippingForms.cpp shipTo.cpp                                          \
               shippingChargeType.cpp shippingChargeTypes.cpp                                         \
               shippingZones.cpp shippingZone.cpp                                                     \
               shipVias.cpp shipVia.cpp siteType.cpp siteTypes.cpp splitReceipt.cpp                   \
               standardOperations.cpp standardOperation.cpp standardJournal.cpp standardJournals.cpp  \
               standardJournalGroup.cpp standardJournalGroupItem.cpp standardJournalGroups.cpp        \
               standardJournalItem.cpp subaccount.cpp subaccounts.cpp subAccntTypes.cpp subAccntType.cpp \
               submitAction.cpp submitReport.cpp substituteList.cpp summarizeInvTransByClassCode.cpp  \
               syncCompanies.cpp                                                                      \
               systemMessage.cpp                                                                      \
               taxAuthorities.cpp taxAuthority.cpp taxCodes.cpp taxCode.cpp taxDetail.cpp             \
               taxBreakdown.cpp                                                                       \
               taxRegistration.cpp taxRegistrations.cpp                                               \
               taxSelection.cpp taxSelections.cpp                                                     \
               taxType.cpp taxTypes.cpp                                                               \
               task.cpp thawItemSitesByClassCode.cpp                                                  \
               termses.cpp terms.cpp                                                                  \
               todoList.cpp todoItem.cpp todoListCalendar.cpp todoCalendarControl.cpp                 \
               toitemTableModel.cpp toitemTableView.cpp                                               \
               transactionInformation.cpp transferOrder.cpp transferOrders.cpp                        \
               transferOrderItem.cpp                                                                  \
               transferTrans.cpp                                                                      \
               transformTrans.cpp                                                                     \
               uiform.cpp uiforms.cpp                                                                 \
               unappliedARCreditMemos.cpp unappliedAPCreditMemos.cpp                                  \
               unpostedCreditMemos.cpp                                                                \
               unpostedGLTransactions.cpp unpostedInvoices.cpp                                        \
               unpostedPoReceipts.cpp unpostedGlSeries.cpp                                            \
               uom.cpp uoms.cpp uomConv.cpp                                                           \
               updateABCClass.cpp updateActualCostsByClassCode.cpp updateActualCostsByItem.cpp        \
               updateCreditStatusByCustomer.cpp updateCycleCountFrequency.cpp                         \
               updateItemSiteLeadTimes.cpp updateListPricesByProductCategory.cpp                      \
               updateLateCustCreditStatus.cpp                                                         \
               updateOUTLevelByItem.cpp updateOUTLevels.cpp updateOUTLevelsByClassCode.cpp            \
               updatePricesByProductCategory.cpp  updatePricesByPricingSchedule.cpp                   \
               updateReorderLevels.cpp								      \
               users.cpp user.cpp userList.cpp userPreferences.cpp                                    \
               userCostingElement.cpp costingElements.cpp                                             \
               version.cpp vendor.cpp vendors.cpp vendorWorkBench.cpp                                 \
               vendorAddress.cpp vendorAddressList.cpp                                                \
               vendorType.cpp vendorTypes.cpp viewCheckRun.cpp voidChecks.cpp                         \
               voucher.cpp voucheringEditList.cpp voucherItemDistrib.cpp voucherItem.cpp              \
               voucherMiscDistrib.cpp                                                                 \
               warehouses.cpp warehouse.cpp warehouseZone.cpp                                         \
               whseCalendar.cpp whseCalendars.cpp whseWeek.cpp workCenter.cpp workCenters.cpp         \
               workOrder.cpp workOrderMaterials.cpp woTimeClock.cpp wotc.cpp                          \
               woOperation.cpp workOrderOperations.cpp                                                \
               zeroUncountedCountTagsByWarehouse.cpp                                                  \
	       creditcardprocessor.cpp authorizedotnetprocessor.cpp verisignprocessor.cpp             \
	       yourpayprocessor.cpp externalccprocessor.cpp externalCCTransaction.cpp                 \
               xmainwindow.cpp xdialog.cpp xuiloader.cpp xwidget.cpp                                  \
               idleShutdown.cpp xdateinputdialog.cpp xsltMap.cpp                                            \
               xTupleDesigner.cpp xTupleDesignerActions.cpp

QT += xml sql script network

RESOURCES += guiclient.qrc

