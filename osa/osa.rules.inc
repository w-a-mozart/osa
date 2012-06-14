<?php

/**
 * @file
 * Rules integration for the OSA.
 */

require_once 'osa.inc';
 
/**
 * Implements hook_rules_condition_info()
 */
function osa_rules_condition_info() {
  $conditions = array();

  $conditions['osa_household_members'] = array(
    'label' => t('Contact\'s household has other members'),
    'parameter' => array(
      'commerce_line_item' => array(
        'type' => 'commerce_line_item',
        'label' => t('Line item'),
        'description' => t('The line item that contains a civicrm_commerce_contact_id whose household to check for members.'),
      ),
      'operator' => array(
        'type' => 'text',
        'label' => t('Operator'),
        'description' => t('The operator used with the quantity value below to compare with the number of household members.'),
        'default value' => '>=',
        'options list' => 'commerce_numeric_comparison_operator_options_list',
        'restriction' => 'input',
      ),
      'value' => array(
        'type' => 'text',
        'label' => t('Quantity'),
        'default value' => '1',
        'description' => t('The value to compare against the number of household members.'),
      ),
    ),
    'group' => t('OSA'),
    'module' => 'osa',
  );

  return $conditions;
}

/**
 * Rules condition: contact's household has other members
 */
function osa_household_members($commerce_line_item, $operator, $value) {
  $number_members = 0;
  
  $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $commerce_line_item);
  if (!isset($line_item_wrapper->civicrm_commerce_contact_id)) {
    // must get a valid contact id
    return FALSE;
  }

  $contact_id = $line_item_wrapper->civicrm_commerce_contact_id->value();
  civicrm_initialize();
  $contact = _osa_getContact($contact_id, FALSE);
  if ((!isset($contact)) || ($contact['contact_type'] <> 'Individual')) {
    // must get a valid Individual contact
    return FALSE;
  }
    
  // get the household
  $hid = _osa_getHousehold($contact_id);
  if (!isset($hid)) {
    // person must belong to a household
    return FALSE;
  }

  // get the contact's membership record
  $membership = _osa_get_membership($contact_id);

  // we don't want to count memberships that come after this one in the shopping cart
  $order = commerce_order_load($commerce_line_item->order_id);
  $ignore_membership_ids = array();
  if ($order) {
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
    foreach ($order_wrapper->commerce_line_items as $delta => $order_line_item_wrapper) {
      if ($order_line_item_wrapper->line_item_id->value() <= $line_item_wrapper->line_item_id->value()) {
        continue;
      }
      if ((isset($order_line_item_wrapper->civicrm_commerce_membership_id)) &&
          ($order_line_item_wrapper->civicrm_commerce_membership_id->value() <> $membership['id'])) {
        $ignore_membership_ids[] = $order_line_item_wrapper->civicrm_commerce_membership_id->value();
      }
    }
  }
  
  // get the family member ids
  $result = civicrm_api('relationship', 'get', array('contact_id' => $hid, 'version' => 3));
  $relationships = $result['values'];

  // setup default search criteria
  $params = array(
    'membership_type_id' => OSA_MEMBER_STUDENT,
    'filters' => array( 
        'is_current' => 1,
      ),
    'version' => 3,
  );

  // only want memberships for the same year as the given contact
  if (isset($membership)) {
    $params['end_date'] = $membership['end_date'];
  }

  foreach ($relationships as $relationship) {
    $params['contact_id'] = $relationship['cid'];
    $result = civicrm_api('membership', 'get', $params);

    if (($result['is_error'] == 0) &&
        ($result['count'] == 1) &&
        (!in_array($result['id'],$ignore_membership_ids))) {
      $number_members++;
    }
  }

  // Make a quantity comparison based on the operator.
  switch ($operator) {
    case '<':
      return $number_members < $value;
    case '<=':
      return $number_members <= $value;
    case '=':
      return $number_members == $value;
    case '>=':
      return $number_members >= $value;
    case '>':
      return $number_members > $value;
  }

  return FALSE;
}
