<?php
/**
 * @file
 * Functions that interact with QuickBooks
 */

/**
 * Create a Sales Receipt from a Drupal Commerce Order
 * (triggered by a Rule, when a check-out is completed & the balance is zero)
 */
function osa_quickbooks_queue_order_as_sales_receipt($commerce_order) {
  _osa_quickbooks_queue_order($commerce_order, 'SalesReceipt');
}

/**
 * Create an Invoice from a Drupal Commerce Order
 * (triggered by a Rule, when a check-out is completed & the balance is still owing)
 */
function osa_quickbooks_queue_order_as_invoice($commerce_order) {
  _osa_quickbooks_queue_order($commerce_order, 'Invoice');
}

/**
 * Queue a QuickBooks QBXML element based on a Drupal Commerce Order
 */
function _osa_quickbooks_queue_order($commerce_order, $element_type) {
  // add/update customer in qb
  $element = osa_quickbooks_queue_uid_as_customer($commerce_order->uid);

  // add the order to the element array
  $element += (array)$commerce_order;
  
  // add each line item based on the components in the price not just each line item
  unset($element['commerce_line_items']);
  $element["{$element_type}Line"] = array();

  $order_wrapper = entity_metadata_wrapper('commerce_order', $commerce_order);
  foreach ($order_wrapper->commerce_line_items as $line_item_wrapper) {
    $line_item = $line_item_wrapper->value();
    $item_price = $line_item->commerce_total['und'][0];
    $components = array();
    
    foreach ($item_price['data']['components'] as $component) {
      $component_type = commerce_price_component_type_load($component['name']);
      $component_price = commerce_price_component_total($item_price, $component['name']);

      if (empty($components[$component['name']])) {
        $components[$component['name']] = array(
          'title'  => check_plain($component_type['display_title']),
          'amount' => commerce_currency_amount_to_decimal($component_price['amount'], $component_price['currency_code']),
          'weight' => $component_type['weight'],
        );
      }
    }

    // create the base line item
    if (isset($components['base_price'])) {
      $line_element = (array)$line_item;
      $line_element += $components['base_price'];
      $line_element['title'] = commerce_line_item_title($line_item);

      // special code for Camp Fees
      if ($line_element['title'] == 'Camp Fees') {
        if ($line_element['type'] == 'civi_event') {
          $table = osa_html_table_to_array($line_element['line_item_label']);
          $participant = $table[0][0];
          $line_element['line_item_label'] = '';
          $sep = '';
          foreach ($table as $row) {
            $line_element['line_item_label'] .= $sep . $row[0];
            $amt = trim($row[1]);
            if (empty($amt)) {
              $sep = ', ';
            }
            else {
              $line_element['line_item_label'] .= " ({$amt})";
              $sep = "\r\n";
            }
          }
        }
        else {
          $participant = $line_element['line_item_label'];
          $line_element['line_item_label'] = "Camp fee adjustment for {$participant}\r\n{$line_element['data']['custom_34']}";
        }
      }

      $element["{$element_type}Line"][] = $line_element;
      unset($components['base_price']);
    }
    
    // create other line items (usually discounts)
    uasort($components, 'drupal_sort_weight');
    foreach ($components as $name => $component) {
      if (abs($component['amount']) > 0.009) {
        $component['line_item_label'] = $component['title']. ' for ' . $line_element['line_item_label'];

        // special code for Camp Fees
        if ($line_element['title'] == 'Camp Fees') {
          $component['line_item_label'] = $component['title']. ' for ' . $participant;
          $component['title'] = 'Camp ' . $component['title'];
        }
        
        $element["{$element_type}Line"][] = $component;
      }
    }
  }

  // add payment detail
  $payments = commerce_payment_transaction_load_multiple(array(), array('order_id' => $commerce_order->order_id));
  if (!empty($payments)) {
    // grab the last transaction
    $payment = array_pop($payments);
    $element['created'] = $payment->created;
    $element += (array)$payment;
  }

  module_load_include('inc', 'quickbooks_webconnect');
  quickbooks_webconnect_queue_request($element, $element_type, 'Add');
}

