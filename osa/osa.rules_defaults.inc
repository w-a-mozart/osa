<?php

/**
 * @file
 * Rules for the OSA.
 */

 /**
 * Implements hook_default_rules_configuration()
 */
function osa_default_rules_configuration() {

  $default_rules = array();
  $rule_defs = _osa_default_rule_definitions();

  foreach ( $rule_defs as $rule_def ) {
    $rule = rules_import($rule_def);
    $default_rules[$rule->name] = $rule;
  }
  
  return $default_rules;
}

/**
 * Define default rules definitions.
 */
function _osa_default_rule_definitions() {

  $rule_defs = array();
  
  // define discount rules 2nd, 3rd and 4th family members 
  $rule_defs[] = '{ "osa_2nd_membership_discount" : {
    "LABEL" : "OSA: 2nd Membership discount",
    "PLUGIN" : "reaction rule",
    "WEIGHT" : "5",
    "REQUIRES" : [ "rules", "osa", "commerce_line_item", "commerce_product_reference" ],
    "ON" : [ "commerce_product_calculate_sell_price" ],
    "IF" : [
      { "entity_has_field" : {
          "entity" : [ "commerce-line-item" ],
          "field" : "civicrm_commerce_membership_id"
        }
      },
      { "osa_household_members" : {
          "commerce_line_item" : [ "commerce_line_item" ],
          "operator" : "=",
          "value" : "2"
        }
      }
    ],
    "DO" : [
      { "commerce_line_item_unit_price_subtract" : {
          "commerce_line_item" : [ "commerce_line_item" ],
          "amount" : "2000",
          "component_name" : "discount",
          "round_mode" : "0"
        }
      }
    ]
  }
}';

  $rule_defs[] = '{ "osa_3rd_membership_discount" : {
    "LABEL" : "OSA: 3rd Membership discount",
    "PLUGIN" : "reaction rule",
    "WEIGHT" : "5",
    "REQUIRES" : [ "rules", "osa", "commerce_line_item", "commerce_product_reference" ],
    "ON" : [ "commerce_product_calculate_sell_price" ],
    "IF" : [
      { "entity_has_field" : {
          "entity" : [ "commerce-line-item" ],
          "field" : "civicrm_commerce_membership_id"
        }
      },
      { "osa_household_members" : {
          "commerce_line_item" : [ "commerce_line_item" ],
          "operator" : "=",
          "value" : "3"
        }
      }
    ],
    "DO" : [
      { "commerce_line_item_unit_price_subtract" : {
          "commerce_line_item" : [ "commerce_line_item" ],
          "amount" : "4000",
          "component_name" : "discount",
          "round_mode" : "0"
        }
      }
    ]
  }
}';

  $rule_defs[] = '{ "osa_4th_membership_discount" : {
    "LABEL" : "OSA: 4th Membership discount",
    "PLUGIN" : "reaction rule",
    "WEIGHT" : "5",
    "REQUIRES" : [ "rules", "osa", "commerce_line_item", "commerce_product_reference" ],
    "ON" : [ "commerce_product_calculate_sell_price" ],
    "IF" : [
      { "entity_has_field" : {
          "entity" : [ "commerce-line-item" ],
          "field" : "civicrm_commerce_membership_id"
        }
      },
      { "osa_household_members" : {
          "commerce_line_item" : [ "commerce_line_item" ],
          "operator" : "\u003E=",
          "value" : "4"
        }
      }
    ],
    "DO" : [
      { "commerce_line_item_unit_price_subtract" : {
          "commerce_line_item" : [ "commerce_line_item" ],
          "amount" : "6500",
          "component_name" : "discount",
          "round_mode" : "0"
        }
      }
    ]
  }
}';

  // define a rule for member discounts on events 
  $rule_defs[] = '{ "osa_member_discount_event" : {
    "LABEL" : "OSA: Member discount on events",
    "PLUGIN" : "reaction rule",
    "WEIGHT" : "5",
    "REQUIRES" : [
      "rules",
      "civicrm_commerce",
      "php",
      "commerce_line_item",
      "commerce_product_reference"
    ],
    "ON" : [ "commerce_product_calculate_sell_price" ],
    "IF" : [
      { "entity_has_field" : {
          "entity" : [ "commerce-line-item" ],
          "field" : "civicrm_commerce_event_id"
        }
      },
      { "civicrm_commerce_is_member" : { "commerce_line_item" : [ "commerce_line_item" ] } },
      { "php_eval" : { "code" : "$line_item_wrapper = entity_metadata_wrapper(\u0027commerce_line_item\u0027, $commerce_line_item);\r\n$event_id = $line_item_wrapper-\u003Ecivicrm_commerce_event_id-\u003Evalue();\r\ncivicrm_initialize();\r\n$result = civicrm_api(\u0027event\u0027, \u0027get\u0027, array(\u0027id\u0027 =\u003E $event_id, \u0027version\u0027 =\u003E 3));\r\nif (($result[\u0027is_error\u0027] == 0) \u0026\u0026 ($result[\u0027count\u0027] \u003E 0) \u0026\u0026 (isset($result[\u0027values\u0027][$event_id][\u0027custom_7_1\u0027]))) {\r\n  return ($result[\u0027event_id\u0027][$event_id][\u0027custom_7_1\u0027] != \u00270.00\u0027);\r\n}\r\nelse {\r\n  return FALSE;\r\n}" } }
    ],
    "DO" : [
      { "commerce_line_item_unit_price_subtract" : {
          "commerce_line_item" : [ "commerce_line_item" ],
          "amount" : {
            "select" : "commerce-line-item:civicrm-commerce-event-id",
            "php" : { "code" : "civicrm_initialize();\r\n$result = civicrm_api(\u0027event\u0027, \u0027get\u0027, array(\u0027id\u0027 =\u003E $value, \u0027version\u0027 =\u003E 3));\r\nif (($result[\u0027is_error\u0027] == 0) \u0026\u0026 ($result[\u0027count\u0027] \u003E 0) \u0026\u0026 (isset($result[\u0027values\u0027][$value][\u0027custom_7_1\u0027]))) {\r\n  return $result[\u0027values\u0027][$value][\u0027custom_7_1\u0027] * 100;\r\n}\r\nelse {\r\n  return 0;\r\n}" }
          },
          "component_name" : "discount",
          "round_mode" : "0"
        }
      }
    ]
  }
}';

  // define a rule to restrict access to photo galleries 
  $rule_defs[] = '{ "osa_restrict_anonymous" : {
    "LABEL" : "Restrict content to logged in users.",
    "PLUGIN" : "reaction rule",
    "REQUIRES" : [ "rules" ],
    "ON" : [ "node_view" ],
    "IF" : [
      { "user_has_role" : {
          "account" : [ "site:current-user" ],
          "roles" : { "value" : { "1" : "1" } }
        }
      },
      { "node_is_of_type" : {
          "node" : [ "node" ],
          "type" : { "value" : { "himuesgallery" : "himuesgallery" } }
        }
      }
    ],
    "DO" : [
      { "redirect" : { "url" : "user\/login?destination=node" } },
      { "drupal_message" : {
          "message" : "You must be logged in to view this content.",
          "type" : "warning",
          "repeat" : 0
        }
      }
    ]
  }
}';

  return $rule_defs;
}
