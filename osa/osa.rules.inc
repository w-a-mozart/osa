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
  $conditions['osa_household_participants'] = array(
    'label' => t('Contact\'s household has other participants'),
    'parameter' => array(
      'commerce_line_item' => array(
        'type' => 'commerce_line_item',
        'label' => t('Line item'),
        'description' => t('The line item that contains a civicrm_commerce_participant_id whose household to check for other participants of the same event.'),
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
        'description' => t('The value to compare against the number of household participants.'),
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
 * Rules condition: participant's household has other participants
 */
function osa_household_participants($commerce_line_item, $operator, $value) {
  // this can be called multiple times for a single line item
  static $number_participants = array();
  
  $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $commerce_line_item);
  $line_item_id = $line_item_wrapper->line_item_id->value();
  
  if (!isset($number_participants[$line_item_id])) {
    // must get a valid participant id in the line item
    if (!isset($line_item_wrapper->civicrm_commerce_participant_id)) {
      throw new RulesEvaluationException('The given commerce_line_item is incompatible with this condition.', array(), NULL, RulesLog::ERROR);
      return FALSE;
    }
    $participant_id = $line_item_wrapper->civicrm_commerce_participant_id->value();

    // should get a valid contact id in the line item
    if (isset($line_item_wrapper->civicrm_commerce_contact_id)) {
      $contact_id = $line_item_wrapper->civicrm_commerce_contact_id->value();
    }
    // should get a valid event id in the line item
    if (isset($line_item_wrapper->civicrm_commerce_event_id)) {
      $event_id = $line_item_wrapper->civicrm_commerce_event_id->value();
    }
    
    if (!isset($contact_id) || !isset($event_id)) {
      civicrm_initialize();
      $result = civicrm_api('participant', 'getsingle', array('id' => $participant_id, 'version' => 3));
      if (isset($result['event_id'])) {$event_id = $result['event_id'];}
      if (isset($result['contact_id'])) {$contact_id = $result['contact_id'];}
    }

    if (!isset($contact_id) || !isset($event_id)) {
      throw new RulesEvaluationException('The given commerce_line_item is incompatible with this condition.', array(), NULL, RulesLog::ERROR);
      return FALSE;
    }

    // get all the family members participating in this event
    civicrm_initialize();
    $hid = _osa_getHousehold($contact_id);
    $household_participants = _osa_get_household_participants($hid, $event_id);

    // eliminate duplicate participants
    $contact_ids = array();
    foreach ($household_participants as $participant) {
      $contact_ids[] = $participant['contact_id'];
    }
    $number_participants[$line_item_id] = count(array_unique($contact_ids));
  }

  // Make a quantity comparison based on the operator.
  switch ($operator) {
    case '<':
      return $number_participants[$line_item_id] < $value;
    case '<=':
      return $number_participants[$line_item_id] <= $value;
    case '=':
      return $number_participants[$line_item_id] == $value;
    case '>=':
      return $number_participants[$line_item_id] >= $value;
    case '>':
      return $number_participants[$line_item_id] > $value;
  }

  return FALSE;
}

/**
 * Implements hook_rules_action_info()
 */
function osa_rules_action_info() {
  $actions = array();

  // action to email concert tickets
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
  $actions['osa_discount_multiple_family_participants'] = array(
    'label' => t('Apply discount for multiple family participants'),
    'parameter' => array(
      'commerce_line_item' => array(
        'type' => 'commerce_line_item',
        'label' => t('Line item'),
        'description' => t('The line item that contains a civicrm_commerce_participant_id whose household to may have other participants of the same event.'),
      ),
      'min_participants' => array(
        'type' => 'integer',
        'label' => t('Participants'),
        'default value' => '3',
        'description' => t('The number of household participants, at which the discount applies.'),
      ),
      'amount' => array(
        'type' => 'decimal',
        'label' => t('Amount'),
        'description' => t('Specify an amount by which to multiply the unit price. (E.g. 0.5)'),
        'default value' => 0.5,
      ),
      'component_name' => array(
        'type' => 'text',
        'label' => t('Price component type'),
        'description' => t('Price components track changes to prices made during the price calculation process, and they are carried over from the unit price to the total price of a line item. When an order total is calculated, it combines all the components of every line item on the order. When the unit price is altered by this action, the selected type of price component will be added to its data array and reflected in the order total display when it is formatted with components showing. Defaults to base price, which displays as the order Subtotal.'),
        'options list' => 'commerce_price_component_titles',
        'default value' => 'base_price',
      ),
      'round_mode' => array(
        'type' => 'integer',
        'label' => t('Price rounding mode'),
        'description' => t('Round the resulting price amount after performing this operation.'),
        'options list' => 'commerce_round_mode_options_list',
        'default value' => COMMERCE_ROUND_HALF_UP,
      ),
    ),
    'group' => t('OSA'),
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

/**
 * Rules action: Apply multiple child participants discount
 */
function osa_discount_multiple_family_participants($commerce_line_item, $min_participants, $amount, $component_name, $round_mode) {
  $commerce_line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $commerce_line_item);
  $commerce_line_item_id = $commerce_line_item_wrapper->line_item_id->value();
  $commerce_order_id = $commerce_line_item_wrapper->order_id->value();

  if (!isset($commerce_line_item_wrapper->civicrm_commerce_participant_id)) {
    throw new RulesEvaluationException('The given commerce_line_item is incompatible with this condition.', array(), NULL, RulesLog::ERROR);
    return;
  }
  $participant_id = $commerce_line_item_wrapper->civicrm_commerce_participant_id->value();

  // get the contact and event ids, and family participants
  // this is duplicated from osa_household_participants() above
  if (isset($commerce_line_item_wrapper->civicrm_commerce_contact_id)) {
    $contact_id = $commerce_line_item_wrapper->civicrm_commerce_contact_id->value();
  }
  if (isset($commerce_line_item_wrapper->civicrm_commerce_event_id)) {
    $event_id = $commerce_line_item_wrapper->civicrm_commerce_event_id->value();
  }
  if (!isset($contact_id) || !isset($event_id)) {
    civicrm_initialize();
    $result = civicrm_api('participant', 'getsingle', array('id' => $participant_id, 'version' => 3));
    if (isset($result['event_id'])) {$event_id = $result['event_id'];}
    if (isset($result['contact_id'])) {$contact_id = $result['contact_id'];}
  }
  if (!isset($contact_id) || !isset($event_id)) {
    throw new RulesEvaluationException('The given commerce_line_item is incompatible with this condition.', array(), NULL, RulesLog::ERROR);
    return;
  }
  civicrm_initialize();
  $hid = _osa_getHousehold($contact_id);
  $household_participants = _osa_get_household_participants($hid, $event_id);
  foreach ($household_participants as $participant) {
    $contact_ids[] = $participant['contact_id'];
  }
  $num_participants = count(array_unique($contact_ids));
  $pids = array_keys($household_participants);

  // Here be dragons.
  // The discount applies to the lest expensive registration fees.
  // Unfortunately these may not be in this line item or even in this order.
  // so we need to figure out what discounts apply to what line items
  static $discounts = array();
  if (empty($discounts[$hid]) && ($num_participants >= $min_participants)) {
    // Get all of the line items for this event and family (should include given line item)
    $query = new EntityFieldQuery();
    $result = $query->entityCondition('entity_type', 'commerce_line_item')
                    ->entityCondition('bundle', 'civi_event')
                    ->fieldCondition('civicrm_commerce_participant_id', 'value', $pids, 'IN')
                    ->fieldOrderBy('commerce_unit_price', 'amount')
                    ->execute();
    
    $line_items_ids = array_keys($result['commerce_line_item']);
    $line_items = commerce_line_item_load_multiple($line_items_ids);
    $tmp_line_items = array();
    $orders = array();
    $contacts = array();
    
    // get the order id and potential discount
    foreach ($line_items as $id => $line_item) {
      $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
      $discounts[$hid][$id] = $line_item_wrapper->commerce_unit_price->amount->value() * $amount;
      $tmp_line_items[$id] = array(
        'id' => $id,
        'order_id' => $line_item->order_id,
        'contact_id' => $line_item_wrapper->civicrm_commerce_contact_id->value(),
        'amount' => $line_item_wrapper->commerce_unit_price->amount->value(),
      );
      $contacts[$tmp_line_items[$id]['contact_id']][$id] = $tmp_line_items[$id];
      $orders[$tmp_line_items[$id]['order_id']][$id] = $tmp_line_items[$id];
    }

    // simplest case: single family order, one registration per person
    if ((count($orders) == 1) && ($num_participants == count($line_items_ids))) {
      // remove discounts for most expensive items
      $discounts[$hid] = array_slice($discounts[$hid], 0, count($line_items_ids) - $min_participants + 1, TRUE);
    }
    // rats!
    else {
      // merge line items for the same person & order number
      if ($num_participants != count($line_items_ids)) {
        foreach ($contacts as $contact_id => $c_line_items) {
          if (count($c_line_items) > 1) {
            $contact_orders = array();
            foreach ($c_line_items as $id => $c_line_item) {
              $c_order_id = $c_line_item['order_id'];
              if (isset($contact_orders[$c_order_id])) {
                $merged_id = $contact_orders[$c_order_id];
                $discounts[$hid][$merged_id]['amount'] += $discounts[$hid][$id]['amount'];
                unset($discounts[$hid][$id]);
                $tmp_line_items[$merged_id]['amount'] += $c_line_item['amount'];
                unset($tmp_line_items[$id]);
                $orders[$c_order_id][$merged_id] = $tmp_line_items[$merged_id];
                unset($orders[$c_order_id][$id]);
              }
              else {
                $contact_orders[$c_order_id] = $id;
              }
            }
          }
        }
        // resort the discounts
        asort($discounts[$hid]);
      } // end of merge multiple registrations for one person

      // remove discounts for most expensive items
      $discounts[$hid] = array_slice($discounts[$hid], 0, count($discounts[$hid]) - $min_participants + 1, TRUE);

      // check if we have discounts on old orders
      if (count($orders) > 1) {
        foreach ($discounts[$hid] as $id => $discount_amount) {
          if ($tmp_line_items[$id]['order_id'] != $commerce_order_id) {
            drupal_set_message(t('You may be eligible for a Multiple Child Discount on previous a previous order. Please contact the OSA Registrar if you did not receive a prior discount.'), 'warning', FALSE);
            break;
          }
        }
      }
    } // end PITA code
  } // end check static cache

  // apply the appropriate discount to this line item
  if (isset($discounts[$hid][$commerce_line_item_id]) && $discounts[$hid][$commerce_line_item_id] > 1) {
    // as this can get called multiple times, don't add the discount more than once
    $found = FALSE;
    $components = $commerce_line_item->commerce_unit_price['und'][0]['data']['components'];
    foreach ($components as $component) {
      if ($component['name'] == $component_name) {
        $found = TRUE;
        break;
      }
    }
    if (!$found) {
      // need to recalculate the actual discount amount
      // as we don't know what other discounts have been applied and saved
      commerce_line_item_unit_price_multiply($commerce_line_item, $amount, $component_name, $round_mode);
    }
    $discounts[$hid][$commerce_line_item_id] = 0;
  }
}
