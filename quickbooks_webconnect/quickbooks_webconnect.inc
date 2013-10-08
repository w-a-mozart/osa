<?php
/**
 * Functions that can be used by other modules.
 */

 
/**
 * Queues a request to be processed by the QuickBooks Web Connector client
 *
 * @param array $element
 *  An associative array of values. This element will be transformed into the
 *  appropriate QuickBooks_QBXML_Objects.
 *
 * @param string $element_type
 *  Type of root element being processed (Customer, Invoice, SalesReceipt, ...)
 *  Sub-elements (InvoiceLine, SalesReceiptLine, ...) will be recursively transformed.
 *
 * @param string $operation
 *  Operation to perform (Add, Mod, Delete, ...)
 */
function quickbooks_webconnect_queue_request($element, $element_type, $operation) {

  // create the item to place on the queue
  $item = array(
    'element' => $element,
    'type'    => $element_type,
    'op'      => $operation,
  );
  
  // add the item to the queue
  $queue = DrupalQueue::get('quickbooks_webconnect');
  $queue->createItem($item);
  
  _quickbooks_webconnect_debug('[queue_request] item : @item', array('@item' => var_export($item, TRUE)));
}

/**
 * Transform source elements into a QuickBooks_QBXML_Objects
 */
function _quickbooks_webconnect_transform($element, $element_type, $operation) {

  // allow modules to establish a mapping of source fields to QBXML schema names
  $tranform = &drupal_static(__FUNCTION__);
  if (!isset($tranform)) {
    $tranform = array('source_name' => array(), 'qbxml_name' => array());
    $map = module_invoke_all('quickbooks_webconnect_qbxml_name_map');
    foreach ($map as $key => $val) {
      $tranform['source_name'][] = '"' . $key . '":';
      $tranform['qbxml_name'][] = '"' . $val. '":';
    }
    $tranform['values'] = module_invoke_all('quickbooks_webconnect_qbxml_value_map');
  }

  // cheat and use json encoding to eliminate recursive array handling for key replacement
  $json = json_encode($element);
  $new_json = str_replace($tranform['source_name'], $tranform['qbxml_name'], $json);
  $element = json_decode($new_json, TRUE);

  // modify values
  if (!empty($tranform['values'])) {
    _quickbooks_webconnect_replace($element, $tranform['values']);
  }

  // allow modules to provide further transformation logic
  drupal_alter('quickbooks_webconnect_element', $element, $element_type, $operation);

  // create a QBXML object
  require_once 'quickbooks_webconnect.devkit.inc';

  // special processing for objects with sub-objects (line items)
  // @todo make generic / recursive one day
  $sub_element_types = array('ItemLine', 'ExpenseLine', 'ItemGroupLine', 'DepositLine', 'InvoiceLine', 'DiscountLine', 'SalesTaxLine', 'ShippingLine', 'JournalCreditLine', 'JournalDebitLine', 'SalesOrderLine', 'SalesReceiptLine',);
  foreach ($sub_element_types as $sub_element_type) {
    if (isset($element[$sub_element_type])) {
      $sub_objects = array();
      foreach ($element[$sub_element_type] as $sub_element) {
        $sub_obj_type = "QuickBooks_QBXML_Object_{$element_type}_{$sub_element_type}";
        $sub_objects[] = new $sub_obj_type($sub_element);
      }
      $element[$sub_element_type] = $sub_objects;
    }
  }
  
  $obj_type = 'QuickBooks_QBXML_Object_' . $element_type;
  $object = new $obj_type($element);

  return $object;
}

/*
 * modify values (not recursive)
 */
function _quickbooks_webconnect_replace(&$element, $map, $breadcrumb = array()) {
  foreach($map as $key => $map_values) {
    if (array_key_exists($key, $element)) {
      $element_value = $element[$key];
      if (is_string($element_value) && array_key_exists($element_value, $map_values)) {
        $element[$key] = $map_values[$element_value];
      }
    }
  }
}

/**
 * Wraps string in qbxml starting and ending tags
 *
 * @param string $qbxml
 *   xml to wrap
 *
 * @return string
 *   wrapped output
 */
function _quickbooks_webconnect_wrap_qbxml($qbxml){

  $ver = variable_get('quickbooks_webconnect_qbxml_version', '8.0');
  $output = <<<EOD
<?xml version="1.0" encoding="utf-8"?>
<?qbxml version="{$ver}"?>
<QBXML>
  <QBXMLMsgsRq onError="stopOnError">
    {$qbxml}
  </QBXMLMsgsRq>
</QBXML>
EOD;

  return $output;
}

/*
 * Log debug messages.
 */
function _quickbooks_webconnect_debug($message, $variables = array()) {
  if (variable_get('quickbooks_webconnect_debug', FALSE)) {
    watchdog('quickbooks_webconnect', $message, $variables, WATCHDOG_DEBUG);
  }
}
