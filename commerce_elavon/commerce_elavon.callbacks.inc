<?php

/**
 * @file
 * Drupal Commerce Payment method callbacks for Elavon Payment Gateway.
 */
require_once 'commerce_elavon.inc';

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
function commerce_elavon_submit_form($payment_method, $pane_values, $checkout_pane, $order) {

  $fields = drupal_map_assoc(variable_get('commerce_elavon_optional_fields', array()));
  $types  = drupal_map_assoc(variable_get('commerce_elavon_credit_card_types', array()));
  unset($types[0]);
  
  if ( array_key_exists('type', $fields) ) {
    $fields['type'] = $types;
  }

  module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');
  $form = commerce_payment_credit_card_form($fields);
  
  // my preference, move to hook form alter later
  $form['credit_card']['owner']['#title'] = t('Name on Card');

  return $form;
}

/**
 * Payment method submit_form_validate callback; validate the payment submission form.
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
function commerce_elavon_submit_form_validate($payment_method, $pane_form, $pane_values, $order, $form_parents = array()) {
  $details = $pane_values['credit_card'];
  $settings['form_parents'] = array_merge($form_parents, array('credit_card'));
  
  module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');
  return commerce_payment_credit_card_validate($details, $settings);
}

/**
 * Payment method submit_form_submit callback; process the payment submission form.
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
function commerce_elavon_submit_form_submit($payment_method, $pane_form, $pane_values, $order, $charge) {
  //$order->data['commerce_elavon'] = $pane_values;
  return commerce_elavon_transaction($payment_method, $pane_values['credit_card'], $order, $charge );
}
