<?php

/**
 * @file
 * Rules integration for CiviCRM and Commerce Cart.
 */
 
/**
 * Implements hook_rules_condition_info()
 */
function civicrm_commerce_rules_condition_info() {
  $conditions = array();

  $conditions['civicrm_commerce_is_member'] = array(
    'label' => t('Contact is a current member.'),
    'parameter' => array(
      'commerce_line_item' => array(
        'type' => 'commerce_line_item',
        'label' => t('Line item'),
        'description' => t('The line item that contains a civicrm_commerce_contact_id to check for membership.'),
      ),
    ),
    'group' => t('CiviCRM Commerce'),
    'module' => 'civicrm_commerce',
  );
  
  $conditions['civicrm_commerce_contains_financial_type'] = array(
    'label' => t('Order contains a particular financial type'),
    'parameter' => array(
      'commerce_order' => array(
        'type' => 'commerce_order',
        'label' => t('Order'),
        'description' => t('The order whose line items should be checked for the specified financial type. If the specified order does not exist, the comparison will act as if it is against a quantity of 0.'),
      ),
      'financial_type' => array(
        'type' => 'text',
        'label' => t('Financial Type'),
        'description' => t('The type of contribution to look for on the order.'),
      ),
      'operator' => array(
        'type' => 'text',
        'label' => t('Operator'),
        'description' => t('The operator used with the quantity value below to compare the quantity of the specified financial type on the order.'),
        'default value' => '>=',
        'options list' => 'commerce_numeric_comparison_operator_options_list',
        'restriction' => 'input',
      ),
      'value' => array(
        'type' => 'text',
        'label' => t('Quantity'),
        'default value' => '1',
        'description' => t('The value to compare against the quantity of the specified financial type on the order.'),
      ),
    ),
    'group' => t('CiviCRM Commerce'),
    'module' => 'civicrm_commerce',
  );

  return $conditions;
}

/**
 * Rules condition: contact is a current member.
 */
function civicrm_commerce_is_member($commerce_line_item) {
  $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $commerce_line_item);
  if (!isset($line_item_wrapper->civicrm_commerce_contact_id)) {
    return FALSE;
  }
  
  $contact_id = $line_item_wrapper->civicrm_commerce_contact_id->value();
  $params = array( 
    'contact_id' => $contact_id,
    'filters' => array( 
        'is_current' => 1,
      ),
    'version' => 3,
  );

  civicrm_initialize();
  $result = civicrm_api('membership', 'get', $params);

  if ($result['is_error'] == 0) {
    return ($result['count'] > 0);
  }
  else {
    return FALSE;
  }
}

/**
 * Rules condition: a particular financial type exists on an order in the specified quantity.
 */
function civicrm_commerce_contains_financial_type($order, $financial_type, $operator, $value) {
  $financial_type_qty = 0;

  // If we actually received a valid order...
  if (!empty($order)) {
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);

    civicrm_initialize();
    // Count the quantities of each line item with the specified financial type
    foreach ($order_wrapper->commerce_line_items as $delta => $line_item_wrapper) {
      if (!isset($line_item_wrapper->civicrm_commerce_contribution_id)) {
        continue;
      }

      $contribution_id = $line_item_wrapper->civicrm_commerce_contribution_id->value();
      $result = civicrm_api('contribution', 'get', array('id' => $contribution_id, 'version' => 3));
      if (isset($result['values'][$contribution_id]['financial_type']) && ($result['values'][$contribution_id]['financial_type'] == $financial_type)) {
        $financial_type_qty += $line_item_wrapper->quantity->value();
      }
    }
  }

  // Make a quantity comparison based on the operator.
  switch ($operator) {
    case '<':
      return $financial_type_qty < $value;
    case '<=':
      return $financial_type_qty <= $value;
    case '=':
      return $financial_type_qty == $value;
    case '>=':
      return $financial_type_qty >= $value;
    case '>':
      return $financial_type_qty > $value;
  }

  return FALSE;
}

/**
 * Implements hook_rules_action_info()
 */
