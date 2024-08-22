<?php

/**
 * @file
 * Helper functions for the Converge Virtual Merchant Payment Form interface.
 */

define('COMMERCE_VIRTUALMERCHANT_URL', 'https://api.convergepay.com/VirtualMerchant/process.do');
define('COMMERCE_VIRTUALMERCHANT_TEST_URL', 'https://api.demo.convergepay.com/VirtualMerchantDemo/process.do');
define('COMMERCE_VIRTUALMERCHANT_TOKEN_URL', 'https://api.convergepay.com/hosted-payments/transaction_token');
define('COMMERCE_VIRTUALMERCHANT_TEST_TOKEN_URL', 'https://api.demo.convergepay.com/hosted-payments/transaction_token');
define('COMMERCE_VIRTUALMERCHANT_TIMEOUT', '5');

/**
 * Sends a request to Converge to generate a one-time token
 */
function _commerce_virtualmerchant_get_token($order) {

  // get the amount to pay
  $order_balance = commerce_payment_order_balance($order);
  if ($order_balance['currency_code'] != commerce_default_currency()) {
    $order_balance['amount'] = commerce_currency_convert($order_balance['amount'], $order_balance['currency_code'], commerce_default_currency());
    $order_balance['currency_code'] = commerce_default_currency();
  }
  $currency = commerce_currency_load();
  $amount = number_format(commerce_currency_amount_to_decimal($order_balance['amount'], $order_balance['currency_code']), $currency['decimals'], $currency['decimal_separator'], '');

  $pre_alter_post_values = [
    'ssl_test_mode'           => variable_get('commerce_virtualmerchant_server', 0) == 0 ? 'true' : 'false',
    'ssl_invoice_number'      => $order->order_id,
  ];

  // Allow other modules to alter the post data
  drupal_alter('commerce_virtualmerchant_transaction', $pre_alter_post_values, $order);

  $post_values = [
    'ssl_account_id'          => variable_get('commerce_virtualmerchant_merchant_id', ''),
    'ssl_user_id'             => variable_get('commerce_virtualmerchant_user_id', ''),
    'ssl_pin'                 => variable_get('commerce_virtualmerchant_pin', ''),
    'ssl_transaction_type'    => 'ccsale',
    'ssl_amount'              => $amount,
  ];
  $post_values = array_merge($pre_alter_post_values, $post_values);

  $url = ($post_values['ssl_test_mode'] == 'true') ? COMMERCE_VIRTUALMERCHANT_TEST_TOKEN_URL : COMMERCE_VIRTUALMERCHANT_TOKEN_URL;
  $options = [
    'headers' => ['Content-Type' => 'application/x-www-form-urlencoded'],
    'method'  => 'POST',
    'data'    => urldecode(http_build_query($post_values)),
    'timeout' => COMMERCE_VIRTUALMERCHANT_TIMEOUT,
  ];

  // post the request to Converge
  $http_response = drupal_http_request($url, $options);
  if (isset($http_response->error)) {
    _commerce_virtualmerchant_fatal_error("Error with HTTP request.\nResponse : " . print_r($http_response, TRUE));
  }

  $token = $http_response->data;
  return $token;
}

function _commerce_virtualmerchant_fatal_error($error_msg) {
  watchdog('commerce_virtualmerchant', $error_msg, array(), WATCHDOG_ERROR);
  drupal_set_message('An unexpected error has occurred. Please contact your system administrator.', 'error');
  drupal_goto('/');
}
