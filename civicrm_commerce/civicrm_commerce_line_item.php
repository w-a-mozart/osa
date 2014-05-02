<?php
/**
 * @file 
 * line items for CiviCRM items
 * - modified from line_item_example.module
 */
require_once 'civicrm_commerce.inc';

/**
 * Implements hook_commerce_line_item_type_info().
 *
 * - add line item types for memberships, events, and other contributions
 * - also add support for member discounts on events (this might be better in a general discount module)
 *
 * @see hook_commerce_line_item_type_info().
 * @see http://www.drupalcommerce.org/specification/info-hooks/line-item
 *
 */
function civicrm_commerce_commerce_line_item_type_info() {

  $line_item_types['civi_membership'] = array(
    'name' => t('Membership'),
    'description' => t('A line item type, for CiviCRM Memberships'),
    // we use a generic "CiviCRM" product
    'product' => TRUE,
    'add_form_submit_value' => t('Add Membership'),
    'base' => 'civicrm_commerce_line_item',
  );

  $line_item_types['civi_event'] = array(
    'name' => t('Event'),
    'description' => t('A line item type, for CiviCRM Events'),
    'product' => TRUE,
    'add_form_submit_value' => t('Add Event'),
    'base' => 'civicrm_commerce_line_item',
  );
  
  $line_item_types['civi_contribution'] = array(
    'name' => t('Contribution'),
    'description' => t('A line item type, for CiviCRM Contributions'),
    'product' => TRUE,
    'add_form_submit_value' => t('Add Contribution'),
    'base' => 'civicrm_commerce_line_item',
  );

  return $line_item_types;
}


/**
 * Configure each line item with required fields.
 *
 * This function is called by the line item module when it is enabled or this
 * module is enabled.
 *
 * @param $line_item_type
 *   The info array of the line item type being configured.
 *
 * @see commerce_product_line_item_configuration()
 */
function civicrm_commerce_line_item_configuration($line_item_type) {
  $type = $line_item_type['type'];

  // add the type specific fields to the line item to link to the CiviCRM items
  switch ($type) {
    case "civi_membership":
      $field_names = array('contribution_id', 'amount', 'currency_code', 'contact_id', 'membership_id', 'mem_type_id');
    break;
    case "civi_event":
      $field_names = array('contribution_id', 'amount', 'currency_code', 'contact_id', 'event_id', 'participant_id');
    break;
    case "civi_contribution":
      $field_names = array('contribution_id', 'amount', 'currency_code', 'contact_id');
    break;
  }
  
  if (isset($field_names)) {
    _civicrm_commerce_create_fields($field_names, 'commerce_line_item', $type);
  }
}

/**
 * Returns a title for this line item.
 */
function civicrm_commerce_line_item_title($line_item) {
  $type = $line_item->type;
  $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
  if (isset($line_item_wrapper->civicrm_commerce_contribution_id)) {
    $contribution_id = $line_item_wrapper->civicrm_commerce_contribution_id->value();
    civicrm_initialize();
    $result = civicrm_api('contribution', 'get', array('id' => $contribution_id, 'version' => 3));
    $title = $result['values'][$contribution_id]['financial_type'];
  }
  else {
    $title = '';
  }

  return($title);
}
 
/**
 * Returns the elements necessary to add a line item through a line item manager widget (on the order form).
 */
function civicrm_commerce_line_item_add_form($element, &$form_state) {

  $form = array();
  $type = $form_state['input']['commerce_line_items']['und']['actions']['line_item_type'];

  $instances = field_info_instances('commerce_line_item', $type);
  
  foreach($instances as $instance => $info) {
    if (substr($instance, 0, 16) == 'civicrm_commerce') {
      $form[$instance] = array(
        '#type' => 'textfield',
        '#title' => $info['label'],
        '#description' => $info['description'],
        '#size' => 60,
        '#maxlength' => 255,
      );
    }
  }

  if (array_key_exists('civicrm_commerce_currency_code', $form)) {
    $form['civicrm_commerce_currency_code']['#default_value'] = commerce_default_currency();
  }
  
  return $form;
}

/**
 * Adds the selected information to a line item added via a line item manager widget (on the admin order page).
 *
 * @param $line_item
 *   The newly created line item object.
 * @param $element
 *   The array representing the widget form element.
 * @param $form_state
 *   The present state of the form upon the latest submission.
 * @param $form
 *   The actual form array.
 *
 * @return
 *   NULL if all is well or an error message if something goes wrong.
 */
function civicrm_commerce_line_item_add_form_submit(&$line_item, $element, &$form_state, $form) {

  $type = $line_item->type;
  $amount = $form_state['input']['commerce_line_items']['und']['actions']['civicrm_commerce_amount'];
  $currency_code = $form_state['input']['commerce_line_items']['und']['actions']['civicrm_commerce_currency_code'];
  $title = civicrm_commerce_line_item_title($line_item);

  // Set field information.
  $line_item->line_item_label = $title;
  $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
  $line_item->commerce_unit_price = array('und' => array('0' => array('amount' => $amount * 100, 'currency_code' => $currency_code)));

  if (!is_null($line_item_wrapper->commerce_unit_price->value())) {
    // Add the base price to the components array.
    if (!commerce_price_component_load($line_item_wrapper->commerce_unit_price->value(), 'base_price')) {
      $line_item_wrapper->commerce_unit_price->data = commerce_price_component_add($line_item_wrapper->commerce_unit_price->value(),
                                                                                    'base_price',
                                                                                    $line_item_wrapper->commerce_unit_price->value(),
                                                                                    TRUE
                                                                                  );
    }
  }
}

