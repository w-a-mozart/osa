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
 * proxy-ing the request to Converge to display it's payment form.
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
    $order_id = $order->order_id;
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

  // check if we are using a dummy response
  if (variable_get('commerce_virtualmerchant_server', 0) != 2) {
    // create a post request to get the Converge Payment Form html
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

    // Post the request to Converge
    $http_response = drupal_http_request($url, $options);
    if (isset($http_response->error)) {
      _commerce_virtualmerchant_fatal_error("Error with HTTP request.\nResponse : " . print_r($http_response, TRUE));
    }

    // include javascript to validate email address of anonymous users
    $js = 'function doPinPad(){';
    if ($user->uid == 0) {
      $js .= "
              var emailInput = top.document.getElementById('edit-account-login-mail');
              if (emailInput && emailInput.value.indexOf('@') < 1) {
                emailInput.focus();
                emailInput.style.color = 'white';
                emailInput.style.backgroundColor = 'red';
                alert('You must enter a valid e-mail address to receive your receipt');
                return false;
              }
              document.getElementById('ssl_email').value = emailInput.value;
             ";
    }

    // hack the html returned by Converge, so it will work in the iframe
    $html = str_replace(array(
                          '<head>',
                          'name="form1"',
                          '<br/>',
                          'function doPinPad(){',
                        ),
                        array(
                          "<head><base href=\"{$url}\"></base>",
                          'name="form1" target=_top',
                          '',
                          $js,
                        ),
                        $http_response->data);

    // hide the ssl_email element
    $html = preg_replace('/<tr>\s*?.*?ssl_email">/', '<tr style="display:none;"><td><label for="ssl_email">', $html, 1);
  }
  else {
    // build a test form to create the dummy response
    $form = drupal_get_form('commerce_virtualmerchant_dummy_form');

    $form['#action'] .= "/{$order_id}";
    $form['ssl_amount']['#default_value'] = $amount;
    $form['ssl_amount']['#value'] = $amount;
    $html = '<!DOCTYPE html>'. drupal_render($form);
  }

  print $html;
  drupal_exit();
}

/**
 * Process the response from Converge's payment form
 *
 * Processing the response may take awhile, so we redirect to ourself,
 * so we can hide the URL query parameters
 */
function commerce_virtualmerchant_payment_response($order_id = 0) {
  global $base_url;

  // first time
  if (empty($_GET['_vmr'])) {
    // log all responses
    watchdog('commerce_virtualmerchant', 'Response: !1', array('!1' => '<pre>' . check_plain(print_r($_GET, TRUE)) . '</pre>'), WATCHDOG_INFO);
    
    // encode/encrypt the response variables
    $response_data = openssl_encrypt(drupal_json_encode($_GET), "aes-128-ctr", variable_get('commerce_virtualmerchant_pin', 'PIN'), 0, 'virtual_merchant');
    drupal_goto($base_url . '/' . request_path(), array('query' => array('_vmr' => $response_data)));
  }
  // second time, post the values to the checkout page to process
  else {
    $html = <<<EOD
<!DOCTYPE html>
<html>
<body>
<div id="virtualmerchant-processing" style="max-width: 400px; margin: 0 auto;"><img id="virtualmerchant-wait" style="max-width: 100%;" alt="Please Wait" src="/images/processing.gif" /></div>
<form action="{$base_url}/checkout/{$order_id}" method="post">
  <input type="hidden" name="_vmr" value="{$_GET['_vmr']}" />
  </form>
  <script type="text/javascript">
    document.forms[0].submit();
  </script>
</body>
</html>
EOD;
    print $html;
  }

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
  $form['#attributes']['target'] = '_top';  
  
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
  $form['ssl_email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
    '#default_value' => '',
    '#size' => 20,
    '#required' => FALSE,
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
