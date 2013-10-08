<?php
/**
 * @file
 * Rules integration for the QuickBooks Web Connector SOAP Service
 */
require_once 'quickbooks_webconnect.inc';
 
/**
 * Implements hook_rules_condition_info()
 */
function quickbooks_webconnect_rules_condition_info() {
  $conditions = array();
  return $conditions;
}

/**
 * Implements hook_rules_action_info()
 */
function quickbooks_webconnect_rules_action_info() {
  $actions = array();
  return $actions;
}
