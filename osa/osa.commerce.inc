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
  global $user;

  // add help to the email prompt for anonymous users
  if (!$user->uid && !empty($form['account']['login']['mail'])) {
    $form['account']['login']['mail']['#required'] = FALSE;
    $form['account']['login']['mail']['#description'] = t('Enter a valid e-mail address to receive your receipt.');
    $form['account']['#title'] = t('User information');
  }
  
  // re label the submit button
  $form['buttons']['continue']['#value'] = t('Pay Now');
  
  // override validation
  $form['buttons']['continue']['#validate'] = array('osa_commerce_checkout_form_validate');
}

/*
 * Override ananoymous email validation
 */
function osa_commerce_checkout_form_validate($form, &$form_state) {

  if (!empty($form_state['values']['account']['login']['mail'])) {
    if (!valid_email_address($form_state['values']['account']['login']['mail'])) {
      $form_state['values']['account']['login']['mail'] = '';
    }
  }

  commerce_checkout_form_validate($form, $form_state);
}

/**
 * Modify the completion message for anonymous orders with no email
 */
function osa_form_alter_commerce_checkout_form_complete(&$form, &$form_state) {
  $form['checkout_completion_message']['message']['#markup'] = str_replace('An email has been sent to  with a confirmation of your order.', '', $form['checkout_completion_message']['message']['#markup']);
}

/**
 * Implements hook_commerce_checkout_pane_info_alter() to not display the Checkout Login pane to logged-in users
 */
function osa_commerce_checkout_pane_info_alter(&$checkout_panes) {
  global $user;

  if ($user->uid && isset($checkout_panes['account_form'])) {
    unset($checkout_panes['account_form']);
  }
}

/**
 * Modify the checkout login form
 */
function osa_form_alter_commerce_checkout_form_account(&$form, &$form_state) {
  global $user;

  // replace help message without the register option
  $replacements = array(
    '%login' => t('Login & checkout'),
    '%guest' => t('Checkout as guest')
  );
  $form['account_form']['help']['#markup'] = t('If you have an existing account you can login by entering your login information in the form on the left and clicking the %login button or you can checkout as a guest by clicking the %guest button below.', $replacements);

  // remove the registration parts of the form
  if (isset($form['account_form']['select']['register'])) {
    unset($form['account_form']['select']['register']);
  }
}

/**
 * Implements hook_commerce_currency_info_alter() to not display the currency symbol (CAD)
 */
function osa_commerce_currency_info_alter(&$currencies, $langcode) {
  $currencies['CAD']['code_placement'] = 'hidden';
}

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