function civicrm_commerce_rules_action_info() {
  $actions = array();

  // action to update contributions in CiviCRM when a payment is made
  $actions['civicrm_commerce_update_contribution'] = array(
    'label' => t('Update CiviCRM contribution'),
    'parameter' => array(
      'commerce_payment_transaction' => array(
        'type' => 'commerce_payment_transaction',
        'label' => t('Payment transaction'),
      ),
    ),
    'group' => t('CiviCRM Commerce'),
    'module' => 'civicrm_commerce',
  );

  // action to remove contributions from CiviCRM associated with the Line Item
  $actions['civicrm_commerce_remove_contribution'] = array(
    'label' => t('Remove CiviCRM contribution'),
    'parameter' => array(
      'commerce_line_item' => array(
        'type' => 'commerce_line_item',
        'label' => t('Line item'),
      ),
    ),
    'group' => t('CiviCRM Commerce'),
    'module' => 'civicrm_commerce',
  );

  return $actions;
}

/**
 * Rules action: update CiviCRM Contribution status
 */
function civicrm_commerce_update_contribution($commerce_payment_transaction) {
  civicrm_initialize();

  // open the order record as read only: Issue 1514618
  $order = commerce_order_load($commerce_payment_transaction->order_id);
  entity_get_controller('commerce_order')->resetCache(array($order->order_id));
  $civi_line_item_types = array_keys(civicrm_commerce_commerce_line_item_type_info());
  $params = array(
    'receive_date' => format_date($commerce_payment_transaction->created, 'custom', 'Y-m-d H:i:s'),
    'version' => 3,
  );
  $participant_params = array('version' => 3);
  
  // CiviCRM doesn't allow partial payments, so for now set status to last payment
  if ($commerce_payment_transaction->status == COMMERCE_PAYMENT_STATUS_SUCCESS) {
    $params['contribution_status_id'] = 1;
    $participant_params['status_id'] = 1;
  }
  elseif ($commerce_payment_transaction->status == COMMERCE_PAYMENT_STATUS_FAILURE) {
    $params['contribution_status_id'] = 4;
    $participant_params['status_id'] = 4;
  }
  else {
    // assume a recorded payment that is pending, must be a cheque
    $params['is_pay_later'] = TRUE;
    $params['contribution_status_id'] = 2;
    $params['payment_instrument_id'] = 4;
    $participant_params['status_id'] = 5;
  }

  if (!empty($order)) {
    $order_wrapper = entity_metadata_wrapper('commerce_order', $order);
    // get the contact id of the order owner (see below)
    $uf_match = civicrm_api('UFMatch', 'getsingle', array('uf_id' => $order->uid, 'version' => 3));
    
    foreach ($order_wrapper->commerce_line_items as $delta => $line_item_wrapper) {
      if (in_array($line_item_wrapper->type->value(), $civi_line_item_types)) {

        if ($line_item_wrapper->civicrm_commerce_contribution_id->value()) {
          $price = $line_item_wrapper->commerce_total->value();
          $params['id'] = $line_item_wrapper->civicrm_commerce_contribution_id->value();
          $params['total_amount'] = commerce_currency_amount_to_decimal($price['amount'], $price['currency_code']);
          $params['trxn_id'] = $commerce_payment_transaction->remote_id . '-' . $line_item_wrapper->line_item_id->value();

          // getting error "Not enough data to create activity object," when the shopping cart was originally Anonymous (I think)
          // so set the source_contact_id to the owner of the order : or the line item contact : or Admin
          if (isset($uf_match['contact_id'])) {
            $params['source_contact_id'] = $uf_match['contact_id'];
          }
          elseif (isset($line_item_wrapper->civicrm_commerce_contact_id) && ($line_item_wrapper->civicrm_commerce_contact_id->value() > 0)) {
            $params['source_contact_id'] = $line_item_wrapper->civicrm_commerce_contact_id->value();
          }
          else {
            $params['source_contact_id'] = 1;
          }

          // @TODO - don't know why this fails if financial_type_id is not specified
          $contribution_id = $line_item_wrapper->civicrm_commerce_contribution_id->value();
          $contribution = civicrm_api('contribution', 'getsingle', array('id' => $contribution_id, 'version' => 3));
          $params['financial_type_id'] = $contribution['financial_type_id'];

          $result = civicrm_api('contribution', 'create', $params);
          if ($result['is_error'] != 0) {
            watchdog('civicrm_commerce', 'Error updating civicrm contribution %1: %2 \r\n %3', array( '%1' => $params['id'], '%2' => $result['error_message'], '%3' => print_r($params, TRUE),), WATCHDOG_ERROR);
          }
        }

        // update the membership records
        if (($commerce_payment_transaction->status == COMMERCE_PAYMENT_STATUS_SUCCESS) && ($line_item_wrapper->type->value() == 'civi_membership')) {
          $membership_id = $line_item_wrapper->civicrm_commerce_membership_id->value();
          $result = civicrm_api('membership', 'get', array('id' => $membership_id, 'version' => 3));
          if ($result['is_error'] == 0) {
            $membership_params = $result['values'][$membership_id];
            unset($membership_params['status_id']);
            $membership_params['skipStatusCal'] = FALSE;
            $membership_params['version'] = 3;

            $dates = CRM_Member_BAO_MembershipType::getDatesForMembershipType($membership_params['membership_type_id']);

            $membership_params['end_date'] = $dates['end_date'];
            $membership_params['start_date'] = $dates['start_date'];
            $membership_params['join_date'] = isset($membership_params['join_date']) ? $membership_params['join_date'] : $dates['join_date'];

            $result = civicrm_api('membership', 'create', $membership_params);
            if ($result['is_error'] != 0) {
              watchdog('civicrm_commerce', 'Error updating civicrm membership %1: %2', array( '%1' => $membership_params['id'], '%2' => $result['error_message']), WATCHDOG_ERROR);
            }
          }
        }
        
        // update the participant records
        if ($line_item_wrapper->type->value() == 'civi_event') {
          $participant_params['id'] = $line_item_wrapper->civicrm_commerce_participant_id->value();
          $result = civicrm_api('participant', 'create', $participant_params);
          if ($result['is_error'] != 0) {
            watchdog('civicrm_commerce', 'Error updating civicrm participant %1: %2', array( '%1' => $participant_params['id'], '%2' => $result['error_message']), WATCHDOG_ERROR);
          }
        }
      }
    }
  }
}

