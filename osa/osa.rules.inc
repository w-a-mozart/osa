<?php

/**
 * @file
 * Rules integration for the OSA
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
  static $number_members = array();
  
  $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $commerce_line_item);
  $line_item_id = $line_item_wrapper->line_item_id->value();
  
  // this can be called multiple times for a single line item
  if (!isset($number_members[$line_item_id])) {
    // must get a valid contact id in the line item
    if (!isset($line_item_wrapper->civicrm_commerce_contact_id)) {
      throw new RulesEvaluationException('The given commerce_line_item is incompatible with this condition.', array(), NULL, RulesLog::ERROR);
      return FALSE;
    }
    $contact_id = $line_item_wrapper->civicrm_commerce_contact_id->value();

    // need civicrm
    civicrm_initialize();
    
    // get all the family members for this contact, use static cacheing as this is called many times
    $hid = _osa_getHousehold($contact_id);
    $hid = isset($hid) ? $hid : $contact_id;
    static $osa_members = array();
    if (isset($osa_members[$hid])) {
      $members = $osa_members[$hid];
    }
    else {
      $members = array();
      $family = _osa_getHouseholdMembers($contact_id);
    
      // get the membership record for each family member
      foreach ($family as $person) {
        $membership = _osa_get_membership($person['contact_id']);
        if ((isset($membership)) && ($membership['is_current_member'])) {
          $members[$membership['id']] = $membership;
        }
      }
      $osa_members[$hid] = $members;
    }

    // eliminate memberships, depending on context of call
    
    // we don't want to count memberships that come after this one in the shopping cart
    $order = commerce_order_load($commerce_line_item->order_id);
    entity_get_controller('commerce_order')->resetCache(array($order->order_id)); // open the order record as read only: Issue 1514618
    if ($order) {
      $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
      foreach ($order_wrapper->commerce_line_items as $delta => $order_line_item_wrapper) {
        if (($order_line_item_wrapper->line_item_id->value() > $line_item_id) && (isset($order_line_item_wrapper->civicrm_commerce_membership_id))) {
          if (isset($members[$order_line_item_wrapper->civicrm_commerce_membership_id->value()])) {
            unset($members[$order_line_item_wrapper->civicrm_commerce_membership_id->value()]);
          }
        }
      }
    }

    // if we are passed a line_item for a new membership, we only want to include memberships for the same membership year
    if (isset($line_item_wrapper->civicrm_commerce_membership_id)) {
      $new_membership = isset($members[$line_item_wrapper->civicrm_commerce_membership_id->value()]) ? $members[$line_item_wrapper->civicrm_commerce_membership_id->value()] : array();
      
      if (!isset($new_membership['end_date'])) {
        $new_membership += _osa_get_membership_dates($new_membership['membership_type_id']);
      }
      
      foreach ($members as $mid => $member) {
        if ((isset($member['end_date'])) && ($member['end_date'] <> $new_membership['end_date'])) {
          unset($members[$mid]);
        }
      }
    }

    $number_members[$line_item_id] = count($members);
  }

  // Make a quantity comparison based on the operator.
  switch ($operator) {
    case '<':
      return $number_members[$line_item_id] < $value;
    case '<=':
      return $number_members[$line_item_id] <= $value;
    case '=':
      return $number_members[$line_item_id] == $value;
    case '>=':
      return $number_members[$line_item_id] >= $value;
    case '>':
      return $number_members[$line_item_id] > $value;
  }

  return FALSE;
}

/**
 * Implements hook_rules_action_info()
 */
function osa_rules_action_info() {
  $actions = array();

  // action to update contributions in CiviCRM when a payment is made
  $actions['osa_email_tickets'] = array(
    'label' => t('Email tickets'),
    'parameter' => array(
      'commerce_order' => array(
        'type' => 'commerce_order',
        'label' => t('Order'),
        'description' => t('The order whose line items contain at least one <em>Concert Ticket</em> contribution type.'),
      ),
    ),
    'group' => t('OSA'),
    'module' => 'osa',
  );

  return $actions;
}

/**
 * Rules action: Email Tickets
 */
function osa_email_tickets($commerce_order) {
  // this will invoke osa_mail_ticket() to generate the message
  drupal_mail('osa', 'ticket', $commerce_order->mail, language_default(), array('commerce_order' => $commerce_order));
}
