<?php

/**
 * @file
 * Functions that customize Drupal Commerce
 */
require_once 'osa.inc';

/**
 * Modify the checkout form
 */
function osa_form_alter_commerce_checkout_form_checkout(&$form, &$form_state) {

  // all this just to set the default payment type on Commerce Checkout
  if (!$form_state['rebuild']) {
    if ($form['commerce_payment']['payment_method']['#default_value'] != 'commerce_elavon|commerce_payment_commerce_elavon') {
      $form['commerce_payment']['payment_method']['#default_value'] = 'commerce_elavon|commerce_payment_commerce_elavon';
      $payment_method = commerce_payment_method_load('commerce_elavon');
      if ($callback = commerce_payment_method_callback($payment_method, 'submit_form')) {
        $form['commerce_payment']['payment_details'] = $callback($payment_method, array(), $form['commerce_payment'], $form_state['order']);
      }
      $form['commerce_payment']['payment_details']['#prefix'] = '<div id="payment-details">';
      $form['commerce_payment']['payment_details']['#suffix'] = '</div>';
    }
  }
  
  $form['buttons']['continue']['#value'] = t('Pay Now');
}

/**
 * Implements hook_commerce_currency_info_alter() to not display the currency symbol (CAD)
 */
function osa_commerce_currency_info_alter(&$currencies, $langcode) {
  $currencies['CAD']['code_placement'] = 'hidden';
}

/**
 * Implements hook_commerce_price_formatted_components_alter()

function osa_commerce_price_formatted_components_alter(&$components, &$item, &$entity) {
  foreach ($components as $component) {
  }
}
 */

/**
 * Implements hook_commerce_elavon_transaction_alter() to add custom variables to the transaction
 */
function osa_commerce_elavon_transaction_alter( &$txn_array, $values, $order ) {
  $txn_array['osa_card_type'] = $values['type'];
  $txn_array['osa_name_card'] = $values['owner'];
}
