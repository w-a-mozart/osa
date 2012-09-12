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
  $order = commerce_order_load($commerce_payment_transaction->order_id);
  $civi_line_item_types = array_keys(civicrm_commerce_commerce_line_item_type_info());
  $key = array_search('civi_member_discount', $civi_line_item_types);
  unset($civi_line_item_types[$key]);
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
    
    civicrm_initialize();
    foreach ($order_wrapper->commerce_line_items as $delta => $line_item_wrapper) {
      if (in_array($line_item_wrapper->type->value(), $civi_line_item_types)) {

        if ($line_item_wrapper->civicrm_commerce_contribution_id->value()) {
          $price = $line_item_wrapper->commerce_total->value();
          $params['id'] = $line_item_wrapper->civicrm_commerce_contribution_id->value();
          $params['total_amount'] = commerce_currency_amount_to_decimal($price['amount'], $price['currency_code']);
          $params['trxn_id'] = $commerce_payment_transaction->remote_id . '-' . $line_item_wrapper->line_item_id->value();
          $result = civicrm_api('contribution', 'create', $params);
          if ($result['is_error'] != 0) {
            watchdog('civicrm_commerce', 'Error updating civicrm contribution %1: %2', array( '%1' => $params['id'], '%2' => $result['error_message']), WATCHDOG_ERROR);
          }
        }

        // updating the contribution status doesn't update the participant
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
  // @todo must be a better way
  if (($commerce_line_item->type == 'civi_membership') && (isset($line_item_wrapper->civicrm_commerce_membership_id))) {
    $membership_id = $line_item_wrapper->civicrm_commerce_membership_id->value();
    
    // delete the membership log entry created when item was added to the cart
    require_once 'CRM/Core/DAO.php';
    $last_id_query = 'SELECT MAX(id) FROM civicrm_membership_log WHERE membership_id = ' . $membership_id;
    $last_id = CRM_Core_DAO::singleValueQuery( $last_id_query );
    if ($last_id) {
      $del_query = 'DELETE FROM civicrm_membership_log WHERE id = ' . $last_id;
      CRM_Core_DAO::executeQuery($del_query);
      
      // get the end date from the previous entry
      $end_date_query = 'SELECT end_date FROM civicrm_membership_log WHERE id in (' . $last_id_query . ')';
      $end_date = CRM_Core_DAO::singleValueQuery($end_date_query);
    }
    // reset the end date of the membership
    if (isset($end_date)) {
      $result = civicrm_api('membership', 'create', array('id' => $membership_id, 'end_date' => $end_date, 'version' => 3));

      // don't want this log entry either
      $last_id = CRM_Core_DAO::singleValueQuery( $last_id_query );
      if ($last_id) {
        $del_query = 'DELETE FROM civicrm_membership_log WHERE id = ' . $last_id;
        CRM_Core_DAO::executeQuery($del_query);
      }
    }
    // or delete the membership if there is no end date
    else {
      $result = civicrm_api('membership', 'delete', array('id' => $membership_id, 'version' => 3));
    }
  }
  
  // remove the participant record
  if (($commerce_line_item->type == 'civi_event') && (isset($line_item_wrapper->civicrm_commerce_participant_id))) {
    $participant_id = $line_item_wrapper->civicrm_commerce_participant_id->value();
    $result = civicrm_api('participant', 'delete', array('id' => $participant_id, 'version' => 3));
  }
}