/**
 * Create a Receive Payment from a Drupal Commerce Payment
 * (triggered by a Rule, when a cheque is cleared in commerce_cheque)
 */
function osa_quickbooks_queue_payment_as_receive_payment($commerce_payment_transaction) {
  // add/update customer in qb
  if ($commerce_payment_transaction->order_id) {
    $commerce_order =  commerce_order_load($commerce_payment_transaction->order_id);
    $uid = $commerce_order->uid;
  }
  else {
    $uid = $commerce_payment_transaction->uid;
  }
  $element = osa_quickbooks_queue_uid_as_customer($uid);

  // add payment detail
  $element += (array)$commerce_payment_transaction;

  module_load_include('inc', 'quickbooks_webconnect');
  quickbooks_webconnect_queue_request($element, 'ReceivePayment', 'Add');
}

/**
 * Create/Update a Customer from a Drupal user id
 */
function osa_quickbooks_queue_uid_as_customer($uid) {
  civicrm_initialize();

  // Use the Household as the Customer in QB
  $cid = _osa_get_contact_id($uid);
  $family_id = _osa_getHousehold($cid);
  if ($family_id) {
    $family = _osa_get_contact($family_id, FALSE);
    $contact = _osa_get_contact($cid, FALSE);
    $family['primary_contact'] = $contact['display_name'];
  }
  else {
    $family_id = 1;
    $family = _osa_get_contact($family_id, FALSE);
    $family['display_name'] = 'Anonymous';
  }

  $element_type = 'Customer';
  if (empty($family['external_identifier'])) {
    $return['CustomerRef FullName'] = "{$family['display_name']} <{$family['id']}>";;
    $operation = 'Add';
  }
  else {
    list($list_id, $edit_seq) =  explode("|", $family['external_identifier']);
    $return['CustomerRef ListID'] = $list_id;
    
    // no need to update the customer in QB, if QB was the last to update it in CiviCRM
    $lastModified = CRM_Core_BAO_Log::lastModified($family_id);
    $operation = ($lastModified['name'] == 'QuickBooks User') ? $operation = 'NoOp' : $operation = 'Mod';
  }

  if ($operation != 'NoOp') {
    module_load_include('inc', 'quickbooks_webconnect');
    quickbooks_webconnect_queue_request($family, $element_type, $operation);
  }
  
  return $return;
}

/**
 * Implements hook_civicrm_post (similar to db trigger) for Contributions
 * used to handle transactions that do not go through Drupal Commerce
 */
function osa_civicrm_post_Contribution($op, $contribution_id, &$contribution) {

  // Teacher Pay instalments are recurring payments that do not go through the Shopping Cart
  if (($contribution->financial_type_id == OSA_FINTYPE_TEACHER_FEE) && ($contribution->total_amount > 0) && ($op == 'create')) {
    // Customers in QB are the households, but the contribution in Civi is linked to the student
    $head_household = _osa_getHeadOfHousehold($contribution->contact_id);
    $result = civicrm_api('UFMatch', 'get', array('contact_id' => $head_household['contact_id'], 'sequential' => 1, 'version' => 3));
    $uid = isset($result['values'][0]['uf_id']) ? $result['values'][0]['uf_id'] : 0;
    // should not happen
    if ($uid == 0) {
      watchdog('osa', 'Created teacher pay contribution without a family profile: @contribution', array('@contribution' => print_r($contribution, TRUE)), WATCHDOG_ERROR);
      return;
    }
    $element = osa_quickbooks_queue_uid_as_customer($uid);
    
    // we use a Sales Order (non-posted transaction) to track the Teacher pay instalments
    $element_type = 'SalesOrder';
    $element += (array)$contribution;

    module_load_include('inc', 'quickbooks_webconnect');
    quickbooks_webconnect_queue_request($element, $element_type, 'Add');
  }
}

/**
 * Implements hook_quickbooks_webconnect_process_response
 */
