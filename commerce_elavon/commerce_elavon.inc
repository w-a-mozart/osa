<?php

/**
 * @file
 * Helper functions for the Elavon Payment Gateway interface.
 */

define('COMMERCE_ELAVON_URL', 'https://www.myvirtualmerchant.com/VirtualMerchant/processxml.do');
define('COMMERCE_ELAVON_TEST_URL', 'https://demo.myvirtualmerchant.com/VirtualMerchantDemo/processxml.do');
define('COMMERCE_ELAVON_TIMEOUT', '30');

/**
 * Returns an array specifying optional CC fields that can be included on a payment form.
 * - see commerce_payment_credit_card_form()
 */
function commerce_elavon_form_fields() {
  return array(
    'type' => t('Card type'),
    'owner' => t('Card owner'),
    'start_month' => t('Start month'),
    'start_year' => t('Start year'),
    'issue' => t('Card issue number'),
    'code' => t('Security code'),
    'bank' => t('Issuing bank'),
  );
}

/**
 * Creates a payment transaction and posts data to Elavon.
 *
 * @param $payment_method
 *   An array of the current settings.
 * @param $values
 *   The values submitted by the user.
 * @param $order
 *   The order object.
 * @param $charge
 *   A price structure that needs to be charged.
 */
function commerce_elavon_transaction($payment_method, $values, $order, $charge) {

  // create a commerce transaction
  $transaction                 = commerce_payment_transaction_new('commerce_elavon', $order->order_id);
  $transaction->instance_id    = $payment_method['instance_id'];
  $transaction->amount         = $charge['amount'];
  $transaction->currency_code  = $charge['currency_code'];

  // do currency conversion
  if ($charge['currency_code'] != commerce_default_currency()) {
    $charge['amount'] = commerce_currency_convert($charge['amount'], $charge['currency_code'], commerce_default_currency());
  }
  
  // create an Elavon transaction
  $txn_array = array(
    'ssl_merchant_id'      => variable_get('commerce_elavon_merchant_id', ''),
    'ssl_user_id' 	       => variable_get('commerce_elavon_user_id', ''),
    'ssl_pin'     	       => variable_get('commerce_elavon_pin', ''),
    'ssl_test_mode'        => variable_get('commerce_elavon_server', 0) == 0 ? 'true' : 'false',
    'ssl_transaction_type' => "ccsale",
    'ssl_card_number'	     => $values['number'],
    'ssl_exp_date'		     => $values['exp_month'] . $values['exp_year'],
    'ssl_amount'	         => commerce_currency_amount_to_decimal($charge['amount'], commerce_default_currency()),

/* TO DO? everything else is optional and can be done in hook_commerce_elavon_transaction_alter
    'ssl_cvv2cvc2'		       => $values['code'],
    'ssl_cvv2cvc2_indicator' => isset($values['code']) ? '1' : null,
    'ssl_invoice_number'	   => $values['invoiceID'],
    'ssl_description'	       => $values['description'],
    'ssl_email'		           => $values['email'],
    'ssl_first_name'	       => $values['billing_first_name'],
    'ssl_last_name'		       => $values['billing_last_name'],
    'ssl_avs_address'	       => $values['street_address'],
    'ssl_city'		           => $values['city'],
    'ssl_state'		           => $values['state_province'],
    'ssl_avs_zip'		         => $values['postal_code'],
    'ssl_country'		         => $values['country'],
    'ssl_ship_to_first_name' => $values['first_name'],
    'ssl_ship_to_last_name'  => $values['last_name'],
    'ssl_ship_to_address1'   => $values['street_address'],
    'ssl_ship_to_address2'   => $values['street_address'],
    'ssl_ship_to_city'       => $values['city'],
    'ssl_ship_to_country'    => $values['country'],
    'ssl_ship_to_state'		   => $values['state_province'],
    'ssl_ship_to_zip'		     => $values['postal_code'],
*/
  );
  
  // Allow other modules to alter the transaction data
  drupal_alter('commerce_elavon_transaction', $txn_array, $values, $order );
  
  // convert to XML
  $request = _commerce_elavon_txnXML($txn_array);
  
  // use curl to send the transaction to Elavon
  $curl_options = array(
    CURLOPT_URL            => ($txn_array['ssl_test_mode']) ? COMMERCE_ELAVON_TEST_URL : COMMERCE_ELAVON_URL,
    CURLOPT_TIMEOUT        => COMMERCE_ELAVON_TIMEOUT,
    CURLOPT_RETURNTRANSFER => TRUE,
    CURLOPT_HEADER         => FALSE,
    CURLOPT_POST           => TRUE,
    CURLOPT_POSTFIELDS     => $request,
    CURLOPT_SSL_VERIFYPEER => FALSE,
//    CURLOPT_REFERER        => 
//    CURLOPT_SSL_VERIFYHOST => FALSE,
  );

  if (variable_get('commerce_elavon_verify_peer', FALSE)) {
    $curl_options[CURLOPT_SSL_VERIFYPEER] = TRUE;
    $curl_options[CURLOPT_CAINFO]         = variable_get('commerce_elavon_cert_path');
  }

  $ch = curl_init();
  curl_setopt_array($ch, $curl_options);
  $response = curl_exec($ch);
  $errorno  = curl_errno($ch);
  $errmsg   = curl_error($ch);
  curl_close($ch);

  if (!$response) {
    watchdog('commerce_elavon', 'Error with cURL request: (!error_no). Message: !error_message',
             array('!error_no' => $errorno, '!error_message' => $errmsg), WATCHDOG_ERROR);
    drupal_set_message(t('An unexpected system error has occured. Please contact the site administrator.'), 'error');
    return FALSE;
  }

  // Parse the response XML
  $response_data = _commerce_elavon_responseXML($response);

  // log all responses
  watchdog('commerce_elavon', 'Response: !param',
           array('!param' => '<pre>' . check_plain(print_r($response_data, TRUE)) . '</pre>'));

  $transaction->message = 'Elavon Result: @code <br />
                           Elavon Message: @message';
  $transaction->message_variables = array(
    '@code' => $response_data['ssl_result'],
    '@message' => $response_data['ssl_result_message'],
  );

  // set the remote ID to the Elavon transaction ID
  $transaction->remote_id = $response_data['ssl_txn_id'];
  $transaction->status = ($response_data['ssl_result'] == 0) ? COMMERCE_PAYMENT_STATUS_SUCCESS : COMMERCE_PAYMENT_STATUS_FAILURE;
  commerce_payment_transaction_save($transaction);

  // If the payment failed, display an error.
  if ($response_data['ssl_result'] != 0) {
    $msg = t('The following error occured processing your transaction. Please verify your information or contact us to resolve the error.');
    drupal_set_message($msg, 'error');
    drupal_set_message(check_plain($response_data['ssl_result_message']), 'error');
    return FALSE;
  }

  return TRUE;
}

/**
 * Generates the xml that will be sent to Evalon.
 *
 * @param Array $txn_array
 *   Data to send to Elavon.
 */
function _commerce_elavon_txnXML($txn_array) {
  $xml = '<txn>\n';
  foreach (txn_array as $name => $value) {
    if (isset($value)) {
      $xml .= "  <$name>$value</$name>\n";
    }
  }
  $xml .= '</txn>';
  return $xml;
}

/**
 * Generates an array from the xml returned by Evalon.
 *
 * @param String $response
 *   Data from Elavon.
 */
function _commerce_elavon_responseXML($response) {
  $xml = simplexml_load_string($response);
  return (array)$xml;
}
