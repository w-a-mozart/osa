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
function commerce_virtualmerchant_submit_form($payment_method, $pane_values, $checkout_pane, $order) {
  global $base_url;

  // include a hidden variable that will be used to return the response from Converge
  $form['virtualmerchant_response'] = array('#type' => 'hidden');

  // create an iframe that will hold the Converge Virtual Merchant Payment Form
  $height = variable_get('commerce_virtualmerchant_form_height', '300px');
  $width = variable_get('commerce_virtualmerchant_form_width', '100%');
  $form['virtualmerchant_iframe'] = array(
    '#markup' => "<iframe id=\"edit-commerce-payment-payment-details-virtualmerchant-iframe\" src=\"{$base_url}/commerce/virtualmerchant/form/{$order->order_id}\" width={$width} height={$height} scrolling=\"no\"></iframe>",
  );

  // add a div to display the Converge logo
  $form['virtualmerchant_logo'] = array(
    '#markup' => '<div id="virtualmerchant-banner">' . t('Payment processing provided by') . ' <a href="https://www.myvirtualmerchant.com" title="Converge" target="_blank"><img id="virtualmerchant-logo" alt="Converge" src="https://www.myvirtualmerchant.com/VirtualMerchant/assets/images/logos/ConvergeLogo-horizontal.png" /></a></div>',
  );

  return $form;
}

/**
 * Payment method submit_form_validate callback; validate the payment
 * submission form.
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
  $valid = TRUE;
  
  if (!empty($pane_values['virtualmerchant_response'])) {
    $response_data = drupal_json_decode($pane_values['virtualmerchant_response']);
  }
  else {
    $response_data = array(
      'errorCode' => 9999,
      'errorMessage' => t('An error occurred processing your transaction.  Please try again.'),
    );
  }

  // Fail on system error
  if (isset($response_data['errorCode']) && ($response_data['errorCode'] !== 0)) {
    form_set_error('virtualmerchant_iframe', $response_data['errorMessage']);
    $valid = FALSE;
  }
  // Fail on transaction processing error
  if (isset($response_data['ssl_result']) && (($response_data['ssl_result'] != 0) || ($response_data['ssl_result_message'] != "APPROVAL" && $response_data['ssl_result_message'] != 'SUCCESS'))) {
    form_set_error('virtualmerchant_iframe', t('An error occurred processing your transaction: !1', array( '!1' => check_plain($response_data['ssl_result_message']))));
    $valid = FALSE;
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
  // Should we create a commerce transaction on error? Don't see why.
  $response_data = drupal_json_decode($pane_values['virtualmerchant_response']);
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
