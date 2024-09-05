<?php

/**
 * @file
 * Drupal Commerce Payment method callbacks for Converge Virtual Merchant Payment Form.
 */
require_once 'commerce_virtualmerchant.inc';

/**
 * Payment method settings_form callback; generate the settings form
 * @param $settings
 *   An array of the current settings.
 * @return
 *   A form snippet.
 */
function commerce_virtualmerchant_settings_form($settings = NULL) {
  require_once 'commerce_virtualmerchant.admin.inc';
  return commerce_virtualmerchant_admin_settings();
}

/**
 * Payment method submit_form callback; generate the payment submission form.
 *
 * This method generates the Converge Virtual Merchant Payment Form
 *
 * @param $payment_method
 *   An array of the current settings.
 * @param $pane_values
 *   The current values of the pane.
 * @param $checkout_pane
 *   The checkout pane array. The checkout pane will be NULL if the payment is
 *   being added through the administration form.
 * @param $order
 *   The order object.
 * @return
 *   A form snippet for the checkout pane.
 */
function commerce_virtualmerchant_submit_form($payment_method, &$pane_values, &$checkout_pane, &$order) {

  // add a css and js
  $path = drupal_get_path('module', 'commerce_virtualmerchant');
  $form['virtualmerchant'] = [
    '#attached' => [
      'css' => [ $path . '/commerce_virtualmerchant.css' ],
      'js' => [ $path . '/commerce_virtualmerchant.js',
                'https://cdn.jsdelivr.net/npm/jquery-validation@1.19.5/dist/jquery.validate.js',
                'https://cdn.jsdelivr.net/npm/jquery-validation@1.19.5/dist/additional-methods.js',
      ],
    ],
  ];

  // have to add this like this, because Converge uses the URL
  $form['virtualmerchant']['script'] = ['#markup' => '<script src="https://api.convergepay.com/hosted-payments/Checkout.js"></script>' ];

  
  // create the form using the commerece default card fields
  module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');
  $fields = [ 'owner' => true ];
  $fields['type'] = ['mastercard', 'visa'];
  $form += commerce_payment_credit_card_form($fields);
  // pretend that the card type is required
  $form['credit_card']['type']['#title'] .= '<span class="form-required" title="This field is required.">*</span> <div class="cc-ico cc-visa"> </div><div class="cc-ico cc-mc"></div>';
  // change the label for card owner
  $form['credit_card']['owner']['#title'] = t('Name on Card');
  // don't reqiure the card number to be submitted (we don't want it), we will enforce this in javascript
  $form['credit_card']['number']['#required'] = false;
  $form['credit_card']['number']['#title'] .= '<span class="form-required" title="This field is required.">*</span>';

  // add hidden fields to be submitted after the payment is submitted to Converge
  $form['virtualmerchant']['token'] = ['#type' => 'hidden', '#value' => _commerce_virtualmerchant_get_token($order) ];
  $form['virtualmerchant']['balance']   = ['#type' => 'hidden', '#value' => _commerce_virtualmerchant_get_order_balance($order) ];
  $form['virtualmerchant']['vms']   = ['#type' => 'hidden', '#value' => $pane_values['payment_details']['virtualmerchant']['vms'] ];
  $form['virtualmerchant']['vmr']   = ['#type' => 'hidden', '#value' => $pane_values['payment_details']['virtualmerchant']['vmr'] ];
  $form['virtualmerchant']['#limit_validation_errors'] = false; 

  return $form;
}

/**
 * Payment method submit_form_validate callback; validate the payment response
 *
 * @param $payment_method
 *   An array of the current settings.
 * @param $pane_form
 *   The pane form.
 * @param $pane_values
 *   The current values of the pane.
 * @param $order
 *   The order object.
 * @param $form_parents
 *   The identifier of the base element of the payment pane.
 */
function commerce_virtualmerchant_submit_form_validate($payment_method, $pane_form, $pane_values, $order, $form_parents = array()) {

  $payment_details = $_POST['commerce_payment']['payment_details'];
  $_status   = $payment_details['virtualmerchant']['vms'];
  $_response = $payment_details['virtualmerchant']['vmr'];
  $valid = false;

  if ($_status == 'error') {
    watchdog('commerce_virtualmerchant', 'Response: !1', ['!1' => '<pre>' . check_plain($_response) . '</pre>'], WATCHDOG_ERROR);
    $response_data = drupal_json_decode($_response);
    form_set_error('credit_card', 'An unexpected error has occurred<br/>' . $response_data['ssl_result_message']);
  } elseif ($_status == 'declined') {
    watchdog('commerce_virtualmerchant', 'Response: !1', ['!1' => '<pre>' . check_plain($_response) . '</pre>'], WATCHDOG_INFO);
    $response_data = drupal_json_decode($_response);
    form_set_error('commerce_payment][payment_details][credit_card][number', 'Your card has been DECLINED<br/>' . $response_data['ssl_result_message']);
  } elseif ($_status == 'approved') {
    watchdog('commerce_virtualmerchant', 'Response: !1', ['!1' => '<pre>' . check_plain($_response) . '</pre>'], WATCHDOG_INFO);
    $valid = true;
  } else {
    watchdog('commerce_virtualmerchant', 'An unexpected error has occurred.', array(), WATCHDOG_ERROR);
    drupal_set_message('An unexpected error has occurred. Please contact your system administrator.', 'error');
    drupal_goto('/');
  }

  return $valid;
}

/**
 * Payment method submit_form_submit callback; process the payment submission
 * form.
 *
 * @param $payment_method
 *   An array of the current settings.
 * @param $pane_form
 *   The pane form.
 * @param $pane_values
 *   The current values of the pane.
 * @param $order
 *   The order object.
 * @param $charge
 *   A price structure that needs to be charged.
 */
function commerce_virtualmerchant_submit_form_submit($payment_method, $pane_form, $pane_values, $order, $charge) {

  // if we get here, the transaction was successful, so create a commerce transaction
  $payment_details = $_POST['commerce_payment']['payment_details'];
  $_status   = $payment_details['virtualmerchant']['vms'];
  $_response = $payment_details['virtualmerchant']['vmr'];

  $response_data = drupal_json_decode($_response);

  $currency = commerce_currency_load();
  $message = <<<EOD
CARD NUMBER: {$response_data['ssl_card_number']}
AMOUNT: {$response_data['ssl_amount']}
RESPONSE: {$response_data['ssl_result_message']}
APPROVAL CD: {$response_data['ssl_approval_code']}
EOD;

  $transaction = commerce_payment_transaction_new('commerce_virtualmerchant', $order->order_id);
  $transaction->instance_id    = $payment_method['instance_id'];
  $transaction->amount         = $response_data['ssl_amount'] * pow(10, $currency['decimals']);
  $transaction->currency_code  = $currency['code'];
  $transaction->remote_id      = $response_data['ssl_txn_id'];
  $transaction->remote_status  = $response_data['ssl_result'];
  $transaction->message        = $message;
  $transaction->status         = COMMERCE_PAYMENT_STATUS_SUCCESS;
  
  commerce_payment_transaction_save($transaction);

  return TRUE;
}