function osa_quickbooks_webconnect_process_response($qbxml, $qb_object, $element, $element_type, $operation) {
  // log the incoming XML
  watchdog('osa_quickbooks', 'IN: @qbxml', array('@qbxml' => $qbxml), WATCHDOG_INFO);

  // save the QB ListId and EditSequence of the Customer
  if ($element_type == 'Customer') {
    $external_id = "{$qb_object->getListID()}|{$qb_object->getEditSequence()}";
    civicrm_initialize();
    $result = civicrm_api('contact', 'create', array('id' => $element['contact_id'], 'external_identifier' => $external_id, 'version' => 3));
  }
  
  // we use Sales Orders to track teacher pay instalments, so we'll create a post-dated invoice for each instalment
  elseif ($element_type == 'SalesOrder') {
    // get the teacher pay item
    $line_item = $qb_object->getSalesOrderLine(0);
    if ((!isset($line_item)) || ($line_item->getItemName() != 'Teacher Fee')) {
      // nothing to do
      return;
    }
    
    // get the date of the first invoice
    $num_installments = variable_get('osa_num_installments', 10);
    $inv_date = _osa_next_payment_date(variable_get('osa_start_date', 'first day of September'), $num_installments, $qb_object->getTxnDate());
    // get the number of instalments from the Sales Order (in case it's different)
    $num_installments = $line_item->getQuantity();

    // build invoices based on the Sales Order
    $element = array(
      'CustomerRef ListID' => $qb_object->getCustomerListID(),
      'InvoiceLine' => array(
        array(
          'Quantity' => 1,
          'LinkToTxn TxnID' => $qb_object->getTxnId(),
          'LinkToTxn TxnLineID' => $line_item->getTxnLineID(),
        ),
      ),
    );
    for ($i = 1; $i <= $num_installments; $i++) {
      $element['TxnDate'] = $inv_date->format('Y-m-d');
      $element['RefNumber'] = $qb_object->getRefNumber() . '-' . $i;
      // queue them to QB
      quickbooks_webconnect_queue_request($element, 'Invoice', 'Add');
      $inv_date->modify("first day of +1 month");
    }
  }
}

/**
 * Implements hook_quickbooks_webconnect_qbxml_name_map for simple transformations
 */
function osa_quickbooks_webconnect_qbxml_name_map() {
  return array(
    // Civi Contact   => QB Customer
    'display_name'    => 'Name',
    'household_name'  => 'CompanyName',
    'primary_contact' => 'Contact',
    'phone'           => 'Phone',
    'email'           => 'Email',
    'street_address'  => 'BillAddress Addr1',
    'city'            => 'BillAddress City',
    'state_province'  => 'BillAddress State',
    'postal_code'     => 'BillAddress PostalCode',
    'country'         => 'BillAddress Country',

    // Order   => SalesReceipt / Invoice
    'order_id' => 'RefNumber',
    'changed'  => 'TxnDate',

    // Civi Contribution => SalesOrder
    'receive_date'       => 'TxnDate',

    // Line Items     => SalesReceiptLine / InvoiceLine
    'title'           => 'ItemRef FullName',
    'line_item_label' => 'Desc',
    'quantity'        => 'Quantity',
    'amount'          => 'Amount',

    // Civi LineItem => SalesOrderLine
    'financial_type' => 'ItemRef FullName',
    'label'          => 'Desc',
    'qty'            => 'Quantity',
    'line_total'     => 'Amount',

    // Payment       => SalesReceipt / ReceivePayment
    'payment_method' => 'PaymentMethodRef FullName',
    'remote_id'      => 'CheckNumber',
  );
}

/**
 * Implements hook_quickbooks_webconnect_qbxml_value_map
 */
function osa_quickbooks_webconnect_qbxml_value_map() {
  return array(
    'PaymentMethodRef FullName' => array(
      'commerce_cheque' => 'Cheque',
      'commerce_payment_example' => 'Direct Payment',
    ),
  );
}

/**
 * Implements hook_quickbooks_webconnect_element_alter for more complex transformation logic
 */
