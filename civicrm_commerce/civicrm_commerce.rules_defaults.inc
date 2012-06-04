<?php

/**
 * @file
 * Rules for CiviCRM and Drupal Commerce integration.
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
 * Define default rules definitions.
 */
function _civicrm_commerce_default_rule_definitions() {

  $rule_defs = array();
  
  // define a rule to set the unit price to the civi contribution amount field 
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

  // define a rule to set the civi contribution status when a payment is made 
  $rule_defs[] = '{ "civicrm_commerce_contribution_status" : {
    "LABEL" : "CiviCRM : Update contribution status",
    "PLUGIN" : "reaction rule",
    "REQUIRES" : [ "commerce_order", "civicrm_commerce", "entity" ],
    "ON" : [ "commerce_payment_transaction_insert" ],
    "IF" : [
      { "commerce_order_contains_product" : {
          "commerce_order" : [ "commerce-payment-transaction:order" ],
          "product_id" : "CiviCRM",
          "operator" : "\u003E=",
          "value" : "1"
        }
      }
    ],
    "DO" : [
      { "civicrm_commerce_update_contribution" : { "commerce_payment_transaction" : [ "commerce_payment_transaction" ] } }
    ]
  }
}';

  // define a rule to remove the civi contribution status when removed from cart 
  $rule_defs[] = '{ "civicrm_commerce_remove_item" : {
    "LABEL" : "CiviCRM : Remove line item contribution",
    "PLUGIN" : "reaction rule",
    "REQUIRES" : [ "rules", "civicrm_commerce", "commerce_cart" ],
    "ON" : [ "commerce_cart_product_remove" ],
    "IF" : [
      { "entity_has_field" : {
          "entity" : [ "commerce-line-item" ],
          "field" : "civicrm_commerce_contribution_id"
        }
      }
    ],
    "DO" : [
      { "civicrm_commerce_remove_contribution" : { "commerce_line_item" : [ "commerce_line_item" ] } }
    ]
  }
}';

  return $rule_defs;
}