/**  
 * Call by the Payment Processor to add new line items to the Drupal Commerce Cart for later checkout
 *  
 * @param array $params  name value pair of contribution data
 *  
 * @return void  
 * @access public 
 *  
 */  
function civicrm_commerce_line_item_add_new($paymentObj, &$params, $component) {

  // define an array of the params required to build a line item
  $line_item_params = $params;
  
  // load the contact record to get the display name
  if (isset($params['contactID'])) {
    $result = civicrm_api('contact', 'get', array('id' => $params['contactID'], 'version' => 3));
    $contact = $result['values'][$params['contactID']];
  }
  else {
    $contact['display_name'] = '';
  }

  // get the name of the contribution type if it is not passed
  if (empty($params['contributionType_name']) && !empty($params['contributionTypeID'])) {
    $params['contributionType_name'] = CRM_Core_DAO::getFieldValue('CRM_Financial_DAO_FinancialType', $params['contributionTypeID']);
  }
  
  // determine the line item type to create
  if ($component == "contribute") {
    if (isset($params['membershipID'])) {
      $line_item_params['label'] = $params['contributionType_name'] . ' for ' . $contact['display_name'];
      $line_item_params['type'] = 'civi_membership';
    }
    else {
      $result = civicrm_api('ContributionPage', 'getvalue', array('id' => $params['contributionPageID'], 'return' => 'title', 'version' => 3));
      $line_item_params['label'] = $result . ' for ' . $contact['display_name'];
      $line_item_params['type'] = 'civi_contribution';
    }
  }
  elseif ($component == "event") {
    $line_item_params['type'] = 'civi_event';
    if (isset($params['participantID'])) {
      $result = civicrm_api('participant', 'get', array('id' => $params['participantID'], 'version' => 3));
      $line_item_params['label'] = $result['values'][$params['participantID']]['event_title'];
      $line_item_params['label'] .= ' (' . $result['values'][$params['participantID']]['display_name']; 

      if ((isset($result['values'][$params['participantID']]['participant_fee_level'])) &&
          (!is_array($result['values'][$params['participantID']]['participant_fee_level']))){
        $line_item_params['label'] .= ' : ' . $result['values'][$params['participantID']]['participant_fee_level'];
      }
      $line_item_params['label'] .= ')';
    }
    else {
      $line_item_params['label'] = CRM_Utils_Array::value('item_name', $params);
      $line_item_params['label'] .= ' (' . $contact['display_name'] . ')';
    }
  }
  else {
    CRM_Core_Error::fatal(ts('Unknown shopping cart item.'));
  }
  
  // allow manipulation of the line item parameters
  $line_item_params['quantity'] = 1;
  CRM_Utils_Hook::alterPaymentProcessorParams($paymentObj, $params, $line_item_params);

  // @TODO move this into hook_alterPaymentProcessorParams
  if ($line_item_params['eventID'] == 1019) {
    $line_item_params['quantity'] = ($line_item_params['amount'] / 12);
    $line_item_params['amount'] = 12;
  }

  // load the "dummy" civicrm product
  $product = commerce_product_load_by_sku(CIVICRM_COMMERCE_PRODUCT_SKU);
  // change the products price to ensure a base price is added to the line item
  // this will be overwritten by the pricing rule
  $product_wrapper = entity_metadata_wrapper('commerce_product', $product);
  $product_wrapper->title = $params['contributionType_name'] . ' for ' . $contact['display_name'];
  $product_wrapper->commerce_price->amount = $line_item_params['amount'];
  $product_wrapper->commerce_price->currency_code = $line_item_params['currencyID'];

  // create the line item and set its attributes
  $line_item = commerce_product_line_item_new($product, $line_item_params['quantity'], 0, $line_item_params, $line_item_params['type']);
  $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);

  $line_item_wrapper->line_item_label = $line_item_params['label'];
  $line_item_wrapper->civicrm_commerce_amount = $line_item_params['amount'];
  $line_item_wrapper->civicrm_commerce_currency_code = $line_item_params['currencyID'];

  if ($line_item_params['contributionID']) {
    $line_item_wrapper->civicrm_commerce_contribution_id = $line_item_params['contributionID'];
  }
  if ($line_item_params['membershipID']) {
    $line_item_wrapper->civicrm_commerce_membership_id = $line_item_params['membershipID'];
  }
  if ($line_item_params['contactID']) {
    $line_item_wrapper->civicrm_commerce_contact_id = $line_item_params['contactID'];
  }
  if ($line_item_params['eventID']) {
    $line_item_wrapper->civicrm_commerce_event_id = $line_item_params['eventID'];
  }
  if ($line_item_params['participantID']) {
    $line_item_wrapper->civicrm_commerce_participant_id = $line_item_params['participantID'];
  }

  // special code to fix the Membership on renewal
  if ($line_item_params['membershipID']) {
    $membership_id = $line_item_params['membershipID'];
    $result = civicrm_api('membership', 'get', array('id' => $membership_id, 'version' => '3'));
    $membership = $result['values'][$membership_id];
    if (isset($membership['join_date'])) {
      $renew_dates = CRM_Member_BAO_MembershipType::getRenewalDatesForMembershipType($membership_id);
      $result = civicrm_api('membership', 'create', array('id' => $membership_id, 'status_id' => 5, 'version' => '3') + $renew_dates);
    }
  }

  // add it to the current user's cart
  $uid = CRM_Utils_System::getLoggedInUfID();
  return commerce_cart_product_add($uid, $line_item, FALSE);
}