/**
 * Rules action: remove CiviCRM Contribution.
 */
function civicrm_commerce_remove_contribution($commerce_line_item) {

  $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $commerce_line_item);
  if (!isset($line_item_wrapper->civicrm_commerce_contribution_id)) {
    return;
  }
  
  $contribution_id = $line_item_wrapper->civicrm_commerce_contribution_id->value();
  civicrm_initialize();
  $result = civicrm_api('contribution', 'delete', array('id' => $contribution_id, 'version' => 3));

  // membership status get messed up when you delete a pending contribution
  if (($commerce_line_item->type == 'civi_membership') && (isset($line_item_wrapper->civicrm_commerce_membership_id))) {
    $membership_id = $line_item_wrapper->civicrm_commerce_membership_id->value();
    
    // 1 or 2 entries have been added to the membership log
    // one when it was added to the cart and one when the status and end_date were updated if this was a renewal
    $last_id_query = "SELECT MAX(id) FROM civicrm_membership_log WHERE membership_id = {$membership_id}";
    $last_id = CRM_Core_DAO::singleValueQuery($last_id_query);
    if ($last_id) {
      // delete the renewal update entry (if it's a new membership we don't care because we will just delete everything anyway)
      CRM_Core_DAO::executeQuery("DELETE FROM civicrm_membership_log WHERE id = {$last_id}");
      
      // get the old end_date and status from the 'added to the cart' entry
      $last_id = CRM_Core_DAO::singleValueQuery($last_id_query);
      if ($last_id) {
        $end_date = CRM_Core_DAO::singleValueQuery("SELECT end_date FROM civicrm_membership_log WHERE id = {$last_id}");
        $status_id = CRM_Core_DAO::singleValueQuery("SELECT status_id FROM civicrm_membership_log WHERE id = {$last_id}");
      }
    }
    
    // reset the status and end_date of the membership
    if (isset($end_date)) {
      $result = civicrm_api('membership', 'create', array('id' => $membership_id, 'status_id' => $status_id, 'end_date' => $end_date, 'version' => 3));
    }
    // or delete the membership if there is no end date
    else {
      $result = civicrm_api('membership', 'delete', array('id' => $membership_id, 'version' => 3));
    }

    // delete all of the unwanted log entries (the reset api call creates another entry)
    $last_id = isset($last_id) ? $last_id : 0;
    CRM_Core_DAO::executeQuery("DELETE FROM civicrm_membership_log WHERE membership_id = {$membership_id} AND id >= {$last_id}");
  }

  // remove the participant record
  if (($commerce_line_item->type == 'civi_event') && (isset($line_item_wrapper->civicrm_commerce_participant_id))) {
    $participant_id = $line_item_wrapper->civicrm_commerce_participant_id->value();
    $result = civicrm_api('participant', 'delete', array('id' => $participant_id, 'version' => 3));
  }
}
