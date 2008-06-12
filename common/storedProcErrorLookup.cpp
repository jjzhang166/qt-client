/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include <QMessageBox>
#include <QMultiHash>
#include <QObject>
#include <QPair>
#include <QString>

#define TR(a)	QObject::tr(a)

/*	try to address bug 4218
  This code assumes that stored procedures
  return zero or positive integers on success
  return negative integers on failure
 */

QMultiHash< QString, QPair<int, QString> >	ErrorLookupHash;

/*
  developers add error messages to an array for ease of adding new ones.
  initErrorLookupHash then inserts these into a QMultiHash(key, value).
  The key is the stored procedure name.
  The value is a QPair* of (return value, error message).
*/

void initErrorLookupHash()
{
  struct {
  QString	procName;	// name of the stored procedure
  int		retVal;		// return value from the stored procedure
  QString	msg;		// msg to display, but see msgPtr and proxyName
  int		msgPtr;		// if <> 0 then look up (procName, msgPtr)
  QString	proxyName;	// look up (proxyName, retVal)
} errors[] = {
  { "changeCMHeadTaxAuth", -1, TR("This Credit Memo was not found."),	0, "" },
  { "changeCMHeadTaxAuth", -2, TR("This Tax Authority was not found."),	0, "" },

  { "changeInvoiceTaxAuth", -1, TR("This Invoice was not found."),	0, "" },
  { "changeInvoiceTaxAuth", -2, TR("This Tax Authority was not found."),0, "" },

  { "changeQuoteTaxAuth", -1, TR("This Quote was not found."),		0, "" },
  { "changeQuoteTaxAuth", -2, TR("This Tax Authority was not found."),	0, "" },

  { "changeSOTaxAuth", -1, TR("This Sales Order was not found."),	0, "" },
  { "changeSOTaxAuth", -2, TR("This Tax Authority was not found."),	0, "" },
  { "changeTOTaxAuth", -1, TR("This Transfer Order was not found."),	0, "" },
  { "changeTOTaxAuth", -2, TR("This Tax Authority was not found."),	0, "" },

  { "closeAccountingPeriod", -1, TR("The selected Accounting Period cannot be "
				    "closed because it is already closed."),
									0, "" },
  { "closeAccountingPeriod", -2, TR("The selected Accounting Period cannot be "
				    "closed because there is a gap between the "
				    "end of the previous Period and the start "
				    "of this Period. You must edit either the "
				    "previous Perod or this Period to "
				    "eliminate the gap."),		0, "" },
  { "closeAccountingPeriod", -3, TR("The selected Accounting Period cannot be "
				    "closed because the previous Period is not "
				    "closed. You must close the previous Period"
				    " before you may close this Period."),
									0, "" },
  { "closeAccountingPeriod", -4, TR("The selected Accounting Period cannot be "
				    "closed because there is a gap between the "
				    "end of this Period and the start of the "
				    "next Period. You must edit either this "
				    "Period or the next Period to eliminate "
				    "the gap."),			0, "" },
  { "closeAccountingPeriod", -5, TR("The selected Accounting Period cannot be "
				    "closed because it ends in the future."),
									0, "" },
  { "closeAccountingPeriod", -6, TR("The selected Accounting Period cannot be "
				    "closed because it is the last period in "
				    "the Fiscal Year and the next Fiscal Year "
				    "has not been defined yet. Create the "
				    "next Fiscal Year before closing this "
				    "Accounting Period."),		0, "" },

  { "closeAccountingYearPeriod", -1, "", -1, "closeAccountingPeriod" },
  { "closeAccountingYearPeriod", -2, "", -2, "closeAccountingPeriod" },
  { "closeAccountingYearPeriod", -3, "", -3, "closeAccountingPeriod" },
  { "closeAccountingYearPeriod", -4, "", -4, "closeAccountingPeriod" },
  { "closeAccountingYearPeriod", -5, "", -5, "closeAccountingPeriod" },
  { "closeAccountingYearPeriod", -6, "", -6, "closeAccountingPeriod" },
  { "closeAccountingYearPeriod", -7, TR("The selected Fiscal Year cannot be closed "
                                        "because you have not specified a Year End Equity Account "
                                        "in the accounting configuration."), 0, "" },
  { "closeAccountingYearPeriod", -8, TR("The selected Fiscal Year cannot be "
                                        "closed because there does not seem to "
                                        "be an Accounting Period defined for "
                                        "the beginning of the next Fiscal "
                                        "Year."),                       0, "" },
  { "closeAccountingYearPeriod", -9, TR("The selected Fiscal Year cannot be closed "
                                        "because there is no Trial Balance record for "
                                        "the account in the required Period. Or you have "
                                        "not specified a Year End Equity Account in the "
                                        "accounting configuration."), 0, "" },

  { "closeToItem",	 -1, TR("The item cannot be Closed at this time "
				"as there is inventory at shipping."),	0, "" },

  { "convertCustomerToProspect",	-1, "", -1, "deleteCustomer" },
  { "convertCustomerToProspect",	-2, "", -2, "deleteCustomer" },
  { "convertCustomerToProspect",	-3, "", -3, "deleteCustomer" },
  { "convertCustomerToProspect",	-4, "", -4, "deleteCustomer" },
  { "convertCustomerToProspect",	-5, "", -5, "deleteCustomer" },
  { "convertCustomerToProspect",
		       -10, TR("Could not convert Customer to Prospect to "
			       "because there is already a Prospect with "
			       "this internal ID."),		   0, "" },

  { "convertProspectToCustomer",	-1, "", -1, "deleteProspect" },
  { "convertProspectToCustomer",
		       -10, TR("Could not convert Prospect to Customer "
			       "because there is already a Customer with "
			       "this internal ID."),		   0, "" },

  { "convertQuote", -1, TR("Quote #%1 has one or more line items without a "
			   "warehouse specified. These line items must be "
			   "fixed before you may convert this quote." ),
								   0, "" },
  { "convertQuote", -2, TR("Cannot find the Customer data for Quote #%1."),
								   0, "" },
  { "convertQuote", -3, TR("Quote #%1 is associated with a Prospect, "
			       "not a Customer. Convert the Prospect to a "
			       "Customer first."),		   0, "" },
  { "convertQuote", -4, TR("Quote #%1 is for a Customer that has been "
			   "placed on a Credit Hold and you do not have "
			   "privilege to create Sales Orders for Customers on "
			   "Credit Hold.  The selected Customer must be taken "
			   "off of Credit Hold before you may create convert "
			   "this Quote."),			   0, "" },

  { "convertQuote", -5, TR("Quote #%1 is for a Customer that has been "
			   "placed on a Credit Warning and you do not have "
			   "privilege to create Sales Orders for Customers on "
			   "Credit Warning.  The selected Customer must be "
			   "taken off of Credit Warning before you may create "
			   "convert this Quote."),		   0, "" },
  { "convertQuote", -6, TR("Quote #%1 has expired and can not be converted."),
								   0, "" },

  { "copyItemSite",  -1, TR("Could not copy the Item Site because it "
			    "does not appear to exist."),		0, "" },
  { "copyItemSite",  -2, TR("Could not copy the Item Site because the "
			    "warehouse for the new Item Site record "
			    "does not appear to exist."),		0, "" },
  { "copyItemSite",  -3, TR("You do not have sufficient privilege to "
			    "create an Item Site."),			0, "" },

  { "copyPO", -1, TR("Could not find the P/O to copy."),		0, "" },
  { "copyPO", -2, TR("The Vendor of the original P/O does not match "
		     "the Vendor for the copy. Changing the Vendor "
		     "is not yet supported when copying a P/O."),	0, "" },
  { "copyPO", -3, TR("The system does not allow purchases of Items for this "
		     "Vendor without Item Sources and at least one line item "
		     "item in the original P/O does not have an active "
		     "Item Source."),					0, "" },
  { "copyPO", -4, TR("At least one line item in the original P/O does not have "
		     "an active Item Source Price for this Vendor."),	0, "" },
  { "correctReceipt",  -12, TR("The receipt has been split and may not be corrected. "
				      "Correct Receipt."),	0, "" },
  { "createAccountingPeriod",  -1, TR("The Start Date falls within another "
				      "Accounting Period."),	0, "" },
  { "createAccountingPeriod",  -2, TR("The End Date falls within another "
				      "Accounting Period."),	0, "" },
  { "createAccountingPeriod",  -3, TR("The Start and End Dates enclose another "
				      "Accounting Period."),	0, "" },
  { "createAccountingPeriod",  -4, TR("The Period dates are outside the "
				      "selected Fiscal Year."),	0, "" },

  { "createAPCreditMemoApplication",
			-1, TR("You may not apply more than the balance due "
			       "to this document."),		0, "" },
  { "createAPCreditMemoApplication",
			-2, TR("You may not apply more than the amount "
			       "available to apply for this Credit Memo."),
								0, "" },

  { "createARCreditMemo", -1, TR("Either the Prepaid Account or the A/R Account"
				 " for this Customer could not be found."),
								0, "" },

  { "createARDebitMemo", -1, "",		-1, "createARCreditMemo" },

  { "createBOMItem",	 -1, TR("You may not create a BOM Item that defines a "
				"Parent that is composed of itself."),	0, "" },
  { "createBOMItem",	 -2, TR( "The Component that you have selected for this"
				 "BOM Item is a manufactured or phantom Item "
				 "that uses the Parent Item as a Component "
				 "Item in its own BOM. You may not create a "
				 "recursive BOM."),			0, "" },

  { "createCrmAcct",	-1, TR("This CRM Account Number is already in use by "
			       "an existing CRM Account. Please choose a "
			       "different number and save again."),	0, "" },
  { "createCrmAcct",	-2, TR("This CRM Account Number is already in use by "
			       "an existing Customer. Please choose a different"
			       " number and save again."),		0, "" },
  { "createCrmAcct",	-5, TR("This CRM Account Number is already in use by "
			       "an existing Prospect. Please choose a different"
			       " number and save again."),		0, "" },
  { "createCrmAcct",	-6, TR("This CRM Account Number is already in use by "
			       "an existing Vendor. Please choose a different"
			       " number and save again."),		0, "" },
  { "createCrmAcct",	-7, TR("This CRM Account Number is already in use by "
			       "an existing Tax Authority. Please choose a "
			       "different number and save again."),	0, "" },

  { "createProspect",	 -1, TR("Cannot create a Prospect because there is no "
				"CRM Account to tie it to."),
								0, "" },
  { "createProspect",	 -2, TR("Cannot create a Prospect for this CRM Account "
				"because it is already a Customer."),
								0, "" },
  { "createProspect",    -3, TR("Cannot create a Prospect for this CRM Account "
				"because it is already a Prospect."),
								0, "" },
  { "CreateRevision", -2, TR("Revision control not enabled."),	0, "" },
  { "createTodoItem",	-1, TR("The To-Do List Item cannot be created as "
			       "there is no assigned User."),	0, "" },
  { "createTodoItem",	-2, TR("The To-Do List Item cannot be created as "
			       "the Task Name is blank."),	0, "" },
  { "createTodoItem",	-3, TR("The To-Do List Item cannot be created as "
			       "there is no Due Date."),	0, "" },
  { "createWo",	-2, TR("Work Order can not be exploded because items on "
			       "the BOM exist without itemsites."),	0, "" }, 

  { "deleteAccount",  -1, TR("The selected G/L Account cannot be deleted as it "
                             "is currently used in one or more Cost Categories."
                             "  You must reassign these Cost Category "
                             "assignments before you may delete the selected "
                             "G/L Account."),                           0, "" },
  { "deleteAccount",  -2, TR("The selected G/L Account cannot be deleted as it "
                             "is currently used in one or more Sales Account "
                             "Assignment. You must reassign these Sales "
                             "Account Assignments before you may delete "
                             "the selected G/L Account."),              0, "" },
  { "deleteAccount",  -3, TR("The selected G/L Account cannot be deleted as it "
                             "is currently used in one or more Customer A/R "
                             "Account assignments. You must reassign these "
                             "Customer A/R Account assignments before you may "
                             "delete the selected G/L Account."),       0, "" },
  { "deleteAccount",  -4, TR("The selected G/L Account cannot be deleted as it "
                             "is currently used as the default Account one or "
                             "more Warehouses. You must reassign the default "
                             "Account for these Warehouses before you may delete "
                             "the selected G/L Account."),              0, "" },
  { "deleteAccount",  -5, TR("The selected G/L Account cannot be deleted as it "
                             "is currently used in one or more Bank Accounts. "
                             "You must reassign these Bank Accounts before you "
                             "may delete the selected G/L Account."),   0, "" },
  { "deleteAccount",  -6, TR("The selected G/L Account cannot be deleted as it "
                             "is currently used in one or more Expense "
                             "Categories. You must reassign these Expense "
                             "Categories before you may delete the selected "
                             "G/L Account."),                           0, "" },
  { "deleteAccount",  -7, TR("The selected G/L Account cannot be deleted as it "
                             "is currently used in one or more Tax Codes. "
                             "You must reassign these Tax Codes before you "
                             "may delete the selected G/L Account."),   0, "" },
  { "deleteAccount",  -8, TR("The selected G/L Account cannot be deleted as it "
                             "is currently used in one or more Standard "
                             "Journals. You must reassign these Standard "
                             "Journal Items before you may delete the selected "
                             "G/L Account."),                           0, "" },
  { "deleteAccount",  -9, TR("The selected G/L Account cannot be deleted as it "
                             "is currently used in one or more Customer A/P "
                             "Account assignments. You must reassign these "
                             "Customer A/P Account assignments before you may "
                             "delete the selected G/L Account."),       0, "" },
  { "deleteAccount", -10, TR("The selected G/L Account cannot be deleted as it "
                             "is currently used in one or more Currency "
                             "definition. You must reassign these Currency "
                             "definitions before you may delete the selected "
                             "G/L Account."),                           0, "" },
  { "deleteAccount", -11, TR("The selected G/L Account cannot be deleted as it "
                             "is currently used in one or more A/R Open Items. "
                             "You must reassign these Currency definitions "
                             "before you may delete the selected G/L Account."),
                                                                        0, "" },
  { "deleteAccount", -99, TR("The selected G/L Account cannot be deleted as "
                             "there have been G/L Transactions posted "
                             "against it."),                            0, "" },

  { "deleteAccountingPeriod", -4, TR("The selected Accounting Period has G/L "
				     "Transactions posted against it and, thus, "
				     "cannot be deleted."), 0, "" },

  { "deleteAccountingYearPeriod", -1, TR("The selected Fiscal Year cannot be "
					 "deleted because it is closed."),
									0, "" },
  { "deleteAccountingYearPeriod", -2, TR("The selected Fiscal Year cannot be "
					 "deleted because there are Accounting "
					 "Periods defined for it."),	0, "" },

  { "deleteAddress",	-1, TR("The selected Address cannot be deleted as "
			       "it is used by an active Contact."),
								 0, "" },
  { "deleteAddress",	-2, TR("The selected Address cannot be deleted as "
			       "it is used by an active Vendor."),
								 0, "" },
  { "deleteAddress",	-3, TR("The selected Address cannot be deleted as "
			       "it is used by an active Ship-To Address."),
								 0, "" },
  { "deleteAddress",	-4, TR("The selected Address cannot be deleted as "
			       "it is used by an active Vendor Address."),
								 0, "" },
  { "deleteAddress",	-5, TR("The selected Address cannot be deleted as "
			       "it is used by an active Warehouse."),
			       					 0, "" },

  { "deleteBankAdjustmentType",  -1, TR("The selected Bank Adjustment Type "
                                        "cannot be deleted because it is "
                                        "currently used by a Bank Adjustment."),
                                                                 0, "" },

  { "deleteCashrcpt",  -1, TR("The selected Cash Receipt cannot be deleted "
                              "because it is a Customer Deposit made with "
                              "a Credit Card and the card has already been "
                              "charged."),                              0, "" },
  { "deleteCharacteristic",
			-1, TR("The selected Characteristic cannot be deleted "
			       "because there are Items assigned to "
			       "it. You must remove these assignments before "
			       "you may delete the selected Characteristic."),
			       					 0, "" },
  { "deleteCharacteristic",
			-2, TR("The selected Characteristic cannot be deleted "
			       "because there are Customers assigned to "
			       "it. You must remove these assignments before "
			       "you may delete the selected Characteristic."),
			       					 0, "" },
  { "deleteCharacteristic",
			-3, TR("The selected Characteristic cannot be deleted "
			       "because there are Addresses assigned to "
			       "it. You must remove these assignments before "
			       "you may delete the selected Characteristic."),
			       					 0, "" },
  { "deleteCharacteristic",
			-4, TR("The selected Characteristic cannot be deleted "
			       "because there are Contacts assigned to "
			       "it. You must remove these assignments before "
			       "you may delete the selected Characteristic."),
			       					 0, "" },
  { "deleteCharacteristic",
			-5, TR("The selected Characteristic cannot be deleted "
			       "because there are CRM Accounts assigned to "
			       "it. You must remove these assignments before "
			       "you may delete the selected Characteristic."),
			       					 0, "" },

  { "deleteContact",	-1, TR("The selected Contact cannot be deleted as "
			       "s/he is the primary or secondary Contact "
			       " for a CRM Account."),		 0, "" },
  { "deleteContact",	-2, TR("The selected Contact cannot be deleted as "
			       "s/he is the Correspondence or Billing "
			       "Contact for a Customer."),	 0, "" },
  { "deleteContact",	-3, TR("The selected Contact cannot be deleted as "
			       "s/he is the primary or secondary "
			       "Contact for a Vendor."),	 0, "" },
  { "deleteContact",	-4, TR("The selected Contact cannot be deleted as "
			       "s/he is the Contact for a Ship-To Address."),
								 0, "" },
  { "deleteContact",	-5, TR("The selected Contact cannot be deleted as "
			       "s/he is the Contact for a Vendor Address."),
			       					 0, "" },
  { "deleteContact",	-6, TR("The selected Contact cannot be deleted as "
			       "s/he is the Contact for a Warehouse."),
								 0, "" },

  { "deleteCRMAccount",	-1, TR("The selected CRM Account cannot be deleted as "
			       "it is a Customer."),		 0, "" },
  { "deleteCRMAccount",	-2, TR("The selected CRM Account cannot be deleted as "
			       "it is a Vendor."),		 0, "" },
  { "deleteCRMAccount",	-3, TR("The selected CRM Account cannot be deleted as "
			       "it is a Prospect."),		 0, "" },
  { "deleteCRMAccount",	-4, TR("The selected CRM Account cannot be deleted as "
			       "it has Contacts. You may Detach the Contacts "
			       "from this CRM Account and try deleting it "
			       "again or set its status to inactive"),
								 0, "" },
  { "deleteCRMAccount",	-5, TR("The selected CRM Account cannot be deleted as "
			       "it is a Tax Authority."),	 0, "" },

  { "deleteCustomer",	-1, TR("The selected Customer cannot be deleted as there "
			       "are still Ship-Tos assigned to it. You must "
			       "delete all of the selected Customer's Ship-Tos "
			       "before you may delete it."),	 0, "" },
  { "deleteCustomer",	-2, TR("The selected Customer cannot be deleted "
			       "as there has been Sales History recorded "
			       "for this Customer. You may Edit the "
			       "selected Customer and set its status to "
			       "inactive."),			 0, "" }, //Purchase Orders
  { "deleteCustomer",	-3, QString("Credit Memos"),		-2, "" },
  { "deleteCustomer",	-4, QString("custhist"),		-2, "" },
  { "deleteCustomer",	-5, QString("A/R Open"),		-2, "" },
  { "deleteCustomer",	-6, TR("The selected Customer cannot be deleted as "
			       "Checks have been written to it."),	0, "" },
  { "deleteCustomer",   -7, TR("The selected Customer cannot be deleted as there "
                               "are still Invoices assigned to it. You must "
                               "delete all of the selected Customer's Invoices "
                               "before you may delete it"),      0, "" },
  { "deleteCustomer",   -8, TR("The selected Customer cannot be deleted as there "
                               "are still Quotes assigned to it. You must "
                               "delete all of the selected Customer's Quotes "
                               "before you may delete it"),      0, "" },

  { "deleteIncident",	-1, TR("This Incident cannot be deleted as there are "
			       "To-Do List Items associated with it."),
								 	0, "" },
  { "deleteIncident",	-2, TR("This Incident cannot be deleted as there are "
			       "Comments associated with it."), 	0, "" },
  { "deleteIncident",	-1, TR("This Incident cannot be deleted as there are "
			       "To-Do List Items associated with it."), 	0, "" },
  { "deleteItem",	-1, TR("This Item cannot be deleted as it is used "
			       "in one or more bills of materials."), 	0, "" },
  { "deleteItem",	-2, TR("This Item cannot be deleted as there are "
			       "Item Site records associated with it."), 	0, "" },
  { "deleteItem",	-3, TR("This Item cannot be deleted as there are "
			       "Substitute records associated with it."), 	0, "" },
  { "deleteItem",	-4, TR("This Item cannot be deleted as there are "
			       "Breeder BOM records associated with it."), 	0, "" },
  { "deleteItem",	-5, TR("This Item cannot be deleted as there are "
			       "assignement records associated with it."), 	0, "" },
  { "deleteItem",	-6, TR("This Item cannot be deleted as there are "
			       "Revision Control records associated with it."), 	0, "" },
  { "deleteItemSite",  -1, TR("The selected Item Site cannot be deleted as "
			      "there is Inventory History posted against it. "
			      "You may edit the Item Site and deactivate it."),	0, "" },
  { "deleteItemSite",  -2, TR("The selected Item Site cannot be deleted as "
			      "there is Work Order History posted against it. "
			      "You may edit the Item Site and deactivate it."),	0, "" },
  { "deleteItemSite",  -3, TR("The selected Item Site cannot be deleted as "
			      "there is Sales History posted against it. "
			      "You may edit the Item Site and deactivate it."),	0, "" },
  { "deleteItemSite",  -4, TR("The selected Item Site cannot be deleted as "
			      "there is Purchasing History posted against it. "
			      "You may edit the Item Site and deactivate it."),	0, "" },
  { "deleteItemSite",  -5, TR("The selected Item Site cannot be deleted as "
			      "there is Planning History posted against it. "
			      "You may edit the Item Site and deactivate it."),	0, "" },
  { "deleteItemSite",  -9, TR("The selected Item Site cannot be deleted as "
			      "there is a non-zero Inventory Quantity posted "
			      "against it."),	0, "" },

  { "deleteItemUOMConv", -1, TR("This UOM Conversion cannot be deleted as "
			       "there are records for this Item which use this "
                               "UOM."), 	                        0, "" },

  { "deleteProspect",	-1, TR("The selected Prospect cannot be deleted as "
			       "there are still Quotes for it. You must delete "
			       "all of this Prospect's Quotes before you may "
			       "delete the Prospect."),		 0, "" },
  { "deleteSalesRep",  -1, TR("The selected Sales Rep. cannot be deleted as "
                              "he/she is still assigned to one or more "
                              "Customers.  You must reassign different Sales "
                              "Reps. to all Customers to which the selected "
                              "Sales Rep. is assigned before you may delete "
                              "the selected Sales Rep." ),      0, "" },
  { "deleteSalesRep",  -2, TR("The selected Sales Rep. cannot be deleted as "
                              "he/she is still assigned to one or more "
                              "Ship-tos.  You must reassign different Sales "
                              "Reps. to all Ship-tos to which the selected "
                              "Sales Rep. is assigned before you may delete "
                              "the selected Sales Rep." ),      0, "" },
  { "deleteSalesRep",  -3, TR("The selected Sales Rep. cannot be deleted as "
                              "there has been sales history recorded against "
                              "him/her.  You may edit and set the selected "
                              "Sales Rep's active status to inactive." ),
                                                                0, "" },

  { "deleteShipto",	-1, TR("The selected Shipto cannot be deleted as there "
			       "is still Archived Sales History assigned to it. You must "
			       "delete all of the selected Customer's Ship-Tos "
			       "before you may delete it."),	 0, "" },
  { "deleteShipto",	-2, TR("The selected Shipto cannot be deleted "
			       "as there has been Sales History recorded "
			       "for this Shipto. You may Edit the "
			       "selected Shipto and set its status to "
			       "inactive."),			 0, "" },
  { "deleteShipto",	-3, QString("The selected Shipto cannot be deleted "
			       "as there has been Credit Memos recorded "
			       "for this Shipto. You may Edit the "
			       "selected Shipto and set its status to "
			       "inactive."),		-2, "" },
  { "deleteShipto",	-4, QString("The selected Shipto cannot be deleted "
			       "as there has been Sales History recorded "
			       "for this Shipto. You may Edit the "
			       "selected Shipto and set its status to "
			       "inactive."),		-2, "" },
  { "deleteShipto",	-5, QString("The selected Shipto cannot be deleted "
			       "as there has been Quote History recorded "
			       "for this Shipto. You may Edit the "
			       "selected Shipto and set its status to "
			       "inactive."),		-2, "" },
  { "deleteShipto",	-6, QString("The selected Shipto cannot be deleted "
			       "as there has been Invoice History recorded "
			       "for this Shipto. You may Edit the "
			       "selected Shipto and set its status to "
			       "inactive."),			 0, "" },
  { "deleteSO",  -1, TR("This Sales Order cannot be deleted as some of its "
			"line items have already been shipped."),	0, "" },
  { "deleteSO",  -2, TR("This Sales Order cannot be deleted as some of its "
			"line items have already been issued to shipping.  "
			"You must return this stock before "
		        "you may delete this Sales Order."),		0, "" },

  /* uncomment this when releaseSoNumber returns INTEGER instead of BOOLEAN
  { "deleteSO",  -3, TR("This Sales Order cannot be deleted because there was "
			"an internal error releasing the Sales Order Number."),
									0, "" },
  */

  { "deleteSO",	 -4, TR("This Sales Order cannot be deleted because a Credit "
			"Card has been charged for it."),		0, "" },
  { "deleteSO",	 -5, TR("This Sales Order cannot be deleted because there is "
			"Credit Card transaction history for it."),	0, "" },

  { "deleteTO",		-1, TR("This Transfer Order cannot be deleted as some of "
			       "its line items have already been shipped."),
								 0, "" },
  { "deleteTO",		-2, TR("This Transfer Order cannot be deleted as some of "
			       "its line items have already been issued to "
			       "shipping. You must return this stock before "
			       "you may delete this Transfer Order."),
			       					 0, "" },

  { "deleteTax", -10,	TR("This Tax Code cannot be deleted as there are "
			   "Tax Selections that refer to it. Change those "
			   "Tax Selections before trying to delete this "
			   "Tax Code."),				0, "" },
  { "deleteTax", -13,	TR("This Tax Code cannot be deleted as there are "
			   "Vouchers that refer to it."),		0, "" },
  { "deleteTax", -14,	TR("This Tax Code cannot be deleted as there are "
			   "Voucher Items that refer to it."),		0, "" },
  { "deleteTax", -18,	TR("This Tax Code cannot be deleted as there are "
			   "Quote Items that refer to it."),		0, "" },
  { "deleteTax", -20,	TR("This Tax Code cannot be deleted as there are "
			   "Sales Order Items that refer to it."),	0, "" },
  { "deleteTax", -21,	TR("This Tax Code cannot be deleted as there are "
			   "Billing Selections that refer to it."),	0, "" },
  { "deleteTax", -22,	TR("This Tax Code cannot be deleted as there are "
			   "Billing Selection Items that refer to it."),0, "" },
  { "deleteTax", -23,	TR("This Tax Code cannot be deleted as there are "
			   "Invoices that refer to it."),		0, "" },
  { "deleteTax", -24,	TR("This Tax Code cannot be deleted as there are "
			   "Invoice Items that refer to it."),		0, "" },
  { "deleteTax", -25,	TR("This Tax Code cannot be deleted as there are "
			   "S/O Credit Memos that refer to it."),	0, "" },
  { "deleteTax", -26,	TR("This Tax Code cannot be deleted as there are "
			   "S/O Credit Memo Items that refer to it."),	0, "" },

  { "deleteTaxAuthority", -1, TR("This Tax Authority cannot be deleted as "
				 "there are Tax Selections for it. Change or "
				 "delete those Tax Selections before deleting "
				 "this Tax Authority."),		0, "" },
  { "deleteTaxAuthority", -7, TR("This Tax Authority cannot be deleted as "
				 "Checks have been written to it."),	0, "" },

  { "deleteTo",	 -1,	TR("This Transfer Order cannot be deleted as line "
			   "items for it have already been shipped."),	0, "" },
  { "deleteTo",	 -2,	TR("This Transfer Order cannot be deleted as line "
			   "items for it have been issued to shipping."),0, "" },
  { "deleteTo",	 -3,	TR("This Transfer Order cannot be deleted as the "
			   "order number cannot be released."),		0, "" },

  { "deleteVendor",	-1, TR("The selected Vendor cannot be deleted as there"
			       "have been P/Os created against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },
  { "deleteVendor",	-2, TR("The selected Vendor cannot be deleted as there"
			       "has been P/O Material Receipt History posted "
			       "against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },
  { "deleteVendor",	-3, TR("The selected Vendor cannot be deleted as there"
			       "has been P/O Material Return History posted "
			       "against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },
  { "deleteVendor",	-4, TR("The selected Vendor cannot be deleted as there"
			       "have been Vouchers posted against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },
  { "deleteVendor",	-5, TR("The selected Vendor cannot be deleted as there"
			       "have been A/P Open Items posted against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },
  { "deleteVendor",	-6, TR("The selected Vendor cannot be deleted as there"
			       "have been A/P Applications posted against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },
  { "deleteVendor",	-7, TR("The selected Vendor cannot be deleted as there "
			       "have been Checks posted against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },

  { "deleteWo",		-1, TR("The Work Order cannot be deleted because time "
			       "clock entries exist for it. Please Close it "
			       "instead of trying to Delete it."),
								 0, "" },

  { "deleteWorkCenter",  -1, TR("The selected Work Center cannot be deleted "
				"because there has been history posted against "
				"it."),	0, "" },
  { "deleteWorkCenter",  -2, TR("The selected Work Center cannot be deleted "
				"because Standard Operations exist that use "
				"it. You must reassign all Standard Operations "
				"that use the selected Work Center to a "
				"different Work Center before you may delete "
				"it."),					0, "" },

  { "deleteWorkCenter",  -3, TR("The selected Work Center cannot be deleted "
				"because Bill of Operations items exist that "
				"use it. You must reassign all Bill of "
				"Operations items that use the selected Work "
				"Center to a different Work Center before you "
				"may delete it." ),			0, "" },

  { "deleteWorkCenter",  -4, TR("The selected Work Center cannot be deleted "
				"because Work Orders exist that use it. You "
				"must complete or close all Work Orders that "
				"use the selected Work Center before you may "
				"delete it." ),				0, "" },


  { "editccnumber",	-1, TR("You must select Master Card, Visa, American "
			       "Express or Discover as the credit card type."),
								 0, "" },
  { "editccnumber",	-2, TR("The length of a Master Card credit card number "
			       "has to be 16 digits."),		 0, "" },
  { "editccnumber",	-3, TR("The length of a Visa credit card number "
			       "has to be either 13 or 16 digits."), 0, "" },
  { "editccnumber",	-4, TR("The length of an American Express credit card "
			       "number has to be 15 digits."),	 0, "" },
  { "editccnumber",	-5, TR("The length of a Discover credit card number "
			       "has to be 16 digits."),		 0, "" },
  { "editccnumber",	-6, TR("The first two digits for a valid Master Card "
			       "number must be between 51 and 55"),
								 0, "" },
  { "editccnumber",	-7, TR("The first digit for a valid Visa number must "
			       "be 4"),				 0, "" },
  { "editccnumber",	-8, TR("The first two digits for a valid American "
			       "Express number must be 34 or 37."),
								 0, "" },
  { "editccnumber",	-9, TR("The first four digits for a valid Discover "
			       "Express number must be 6011."),	 0, "" },
  { "editccnumber",    -10, TR("The credit card number that you have provided "
			       "is not valid."),		 0, "" },

  { "enterReceipt",  -1, TR("Information for this order line item could not "
			       "be found. If it is a Purchase Order Item then "
			       "it does not appear to exist. If it is a "
			       "Transfer Order Item then either the Transfer "
			       "Order does not exist or there is no Item Site "
			       "for this line item."),		0, "" },

  { "explodeWo",  -1, TR("Work Order %1 cannot be Exploded as there is no "
			 "valid Bill of Materials on file for the Work Order "
			 "Item. You must create a valid Bill of Materials "
			 "for the Work Order Item before you may explode the "
			 "Work Order."),				0, "" },
  { "explodeWo",  -2, TR("Work Order %1 cannot be Exploded as there are one "
			 "or more Component Items on the Bill of Materials for "
			 "the Work Order Item that do not exist in the Work "
			 "Order Warehouse. You must create a valid Item Site "
			 "for all of the Component Items before you may "
			 "explode this Work Order."),			0, "" },
  { "explodeWo",  -3, TR("Work Order %1 cannot be Exploded as there are one "
			 "or more Co-Product/By-Product Items on the Breeder "
			 "Bill of Materials for the Work Order Item that do "
			 "not exist in the Work Order Warehouse. You must "
			 "create a valid Item Site for all of the Co-Product/"
			 "By-Product Items before you may explode this "
			 "Work Order."),				0, "" },
  { "explodeWo",  -4, TR("Work Order %1 cannot be Exploded because it is not "
			 "Open."),					0, "" },
  { "explodeWo",  -9, TR("Work Order %1 cannot be Exploded because the quantity"
			 "ordered is not valid."),			0, "" },

  { "freezeAccountingPeriod", -1, TR("Cannot freeze this Accounting Period "
				     "because it is still open."),     0, "" },
  { "freezeAccountingPeriod", -2, TR("Cannot freeze this Accounting Period "
				     "because it is already frozen."), 0, "" },

  { "insertGLTransaction",
			-3, TR("Nothing to do as the value to post to the "
			       "G/L is 0."),			 0, "" },
  { "insertGLTransaction",
			-4, TR("Cannot post a G/L transaction to a "
			       "closed period."),		 0, "" },

  { "insertIntoGLSeries", -1, TR("Cannot add to a G/L Series because the "
				 "Account is NULL or -1."),	0, "" },
  { "insertIntoGLSeries", -4, TR("Cannot add to a G/L Series because the "
				 "Accounting Period is closed."), 0, "" },

  { "issueAllBalanceToShipping",  -1, "",	 -1, "issueToShipping" },
  { "issueAllBalanceToShipping", -10, "",	-10, "issueToShipping" },
  { "issueAllBalanceToShipping", -12, "",	-12, "issueToShipping" },
  { "issueAllBalanceToShipping", -13, "",	-13, "issueToShipping" },
  { "issueAllBalanceToShipping", -14, "",	-14, "issueToShipping" },

  { "issueLineBalanceToShipping",  -1, "",	 -1, "issueToShipping" },
  { "issueLineBalanceToShipping", -10, "",	-10, "issueToShipping" },
  { "issueLineBalanceToShipping", -12, "",	-12, "issueToShipping" },
  { "issueLineBalanceToShipping", -13, "",	-13, "issueToShipping" },
  { "issueLineBalanceToShipping", -14, "",	-14, "issueToShipping" },

  { "issueToShipping",	-1, "",		 	 -1, "postInvTrans" },
  { "issueToShipping", -10, TR("The Next Shipment Number has not been set in "
			       "the Configure S/R window. Set that value and "
			       "try issuing to shipping again."), 0, "" },
  { "issueToShipping", -12, TR("The selected Sales Order is on Credit Hold and "
			       "must be taken off of Credit Hold before any "
			       "inventory may be issued to it."),	0, "" },
  { "issueToShipping", -13, TR("The selected Sales Order is on Packing Hold "
			       "and must be taken off of Packing Hold before "
			       "any inventory may be issued to it."),	0, "" },
  { "issueToShipping", -14, TR("The selected Sales Order is on Return Hold. "
			       "The Customer must return all materials for a "
			       "related Return Authorization before any inven"
			       "tory may be issued to this Order."),	0, "" },

  { "massReplaceBomitem",  -1, TR("Cannot make this BOM Item replacement "
				  "because it would create a recursive BOM."),
									0, "" },

  { "openAccountingPeriod", -1, TR("Cannot open this Accounting Period because "
				   "it is already open."),		0, "" },
  { "openAccountingPeriod", -2, TR("Cannot open this Accounting Period because "
				   "it is frozen."),			0, "" },

  { "openAccountingYearPeriod", -1, "", -1, "openAccountingPeriod" },

  { "postAPCreditMemoApplication", -1,
      TR("There are no A/P Credit Memo applications to post."),         0, "" },
  { "postAPCreditMemoApplication", -2,
      TR("There are no A/P Credit Memo applications to post."),         0, "" },
  { "postAPCreditMemoApplication", -3,
      TR("The total value of the applications that are you attempting to post "
         "is greater than the value of the A/P Credit Memo itself." ),  0, "" },
  { "postAPCreditMemoApplication", -4,
	TR("At least one A/P Credit Memo application cannot be posted because "
           "there is no current exchange rate for its currency."),      0, "" },
  { "postAPCreditMemoApplication", -5,
	TR("The A/P Credit Memo to apply was not found."),              0, "" },
  { "postAPCreditMemoApplication", -6,
	TR("The amount to apply for this A/P Credit Memo is NULL."),    0, "" },

  { "postARCreditMemoApplication", -1,
	    TR("There are no A/R Credit Memo applications to post."),	0, "" },
  { "postARCreditMemoApplication", -2,
	    TR("Either there are no A/R Credit Memo applications to post or there"
	       " is no exchange rate for one of the applications."),	0, "" },
  { "postARCreditMemoApplication", -3,
	    TR("The total value of the applications that you are attempting to "
	       "post is greater than the value of the A/R Credit Memo itself. "
               "Please reduce the applications to total less than the value "
               "of the Credit Memo."),                                  0, "" },
  { "postARCreditMemoApplication", -4,
	    TR("At least one A/R Credit Memo application cannot be posted "
	       "because there is no current exchange rate for its currency."),
									0, "" },
  { "postARCreditMemoApplication", -5,
	    TR("The A/R Credit Memo to apply was not found."),		0, "" },

  { "postBankAdjustment", -1, TR("This Bank Adjustment could not be posted "
				 "because the one or more required records do "
				 "not exist."),				0, "" },
  { "postBankAdjustment", -3, TR("This Bank Adjustment could not be posted "
				 "because the total adjustment is 0 so there "
				 "is nothing to post."),		0, "" },

  { "postBankReconciliation", -1, TR("This Bank Reconciliation could not be "
				     "posted because the G/L Account could not "
				     "be verified."),		0, "" },
  { "postBillingSelection",
			-1, TR("This Billing Selection cannot be posted "
			       "because it has already been posted."),	0, "" },

  { "postCashReceipt", -1, TR("The selected Cash Receipt cannot be posted as "
			      "the amount distributed is greater than the "
			      "amount received. You must correct this before "
			      "you may post this Cash Receipt."),	0, "" },
  { "postCashReceipt", -5, TR("The selected Cash Receipt cannot be posted as "
                               "the A/R Account cannot be determined. You must "
			       "make an A/R Account Assignment for the "
			       "Customer Type to which this Customer is "
			       "assigned before you may post this Cash "
			       "Receipt."),				0, "" },
  { "postCashReceipt", -6, TR("The selected Cash Receipt cannot be posted as "
			      "the Bank Account cannot be determined. You must "
			      "make a Bank Account Assignment for this Cash "
			      "Receipt before you may post it." ),	0, "" },
  { "postCashReceipt", -7, TR("The selected Cash Receipt cannot be posted, "
			      "probably because the Customer's Prepaid Account "
			      "was not found."),			0, "" },

  { "postCCcredit",  -1, TR("Cannot post this Credit Card refund because the "
                            "default Bank Account for Credit Card transactions "
                            "could not be found."),                     0, "" },
  { "postCCcredit",  -2, TR("Cannot post this Credit Card refund because an "
                            "invalid id/reference-type pair was passed."),
                                                                        0, "" },
  { "postCCcredit",  -3, TR("Cannot post this Credit Card refund because the "
                            "credit card and refund records could not be "
                            "found."),                                  0, "" },
  { "postCCcredit",  -4, TR("Cannot post this Credit Card refund because the "
                            "credit card payment records is not for a refund."),
                                                                        0, "" },

  { "postCheck",  -10, TR("Cannot post this Check because it has already "
			 "been posted."),				0, "" },
  { "postCheck",  -11, TR("Cannot post this Check because the recipient "
			 "type is not valid."),				0, "" },
  { "postCheck",  -12, TR("Cannot post this Check because the Expense "
			 "Category could not be found."),		0, "" },
  { "postCheck",  -13, TR("Cannot post this Check because the G/L Account "
			 "against which it is to be posted is not valid."),
			 						0, "" },

  { "postCreditMemo",	-1, TR("This Credit Memo cannot be posted because it "
			       "has already been posted."),	 0, "" },
  { "postCreditMemo",	-2, TR("This Credit Memo is on Hold and, thus, cannot "
			       "be posted."),			 0, "" },
  { "postCreditMemo",	-5, TR("The G/L Account Assignments for this Credit "
			       "Memo are not configured correctly. Because of "
			       "this, G/L Transactions cannot be posted for "
			       "this Credit Memo. You must contact your "
			       "Systems Administrator to have this corrected "
			       "before you may post this Credit Memo."),
								 0, "" },

  { "postGLSeries", -4, TR("Could not post this G/L Series because the "
				 "Accounting Period is closed."), 0, "" },
  { "postGLSeries", -5, TR("Could not post this G/L Series because the G/L "
			   "Series Discrepancy Account was not found."),
								 0, "" },

  { "postInvoice",  -1, "", -1, "insertIntoGLSeries" },
  { "postInvoice",  -4, "", -4, "insertIntoGLSeries" },
  { "postInvoice",  -5, "", -5, "postGLSeries" },
  { "postInvoice", -10, TR("Unable to post this Invoice because it has "
			   "already been posted."),		 0, "" },
  { "postInvoice", -11, TR("Unable to post this Invoice because the Sales "
			   "Account was not found."),		 0, "" },
  { "postInvoice", -12, TR("Unable to post this Invoice because there was an "
			   "error processing Line Item taxes."), 0, "" },
  { "postInvoice", -13, TR("Unable to post this Invoice because there was an "
			   "error processing Misc. Line Item taxes."), 0, "" },
  { "postInvoice", -14, TR("Unable to post this Invoice because the Freight "
			   "Account was not found."),		 0, "" },
  { "postInvoice", -15, TR("Unable to post this Invoice because there was an "
			   "error processing Freight taxes."),	 0, "" },
  { "postInvoice", -16, TR("Unable to post this Invoice because there was an "
			   "error processing Tax Adjustments."), 0, "" },
  { "postInvoice", -17, TR("Unable to post this Invoice because the A/R "
			   "Account was not found."),		 0, "" },


  { "postInvTrans",	-1, TR("Could not post an inventory transaction because"
			       " the Item Site has no Control Method or the "
			       "Item has an Item Type of Reference."),
								0, "" },
  { "postInvTrans",	-3, "",	 -3, "insertGLTransaction" },
  { "postInvTrans",	-4, "",	 -4, "insertGLTransaction" },

  { "postPoReceipt",	-1, "",	 -1, "postReceipt" },
  { "postPoReceipt",	-3, "",	 -3, "postReceipt" },
  { "postPoReceipt",	-4, "",	 -4, "postReceipt" },
  { "postPoReceipt",   -10, "",	-10, "postReceipt" },
  { "postPoReceipt",   -11, "", -11, "postReceipt" },
  { "postPoReceipt",   -12, "", -12, "postReceipt" },

  { "postReceipt",  -1, "",	 -1, "postInvTrans" },
  { "postReceipt",  -3, "",	 -3, "insertGLTransaction" },
  { "postReceipt",  -4, "",	 -4, "insertGLTransaction" },
  { "postReceipt", -10, TR("This Receipt Line has already been posted."),
  									0, "" },
  { "postReceipt", -11, TR("This Receipt Line cannot be "
			   "posted because it has a quantity of 0."),	0, "" },
  { "postReceipt", -12, TR("This Purchase Order Receipt Line has no "
			   "Standard Cost assigned to it."),		0, "" },

  { "postPoReceipts",   -1, "",	 -1, "postPoReceipt" },
  { "postPoReceipts",   -3, "",	 -3, "postPoReceipt" },
  { "postPoReceipts",   -4, "",	 -4, "postPoReceipt" },
  { "postPoReceipts",  -10, "",	-10, "postPoReceipt" },
  { "postPoReceipts",  -12, "",	-12, "postPoReceipt" },

  { "postPoReturns", -1, "", -1, "postInvTrans" },
  { "postPoReturns", -3, "", -3, "insertGLTransaction" },
  { "postPoReturns", -4, "", -4, "insertGLTransaction" },

  { "postVoucher",	-5, TR("The Cost Category for one or more Item Sites "
			       "for the Purchase Order covered by this Voucher "
			       "is not configured with Purchase Price Variance "
			       "or P/O Liability Clearing Account Numbers or "
			       "the Vendor of this Voucher is not configured "
			       "with an A/P Account Number. Because of this, "
			       "G/L Transactions cannot be posted for this "
			       "Voucher."),
								0, "" },

  { "recallShipment",	-1, TR("This shipment cannot be recalled because it "
			       "does not appear to have been shipped."),
								0, "" },
  { "recallShipment",	-2, TR("This shipment cannot be recalled because it "
			       "appears to have been invoiced."),
								0, "" },
  { "recallShipment",	-3, TR("This shipment cannot be recalled "
			       "because it has already been received "
			       "at its destination."),		0, "" },
  { "recallShipment",	-5, TR("This shipment cannot be recalled because it "
			       "contains one or more Line Items with Warehouse/"
			       "Product Category/Customer combinations that "
			       "have not been properly described in Sales "
			       "Account Assignments. These assignments must be "
			       "made before G/L Transactions can be posted and"
			       "this Sales Order is allowed to be recalled."),
								0, "" },
  { "releaseUnusedBillingHeader",
			-1, TR("Cannot release this Billing Header because it "
			       "has already been posted."),	0, "" },
  { "releaseUnusedBillingHeader",
			-2, TR("Cannot release this Billing Header because it "
			       "has Line Items."), 		0, "" },

  { "relocateInventory", -1, TR("You cannot Relocate more inventory than is "
                                "available."),                  0, "" },

  { "replaceAllVoidedChecks", -1, "", -1, "replaceVoidedCheck" },

  { "replaceVoidedCheck", -1, TR("Cannot replace this voided check because "
				 "either it has not been voided, it has "
				 "already been posted, or it has already been"
				 "replaced."), 0, "" },

  { "returnCompleteShipment",
			-5, TR("Either a Cost Category for the Items you are "
			       "trying to Return is not configured with a "
			       "Shipping Asset Account Number or a Customer "
			       "Type/Product Category/Warehouse Sales Account "
			       "assignment does not exist . Because of this, " 
			       "G/L Transactions cannot be posted for this "
			       "Return. You must contact your Systems "
			       "Administrator to have this corrected before "
			       "you may Return this Shipment."),
								0, "" },
  { "returnItemShipments", -1, "", -1, "postInvTrans" },
  { "returnItemShipments",
			-5, TR("Either a Cost Category for the Items you are "
			       "trying to Return is not configured with a "
			       "Shipping Asset Account Number or a Customer "
			       "Type/Product Category/Warehouse Sales Account "
			       "assignment does not exist . Because of this, " 
			       "G/L Transactions cannot be posted for this "
			       "Return. You must contact your Systems "
			       "Administrator to have this corrected before "
			       "you may Return this Shipment."),
								0, "" },

  { "returnShipmentTransaction",
			-5, TR("Either a Cost Category for the Items you are "
			       "trying to Return is not configured with a "
			       "Shipping Asset Account Number or a Customer "
			       "Type/Product Category/Warehouse Sales Account "
			       "assignment does not exist . Because of this, " 
			       "G/L Transactions cannot be posted for this "
			       "Return. You must contact your Systems "
			       "Administrator to have this corrected before "
			       "you may Return this Shipment."),
								0, "" },
  { "selectForBilling",	-1, TR("The quantity you have selected for Billing is "
			       "less than the quantity shipped. You may not "
			       "bill for less than the quantity shipped."),
								0, "" },

  { "shipShipment",  -1, "",	 -1, "postInvTrans"		},
  { "shipShipment",  -3, "",	 -3, "insertGLTransaction"	},
  { "shipShipment",  -4, "",	 -4, "insertGLTransaction"	},
  { "shipShipment",  -5, TR("This Sales Order may not be shipped as it "
			    "contains one or more Line Items that have "
			    "Warehouse/Product Category/Customer combinations "
			    "that have not been properly described in Sales "
			    "Account Assignments.  These assignments must be "
			    "made before G/L Transactions can be posted and "
			    "this Sales Order is allowed to ship."),	0, "" },
  { "shipShipment",  -6, TR("This Transfer Order may not be shipped "
			    "because there is no Item Site for the "
			    "Transit Warehouse."),			0, "" },
  { "shipShipment",  -8, TR("This Shipment cannot be shipped because it "
			    "appears to have already shipped."),	0, "" },
  { "shipShipment", -12, TR("The selected Order is on Credit Hold and must be "
			    "taken off of Credit Hold before it "
			    "may be shipped."),				0, "" },
  { "shipShipment", -13, TR("The selected Order is on Packing Hold and must be "
			     "taken off of Packing Hold before it "
			     "may be shipped."),			0, "" },
  { "shipShipment", -14, TR("The selected Order is on Return Hold. The Customer"
			     " must return all materials for a related Return "
			     "Authorization before this order may be shipped."),
			     						0, "" },
  { "shipShipment", -15, TR("The selected Order is on Shipping Hold and must "
			     "be taken off of Shipping Hold before it "
			     "may be shipped."),			0, "" },
  { "shipShipment", -50, TR("This Shipment cannot be shipped because it "
			    "does not appear to exist."),		0, "" },
  { "shipShipment", -99, TR("This Order may not be shipped because it has been "
			    "marked as Ship Complete and quantities for one or "
			    "more Line Items are still not completely issued. "
			    "Please correct this before shipping the Order."),
									0, "" },
  { "splitReceipt", -1, TR("Only Purchase Order Receipts may be split."),		0, "" },
  { "splitReceipt", -2, TR("Only posted receipts may be split."),		0, "" },
  { "splitReceipt", -3, TR("Vouchered receitps may not be split."),		0, "" },
  { "splitReceipt", -4, TR("Split quantity must me less than original receipt quantity."),		0, "" },
  { "splitReceipt", -5, TR("Split freight may not be greater than original freight."),		0, "" },
  { "splitReceipt", -6, TR("Receipt not found."),		0, "" },
  { "splitReceipt", -7, TR("The split quantity must be a positive number."),		0, "" },

  { "sufficientInventoryToShipItem", -1,
	    TR("Cannot figure out which line item to issue."),	0, "" },
  { "sufficientInventoryToShipItem", -2,
	    TR("There is not enough Inventory to issue the amount required of "
	       "Item %1 in Warehouse %2."),			0, "" },
  { "sufficientInventoryToShipItem", -3, 
            TR("Item Number %1 in Warehouse %2 is a Multiple Location or "
               "Lot/Serial controlled Item which is short on Inventory. "
               "This transaction cannot be completed as is. Please make "
               "sure there is sufficient Quantity on Hand before proceeding."),
								0, "" },
  { "sufficientInventoryToShipItem", -11, 
            TR("Invalid Order Type.  Only Sales Orders and Transfer Orders "
               "may be shipped from this window."),
								0, "" },                            
  { "sufficientInventoryToShipOrder", -1, TR("Cannot check inventory levels for"
                                             "an invalid item."),       0, ""},
  { "sufficientInventoryToShipOrder", -2, TR("There is not enough Inventory to "
                                             "issue the amount required of one "
                                             "of the items requested."),0, ""},
  { "sufficientInventoryToShipOrder", -3, TR("One of the requested items is a "
                                             "Multiple Location or Lot/Serial "
                                             "controlled Item which is sort on "
                                             "Inventory."),             0, ""},
  { "sufficientInventoryToShipOrder",-11, "",-11, "sufficientInventoryToShipItem"},

  { "thawAccountingPeriod", -2, TR("Cannot thaw this Accounting Period "
				     "because it is not frozen."), 0, "" },

  { "todoItemMove",	-1, TR("Cannot change the Sequence of a non-existent "
			       "To-Do List Item. Possible cause: no To-Do "
			       "List Item was selected."),
								0, "" },

  { "updateTodoItem",	-1, TR("The To-Do List Item cannot be updated as "
			       "there is no assigned User."),	0, "" },
  { "updateTodoItem",	-2, TR("The To-Do List Item cannot be updated as "
			       "the Task Name is blank."),	0, "" },
  { "updateTodoItem",	-3, TR("The To-Do List Item cannot be updated as "
			       "there is no Due Date."),	0, "" },
  { "updateTodoItem",  -10, TR("The To-Do List Item cannot be updated as "
			       "an invalid internal ID was supplied ."),
								 0, "" },

  { "voidCheck", -1, TR("Cannot void this check because either it has already "
			"been voided, it has already been posted, or it has "
			"already been replaced. If this check has been posted, "
			"try Void Posted Check with the Check Register "
			"window."),					0, "" },

  { "voidPostedCheck", -10, TR("Cannot void this check because it has already "
			       "been voided."),				0, "" },
  { "voidPostedCheck", -11, TR("Cannot void this check because the recipient "
			       "type is not valid."),			0, "" },
  { "voidPostedCheck", -12, TR("Cannot void this check because the Expense "
			       "Category could not be found."),		0, "" },
  { "voidPostedCheck", -13, TR("Cannot void this check because the G/L account "
			       "to which the funds should be credited is not "
			       "valid."),				0, "" },

  { "reserveSoLineBalance", -1, "", -1, "reserveSoLineQty" },
  { "reserveSoLineBalance", -2, "", -2, "reserveSoLineQty" },
  { "reserveSoLineBalance", -3, "", -3, "reserveSoLineQty" },

  { "reserveSoLineQty", -1, TR("Cannot reserve more quantity than remaining on order."), 0, ""},
  { "reserveSoLineQty", -2, TR("Cannot reserve negative quantities."), 0, ""},
  { "reserveSoLineQty", -3, TR("Cannot reserve more quantity than currently on hand and already reserved."), 0, ""},


  { "woClockIn",  -1, "", -1, "explodeWo" },
  { "woClockIn",  -2, "", -2, "explodeWo" },
  { "woClockIn",  -3, "", -3, "explodeWo" },
  { "woClockIn",  -9, TR("Work Order %1 cannot be Exploded as it seems to have "
			 "an invalid Order Quantity."),			0, "" },
  { "woClockIn", -10, TR("Work Order %1 has at least one Item in its Bill of "
			 "Materials with the Push issue method that has not "
			 "yet been issued. You must issue all Push Items "
			 "to this Work Order."),			0, "" },
  { "woClockIn", -11, TR("Work Order %1 has at least one Item in its Bill of "
			 "Materials with the Push issue method that does not "
			 "have the required quantity issued. You must issue "
			 "all Push Items to this Work Order."),		0, "" },
  { "woClockIn", -12, TR("Work Order %1 is closed."),			0, "" },
};

  if (! ErrorLookupHash.isEmpty())
  {
    QMessageBox::critical(0, TR("ErrorLookupHash initialization error"),
		TR("ErrorLookupHash has already been initialized."));
    return;
  }

  unsigned int numElems = sizeof(errors) / sizeof(errors[0]);
  for (unsigned int i = 0; i < numElems; i++)
  {
    QPair<int, QString> currPair;
    if (errors[i].msgPtr == 0)
      currPair = qMakePair(errors[i].retVal, errors[i].msg);
    else
    {
      QList<QPair<int, QString> > proxyList;

      QString proxyname;
      if (errors[i].proxyName.isEmpty())
	proxyname = errors[i].procName.toUpper();
      else
	proxyname = errors[i].proxyName.toUpper();

      proxyList = ErrorLookupHash.values(proxyname);

      if (proxyList.size() > 0)
      {
	int proxyIndex;
	for (proxyIndex = 0; proxyIndex < proxyList.size(); proxyIndex++)
	{
	  if (proxyList.at(proxyIndex).first == errors[i].msgPtr)
	  {
	    currPair = qMakePair(errors[i].retVal,
				 QString(proxyList.at(proxyIndex).second));
	    break;
	  }
	}

	if (proxyIndex >= proxyList.size())
	{
          QMessageBox::critical(0, TR("Lookup Error"),
                                TR("Could not find (%1, %2) in ErrorLookupHash "
                                   "when trying to insert proxy entry for "
                                   "(%3, %4).")
			   .arg(proxyname).arg(errors[i].msgPtr)
			   .arg(errors[i].procName).arg(errors[i].retVal));
	  continue;
	}
      }
      else // couldn't find proxyList => forward reference
      {
	unsigned int j;
	for (j = i + 1; j < numElems; j++)
	{
	  if (errors[j].retVal == errors[i].msgPtr &&
	      errors[j].procName.toUpper() == proxyname)
	    {
	    currPair = qMakePair(errors[i].retVal,
				 QString(errors[j].msg));
	    break;
	    }
	}
	if (j >= numElems)
	{
          QMessageBox::critical(0, TR("Lookup Error"),
                                TR("Could not find (%1, %2) in ErrorLookupHash "
                                   "when trying to insert proxy entry for "
                                   "(%3, %4).")
			  .arg(errors[i].proxyName).arg(errors[i].msgPtr)
			  .arg(errors[i].procName).arg(errors[i].retVal));
	continue;
	}
      }
    }

    ErrorLookupHash.insert(errors[i].procName.toUpper(), currPair);
  } // for
}


QString storedProcErrorLookup(const QString procName, const int retVal)
{
  QString returnStr = "";

  if (ErrorLookupHash.isEmpty())
    initErrorLookupHash();

  QList<QPair<int, QString> > list = ErrorLookupHash.values(procName.toUpper());

  for (int i = 0; i < list.size(); i++)
  {
    if (list.at(i).first == retVal)
    {
      returnStr = list.at(i).second;
      break;
    }
  }

  if (returnStr.isEmpty())
    returnStr = TR("A Stored Procedure failed to run properly.");

  returnStr = QString("<p>") + returnStr +
	      QString("<br>(%1, %2)<br>").arg(procName).arg(retVal);

  return returnStr;
}