function osa_quickbooks_webconnect_element_alter(&$element, $element_type, &$operation) {
  switch($element_type) {
    case 'Customer':
      $element['Name'] = "{$element['Name']} <{$element['id']}>";
      // make sure more than one Add request queued prior to be processed by QB
      if ($operation == 'Add') {
        civicrm_initialize();
        $contact = _osa_get_contact($element['id'], FALSE);
        if (!empty($contact['external_identifier'])) {
          $element['external_identifier'] = $contact['external_identifier'];
          $operation = 'Mod';
        }
      }
      if ($operation == 'Mod') {
        list($list_id, $edit_seq) =  explode("|", $element['external_identifier']);
        $element['ListID'] = $list_id;
        $element['EditSequence'] = $edit_seq;
      }
    break;

    case 'SalesOrder':
      // fix date format from CRM_Contribute_BAO_Contribution object
      $element['TxnDate'] = date('Y-m-d', strtotime($element['TxnDate']));
      
      // get the contribution to get the custom data fields not available in hook_civicrm_post
      $contribution_id = $element['id'];
      civicrm_initialize();
      $result = civicrm_api('contribution', 'get', array('id' => $contribution_id, 'version' => 3));
      $contribution = $result['values'][$contribution_id];
      $element += $contribution;
      $teacher = _osa_get_contact($contribution[_osa_field('selected_teacher')]);
      $lesson_level = $teacher['lesson_fee_level'][$contribution[_osa_field('lesson_level')]];

      // set the line item
      $result = civicrm_api('LineItem', 'get', array('entity_id' => $contribution_id, 'entity_table' => 'civicrm_contribution', 'rowCount' => PHP_INT_MAX, 'version' => 3));
      foreach ($result['values'] as $lineitem) {
        if ($lineitem['line_total'] > 0) {
          $line['ItemRef FullName'] = $contribution['financial_type'];
          $line['Desc'] = $teacher['display_name'] . ' - ' . trim(strip_tags($lineitem['label'])). ' (' . $lesson_level . ')';
          $line['Quantity'] = $lineitem['qty'];
          $line['Amount'] = $lineitem['line_total'];
          $element["{$element_type}Line"][] = $line;
        }
      }

      $element['IsToBePrinted'] = 'false';
      $element['IsToBeEmailed'] = 'false';
    break;

    case 'SalesReceipt':
    case 'Invoice':
      $element['IsToBePrinted'] = 'false';
      $element['IsToBeEmailed'] = 'false';
    break;
    
    case 'ReceivePayment':
      $element['IsAutoApply'] = 'true';
      $element['RefNumber'] = $element['CheckNumber'];
      $element['TotalAmount'] = commerce_currency_amount_to_decimal($element['Amount'], $element['currency_code']);
    break;
  }

  // modify some of the payment info
  // note that we can't set the CreditCardTxnInfo because it is reserved for QB Merchant Services
  if (isset($element['PaymentMethodRef FullName'])) {
    if ($element['PaymentMethodRef FullName'] == 'commerce_elavon') {
      // the card number is buried in the payment message
      $txn_msgs = preg_split('/\R/', $element['message']);
      $tmp = explode(': ', $txn_msgs[0]);
      // overload the Check Number with the card number
      $element['CheckNumber'] = $tmp[1];
      $element['PaymentMethodRef FullName'] = strncmp($tmp[1], '4', 1) === 0 ? 'Visa' : 'MasterCard';
    }
    elseif (($element['PaymentMethodRef FullName'] == 'Cheque') && ($element_type == 'ReceivePayment')) {
      if (is_array($element['payload'])) {
        $tmp = array_pop($element['payload']);
        if (isset($tmp['cheque_date'])) {
          $element['TxnDate'] = $tmp['cheque_date'];
        }
      }
    }
  }
}

/**
 * Implements hook_quickbooks_webconnect_qbxml_alter
 */
function osa_quickbooks_webconnect_qbxml_alter(&$qbxml, $element, $element_type) {
  // log the outgoing XML
  watchdog('osa_quickbooks', 'OUT: @qbxml', array('@qbxml' => $qbxml), WATCHDOG_INFO);
}
