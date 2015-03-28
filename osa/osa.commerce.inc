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
  
  // open the order record as read only: Issue 1514618
  $cart = commerce_cart_order_load($user->uid);
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

  // remove spaces from the card number
  if (isset($form['commerce_payment']['payment_details']['credit_card']['number'])) {
    $js = <<<EOD
jQuery(document).ready(function () {
  jQuery('#edit-commerce-payment-payment-details-credit-card-number').change(function() {
    var cc = jQuery('#edit-commerce-payment-payment-details-credit-card-number').val()
      .replace(/ /g, '')
      .replace(/-/g, '');
    jQuery('#edit-commerce-payment-payment-details-credit-card-number').val(cc);
  });
});
EOD;
    drupal_add_js($js, 'inline');
  }

  // re label the submit button
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
 * Implements hook_commerce_virtualmerchant_transaction_alter() to add custom variables to the transaction
 */
function osa_commerce_virtualmerchant_transaction_alter(&$post_values, $order) {
  $post_values['osa_qb_name'] = htmlspecialchars(_osa_qb_name(_osa_get_contact_id($order->uid)));
}

/**
 * Implements hook_commerce_elavon_transaction_alter() to add custom variables to the transaction
 */
function osa_commerce_elavon_transaction_alter( &$txn_array, $values, $order ) {
  $txn_array['osa_card_type'] = $values['type'];
  $txn_array['osa_name_card'] = htmlspecialchars($values['owner']);
  $txn_array['osa_qb_name'] = htmlspecialchars(_osa_qb_name(_osa_get_contact_id($order->uid)));
}
