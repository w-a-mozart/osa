<?php

/**
 * Allows modules to define a mapping to QBXML element names
 *
 * @return array $map
 *  Array in the form "source name" => "qbxml name". Specify QBXML reference names as "qbNameRef ListID" or "qbNameRef FullName"
 */
function hook_quickbooks_webconnect_qbxml_name_map() {
  return array(
    'first_name' => 'FirstName',
    'last_name' => 'LastName',
  );
}

/**
 * Allows modules to define a mapping to QBXML element values
 *
 * @return array $map
 *  Nested array in the form "qbxml name" => array("source value" => "qbxml value")
 */
function hook_quickbooks_webconnect_qbxml_value_map() {
  return array(
    'PaymentMethodRef FullName' => array(
      'Cheque' => 'Check',
    )
  );
}

/**
 * Allows modules to alter the transformation between source and QuickBooks_QBXML_Objects 
 *
 * @param array $element
 *  Element used to construct the QuickBooks_QBXML_Object (after name replacement)
 *
 * @param string $element_type
 *  Type of element being processed
 *
 * @param string $operation
 *  Type of operation being processed
 */
function hook_quickbooks_webconnect_element_alter(&$element, $element_type, &$operation) {
  switch($element_type) {
    case 'Customer':
      $element['Name'] = "{$element['id']} - {$element['FirstName']} {$element['LastName']}";
      break;
  }
}

/**
 * Allow modules to alter the final QBXML before it is sent the client
 *
 * @param string $qbxml
 *  String of QBXML
 *
 * @param array $element
 *  Element being processed
 *
 * @param string $element_type
 *  Type of element being processed
 */
function hook_quickbooks_webconnect_qbxml_alter(&$qbxml, $element, $element_type) {
  $xml = simplexml_load_string($qbxml);

  foreach ($xml->children() as $child) {
    $child->addAttribute('onError', 'stopOnError');
  }
}

/**
 * Allow modules to process the QBXML response from the client
 *
 * @param string $qbxml
 *  The QBXML response
 *
 * @param QuickBooks_QBXML_Object $qb_object
 *  Object created from the QBXML response
 *
 * @param array $element
 *  Original element being processed
 *
 * @param string $element_type
 *  Type of element being processed
 *
 * @param string $operation
 *  Type of operation being processed
 *
 */
function hook_quickbooks_webconnect_process_response($qbxml, $qb_object, $element, $element_type, $operation) {
}
