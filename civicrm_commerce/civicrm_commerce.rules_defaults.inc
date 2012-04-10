<?php

/**
 * @file
 * Rules for CiviCRM and Drupal Commerce integration.
 *
 */

 /**
 * Implements hook_default_rules_configuration()
 */
function civicrm_commerce_default_rules_configuration() {

  $default_rules = array();
  $rule_defs = _civicrm_commerce_default_rule_definitions();

  foreach ( $rule_defs as $rule_def ) {
    $rule = rules_import($rule_def);
    $default_rules[$rule->name] = $rule;
  }
  
  return $default_rules;
}

/**
 * Cheat here and use rules definitions we added manually and then exported
 */
function _civicrm_commerce_default_rule_definitions() {

  $rule_defs = array();
  $rule_defs[] = '{ "civicrm_commerce_unit_price" : {
    "LABEL" : "CiviCRM : Set unit price to contribution amount",
    "PLUGIN" : "reaction rule",
    "REQUIRES" : [ "rules", "commerce_line_item", "commerce_product_reference" ],
    "ON" : [ "commerce_product_calculate_sell_price" ],
    "IF" : [
      { "entity_has_field" : {
          "entity" : [ "commerce-line-item" ],
          "field" : "civicrm_commerce_amount"
        }
      },
      { "entity_has_field" : {
          "entity" : [ "commerce-line-item" ],
          "field" : "civicrm_commerce_currency_code"
        }
      }
    ],
    "DO" : [
      { "data_calc" : {
          "USING" : {
            "input_1" : [ "commerce-line-item:civicrm-commerce-amount" ],
            "op" : "*",
            "input_2" : "100"
          },
          "PROVIDE" : { "result" : { "contribution_amount" : "Contribution Amount" } }
        }
      },
      { "commerce_line_item_unit_price_amount" : {
          "commerce_line_item" : [ "commerce-line-item" ],
          "amount" : [ "contribution-amount" ],
          "component_name" : "base_price",
          "round_mode" : "1"
        }
      },
      { "commerce_line_item_unit_price_currency_code" : {
          "commerce_line_item" : [ "commerce_line_item" ],
          "currency_code" : [ "commerce-line-item:civicrm-commerce-currency-code" ]
        }
      }
    ]
  }
}';

  return $rule_defs;
}