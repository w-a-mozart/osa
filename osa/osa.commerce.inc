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
 * Override anonymous email validation
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

/**
 * Implements hook_civicrm_alterPaymentProcessorParams to customize payment processing
 *
 */
function osa_civicrm_alterPaymentProcessorParams($paymentObj, &$rawParams, &$cookedParams) {
  // customize private lesson payments
  if (isset($rawParams['contributionPageID']) && ($rawParams['contributionPageID'] == OSA_CONTRIB_TEACHER)) {
    osa_civicrm_alterPaymentProcessorParams_teacher_registration($paymentObj, $rawParams, $cookedParams);
  }

  // add custom variables to the Elavon transaction
  if (isset($paymentObj) && ($paymentObj->_processorName == 'Elavon')) {
    $contact = _osa_get_contact(NULL, FALSE);
    $cookedParams['osa_name_card'] = htmlspecialchars($contact['display_name']);
    $cookedParams['osa_card_type'] = strtolower($rawParams['credit_card_type']);
    $cookedParams['osa_qb_name'] = htmlspecialchars(_osa_qb_name($contact['id']));
    $cookedParams['ssl_description'] = htmlspecialchars($cookedParams['ssl_description']);
    
    unset($cookedParams['ssl_ship_to_first_name']);
    unset($cookedParams['ssl_ship_to_last_name']);
    unset($cookedParams['ssl_customer_number']);
  }

  // modify the line item description for Event registrations
  if (isset($paymentObj) && ($paymentObj->_processorName == 'Drupal Commerce Cart') && ($cookedParams['type'] == 'civi_event')) {
    $participantID = $rawParams['participantID'];
    $eventID = $rawParams['eventID'];
    $event = civicrm_api('Event', 'getsingle', array('id' => $eventID, 'version' => 3,));
    if ($event['event_type_id'] != OSA_EVENT_SUMMER_CAMP) {
      $html = _osa_civicrm_participant_theme($participantID, 'short');
      $cookedParams['label'] = trim($html);
    }
  }
  // modify the line item description for Memberships
  elseif (isset($paymentObj) && ($paymentObj->_processorName == 'Drupal Commerce Cart') && ($cookedParams['type'] == 'civi_membership')) {
    // add next year's school year to the title
    $contactID = $rawParams['contactID'];
    $membership = _osa_get_membership($contactID);
    if (isset($membership['end_date']) && $membership['is_current_member']) {
      $end_date = new DateTimeEx($membership['end_date']);
    }
    else {
      $membershipDates = _osa_get_membership_dates();
      $end_date = new DateTimeEx($membershipDates['end_date']);
      // need a date in the middle of the school year as the membership end date (August) is after the school end date (June)
      $end_date->modify('-6 months');
    }
    $school_year = _osa_school_year($school_start_date, $school_end_date, $end_date->format('Y-m-d'));
    $cookedParams['label'] = trim("$school_year {$cookedParams['label']}");
  }

  // @TODO fix this
  if (($rawParams['eventID'] == 1019) || ($rawParams['eventID'] == 2191)) {
    $cookedParams['quantity'] = ($cookedParams['amount'] / 12);
    $cookedParams['amount'] = 12;
  }  
}

/**
 * Implements hook_commerce_price_component_type_info.
 */
function osa_commerce_price_component_type_info() {
  $component_types = array();

  // Add custom discount types
  $discount_types = variable_get('osa_discount_types');
  $types = empty($discount_types) ? array() : explode(PHP_EOL, trim($discount_types));

  foreach ($types as $idx => $type) {
    $type = trim($type);
    if (!empty($type)) {
      $component_types["osa_discount_$idx"] = array(
        'title' => $type,
        'parent_type' => 'discount',
        'weight' => -40 + $idx,
      );
    }
  }

  return $component_types;
}

/**
 * Implements hook_commerce_price_formatted_components_alter.
 *
 * Change the way Order totals are displayed to consolidate discounts.
 */
function osa_commerce_price_formatted_components_alter(&$components, $price, $entity) {
  if (isset($entity->commerce_order_total)) {
    foreach ($components as $type => $component) {
      $component_type = commerce_price_component_type_load($type);
      if (isset($component_type['parent_type'])) {
        $parent_type = commerce_price_component_type_load($component_type['parent_type']);
        $components[$component_type['parent_type']]['title'] = $parent_type['display_title'];
        $components[$component_type['parent_type']]['weight'] = $parent_type['weight'];
        if (!isset($components[$component_type['parent_type']]['price'])) {
          $components[$component_type['parent_type']]['price'] = commerce_price_field_data_auto_creation();
        }
        $components[$component_type['parent_type']]['price']['amount'] += commerce_currency_convert($component['price']['amount'], $component['price']['currency_code'], $components[$component_type['parent_type']]['price']['currency_code']);
        unset($components[$type]);
      }
    }
  }
}

/**
 * Add civicrm elements to shopping cart
 */
// @TODO : reuse this for camp and other places and/or move to civicrm_commerce module
function osa_add_to_commerce_cart($type, $element, $contact, $contribution) {
  static $line_item_type_map = null;
  if (empty($line_item_type_map))
    $line_item_type_map = osa_commerce_line_item_type_map();

  // create a product object
  $product = commerce_product_load_by_sku('CiviCRM');
  $product_wrapper = entity_metadata_wrapper('commerce_product', $product);
  $product_wrapper->title = $type;
  $product_wrapper->commerce_price->amount = $contribution['total_amount'];
  $product_wrapper->commerce_price->currency_code = $contribution['currency'];

  // create a line item object
  $line_item = commerce_product_line_item_new($product, 1, 0, $element, $line_item_type_map[$type]);
  $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
  $line_item_wrapper->civicrm_commerce_amount = $contribution['total_amount'];
  $line_item_wrapper->civicrm_commerce_currency_code = $contribution['currency'];
  $line_item_wrapper->civicrm_commerce_contribution_id = $contribution['id'];
  $line_item_wrapper->civicrm_commerce_contact_id = $contact['id'];

  if ($type == 'Membership') {
    $line_item_wrapper->line_item_label = "{$element['school_year']} Membership for {$contact['display_name']}";
    $line_item_wrapper->civicrm_commerce_membership_id = $element['id'];
    $line_item_wrapper->civicrm_commerce_mem_type_id = $element['membership_type_id'];   
  }
  elseif ($type == 'Event') {
    if (isset($element['event_title'])) {
      $event_title = $element['event_title'];
    } else {
      $event_title = civicrm_api3('Event', 'getvalue', ['return' => 'title', 'id' => $element['event_id'],]);
    }
    $line_item_wrapper->line_item_label = (!empty($event_title) ? $event_title : 'Event participation') . (!empty($element['fee_level']) ? " - {$element['fee_level']}, " : ' ') . "for {$contact['display_name']}";
    $line_item_wrapper->civicrm_commerce_participant_id = $element['id'];
    $line_item_wrapper->civicrm_commerce_event_id = $element['event_id'];
    $line_item_wrapper->commerce_display_path = "civicrm/event/info?reset=1&id={$element['event_id']}";
  }
  else {
    $line_item_wrapper->line_item_label = $contribution['source'];
  }

  return commerce_cart_product_add(_osa_getUserId(), $line_item, FALSE);
}

// should call civicrm_commerce_commerce_line_item_type_info, but hard code for now
function osa_commerce_line_item_type_map() {
  return [
    'Membership'   => 'civi_membership',
    'Event'        => 'civi_event',
    'Contribution' => 'civi_contribution',
  ];
}
