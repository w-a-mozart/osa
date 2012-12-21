<?php

/**
 * @file
 * Functions that customize Drupal Commerce
 */
require_once 'osa.inc';

/**
 * Modify the cart edit form to remove the update cart button, if all of the products have a fixed quantity
 */
function osa_form_alter_views_form_commerce_cart_form_default(&$form, &$form_state) {

  global $user;
  $cart = commerce_cart_order_load($user->uid);
  // Unlock the order record: Issue 1514618
  entity_get_controller('commerce_order')->resetCache(array($cart->order_id));
  $non_civi_line_item_found = FALSE;

  if (!empty($cart)) {
    $order_wrapper = entity_metadata_wrapper('commerce_order', $cart);
    
    foreach ($order_wrapper->commerce_line_items as $delta => $line_item_wrapper) {
      if (in_array($line_item_wrapper->type->value(), commerce_product_line_item_types())) {
        // get the SKU.
        $line_item_sku = $line_item_wrapper->commerce_product->sku->value();
        if ($line_item_sku != 'CiviCRM') {
          $non_civi_line_item_found = TRUE;
          break;
        }
      }
    }
    
    if (!$non_civi_line_item_found) {
      unset($form['actions']['submit']);
    }
  }
}

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
