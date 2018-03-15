<?php

/**
 * @file
 * Helper functions for the Converge Virtual Merchant Payment Form interface.
 */

define('COMMERCE_VIRTUALMERCHANT_URL', 'https://api.convergepay.com/VirtualMerchant/process.do');
define('COMMERCE_VIRTUALMERCHANT_TEST_URL', 'https://api.demo.convergepay.com/VirtualMerchantDemo/process.do');
define('COMMERCE_VIRTUALMERCHANT_TIMEOUT', '5');

/**
 * Fills the iFrame created in commerce_virtualmerchant_submit_form by
 * proxy-ing the request to Converge's to display it's payment form.
 * This way, the iframe content is from the same domain as it's host page, and
 * we don't send our Converge credentials to the browser.
 *
 * Also, by using the Converge payment form, no card information is sent to
 * our server, so we are out of scope for PCI compliance.
 */
function commerce_virtualmerchant_payment_form($order_id = 0) {
  global $base_url, $user;
  
  // need the order to get the order balance to pay
  // default to the current user's shopping cart
  if ($order_id == 0) {
    $order = commerce_cart_order_load($user->uid);
  }
  else {
    $order = commerce_order_load($order_id);
  }

  // should never happen
  if ($order == FALSE) {
    _commerce_virtualmerchant_fatal_error("Error loading order. Order Number {$order_id} not found");
  }

  // get the amount to pay
  $order_balance = commerce_payment_order_balance($order);
  if ($order_balance['currency_code'] != commerce_default_currency()) {
    $order_balance['amount'] = commerce_currency_convert($order_balance['amount'], $order_balance['currency_code'], commerce_default_currency());
    $order_balance['currency_code'] = commerce_default_currency();
  }
  $currency = commerce_currency_load();
  $amount = number_format(commerce_currency_amount_to_decimal($order_balance['amount'], $order_balance['currency_code']), $currency['decimals'], $currency['decimal_separator'], '');

  // check if using a dummy response
  if (variable_get('commerce_virtualmerchant_server', 0) != 2) {
    // post a request to Converge for the Payment Form html
    $post_values = array(
      'ssl_merchant_id'         => variable_get('commerce_virtualmerchant_merchant_id', ''),
      'ssl_user_id'             => variable_get('commerce_virtualmerchant_user_id', ''),
      'ssl_pin'                 => variable_get('commerce_virtualmerchant_pin', ''),
      'ssl_test_mode'           => variable_get('commerce_virtualmerchant_server', 0) == 0 ? 'true' : 'false',
      'ssl_transaction_type'    => "ccsale",
      'ssl_show_form'           => 'true',
      'ssl_amount'              => $amount,
      'ssl_receipt_link_method' => 'REDG',
      'ssl_receipt_link_url'    => $base_url . '/commerce/virtualmerchant/response/' . $order_id,
    );

    // Allow other modules to alter the post data
    drupal_alter('commerce_virtualmerchant_transaction', $post_values, $order);

    $url = ($post_values['ssl_test_mode'] == 'true') ? COMMERCE_VIRTUALMERCHANT_TEST_URL : COMMERCE_VIRTUALMERCHANT_URL;
    $options = array(
      'headers' => array('Content-Type' => 'application/x-www-form-urlencoded'),
      'method'  => 'POST',
      'data'    => urldecode(http_build_query($post_values)),
      'timeout' => COMMERCE_VIRTUALMERCHANT_TIMEOUT,
    );

    $http_response = drupal_http_request($url, $options);
    if (isset($http_response->error)) {
      _commerce_virtualmerchant_fatal_error("Error with HTTP request.\nResponse : " . print_r($http_response, TRUE));
    }
    
    $html = str_replace(array('<head>', '<br/>'), array("<head><base href=\"{$url}\"></base>"), $http_response->data);
  }
  else {
    // build a form to create the dummy response
    $form = drupal_get_form('commerce_virtualmerchant_dummy_form');
    $form['ssl_amount']['#default_value'] = $amount;
    $form['ssl_amount']['#value'] = $amount;
    $html = '<!DOCTYPE html>'. drupal_render($form);
  }

  print $html;
  drupal_exit();
}

/**
 * Process the response from Converge's payment form
 */
function commerce_virtualmerchant_payment_response($order_id = 0) {

  // log all responses
  watchdog('commerce_virtualmerchant', 'Response: !1', array('!1' => '<pre>' . check_plain(print_r($_GET, TRUE)) . '</pre>'), WATCHDOG_INFO);

  $response = drupal_json_encode($_GET);
  $html = <<<EOD
<html>
  <body>
    <br/>
    <br/>
    <h2>Please wait...</h2>
  </body>
  <script type="text/javascript">
    window.parent.document.forms['commerce-checkout-form-checkout'].elements['commerce_payment[payment_details][virtualmerchant_response]'].value = '{$response}';
    window.parent.document.forms['commerce-checkout-form-checkout'].elements['edit-continue'].click();
  </script>
</html>
EOD;

  print $html;
  drupal_exit();
}

function _commerce_virtualmerchant_fatal_error($error_msg) {
  watchdog('commerce_virtualmerchant', $error_msg, array(), WATCHDOG_ERROR);

  $html = <<<EOD
<!DOCTYPE html>
<html>
<body>
  <p>An unexpected error has occurred. Please contact your system administrator.</p>
</body>
</html>
EOD;
  print $html;
  drupal_exit();
}

function commerce_virtualmerchant_dummy_form() {
  global $base_url;
  $now = date('Y/m/d h:i:s A');

  $form = array();
  $form['#type'] = 'form';
  $form['#action'] = $base_url . '/commerce/virtualmerchant/response';
  $form['#method'] = 'get';
  
  $form['ssl_card_number'] = array(
    '#type' => 'textfield', 
    '#title' => t('Card Number'), 
    '#default_value' => '41**********1111', 
    '#size' => 20, 
    '#required' => TRUE,
  );
  $form['ssl_exp_date'] = array(
    '#type' => 'textfield', 
    '#title' => t('Exp Date'), 
    '#default_value' => '0120', 
    '#size' => 20, 
    '#required' => TRUE,
  );
  $form['ssl_amount'] = array(
    '#type' => 'textfield', 
    '#title' => t('Amount'), 
    '#default_value' => '1.00',
    '#size' => 20, 
    '#required' => TRUE,
  );
  $form['ssl_result'] = array(
    '#type' => 'textfield', 
    '#title' => t('Result Code'), 
    '#default_value' => 0, 
    '#size' => 20, 
    '#required' => TRUE,
  );
  $form['ssl_result_message'] = array(
    '#type' => 'textfield', 
    '#title' => t('Result Message'), 
    '#default_value' => 'APPROVAL', 
    '#size' => 20, 
    '#required' => TRUE,
  );
  $form['ssl_txn_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Transaction Id'),
    '#default_value' => strtoupper(hash('crc32b', $now)),
    '#size' => 20,
    '#required' => TRUE,
  );
  $form['ssl_approval_code'] = array(
    '#type' => 'textfield',
    '#title' => t('Approval Code'),
    '#default_value' => 'AA',
    '#size' => 20,
    '#required' => TRUE,
  );
  $form['ssl_account_balance'] = array(
    '#type' => 'hidden',
    '#value' => 0,
  );
  $form['ssl_txn_time'] = array(
    '#type' => 'hidden',
    '#value' => $now,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}
