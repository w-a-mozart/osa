<?php

/**
 * @file
 * Helper functions to integrate CiviCRM and Drupal Commerce.
 */

/**
 * Define constants for CiviCRM and Drupal Commerce integration.
 */
define('CIVICRM_COMMERCE_PRODUCT_SKU', 'CiviCRM');  // need a product entity

/**
 * Map custom field names and types
 *
 * @param string $entity
 *   The name of the entity / contact type.
 * @return integer
 *   Default profile id.
 */
function _civicrm_commerce_fields() {
    static $_fields = null;
    
    if ( ! $_fields ) {
        $_fields = array(
            'contribution_id' => array(
                'name' => 'civicrm_commerce_contribution_id',
                'type' => 'number_integer',
                'label' => t('Contribution'),
                'widget' => 'number',
            ),
            'membership_id' => array(
                'name' => 'civicrm_commerce_membership_id',
                'type' => 'number_integer',
                'label' => t('Membership'),
                'widget' => 'number',
            ),
            'mem_type_id' => array(
                'name' => 'civicrm_commerce_mem_type_id',
                'type' => 'number_integer',
                'label' => t('Membership Type'),
                'widget' => 'number',
            ),
            'contact_id' => array(
                'name' => 'civicrm_commerce_contact_id',
                'type' => 'number_integer',
                'label' => t('Contact'),
                'widget' => 'number',
            ),
            'event_id' => array(
                'name' => 'civicrm_commerce_event_id',
                'type' => 'number_integer',
                'label' => t('Event'),
                'widget' => 'number',
            ),
            'participant_id' => array(
                'name' => 'civicrm_commerce_participant_id',
                'type' => 'number_integer',
                'label' => t('Participant'),
                'widget' => 'number',
            ),
            'amount' => array(
                'name' => 'civicrm_commerce_amount',
                'type' => 'number_decimal',
                'label' => t('Amount'),
                'widget' => 'number',
            ),
            'currency_code' => array(
                'name' => 'civicrm_commerce_currency_code',
                'type' => 'text',
                'label' => t('Currency'),
                'widget' => 'text_textfield',
            ),
        );
    }
    return $_fields;
}

/**
 * Wrapper of Field API to create fields for required entities
 *
 * @param string $entity
 *   The name of the entity / contact type.
 * @return integer
 *   Default profile id.
 */
function _civicrm_commerce_create_fields( $field_names, $entity_type, $bundle ) {

    $civicrm_commerce_fields = _civicrm_commerce_fields();

    foreach ( $field_names as $field_name ) {
        $full_field_name = $civicrm_commerce_fields[$field_name]['name'];
        $field = field_info_field( $full_field_name );
        $instance = field_info_instance( $entity_type, $full_field_name, $bundle );

        if ( empty($field) ) {
            $field = array(
                'field_name' => $full_field_name,
                'type' => $civicrm_commerce_fields[$field_name]['type'],
                'cardinality' => 1,
                'entity_types' => array($entity_type),
                'translatable' => FALSE,
                'locked' => TRUE,
            );
            $field = field_create_field($field);
        }

        if ( empty($instance) ) {
            $instance = array(
                'field_name' => $full_field_name,
                'entity_type' => $entity_type,
                'bundle' => $bundle,

                'label' => $civicrm_commerce_fields[$field_name]['label'],
                'required' => TRUE,
                'settings' => array(),

                'widget' => array(
                    'type' => $civicrm_commerce_fields[$field_name]['widget'],
                    'weight' => 0,
                ),

                'display' => array(
                    'display' => array(
                    'label' => 'hidden',
                    'weight' => 0,
                    ),
                ),
            );
            field_create_instance($instance);
        }
    }
}

/**
 * Wrapper of CiviCRM API to create relationships
 *
 * @param integer $relationship_type_id
 * @param integer $contact_id_a
 * @param integer $contact_id_b
 */
function _civicrm_commerce_create_relationship( $relationship_type_id, $contact_id_a, $contact_id_b ) {
}

/**
 * Wrapper of CiviCRM API to get membership
 *
 * @param integer $contact_id
 */
function _civicrm_commerce_get_membership( $contact_id ) {
}

/**
 *
 */
function _civicrm_commerce_appendTemplateVar( &$smarty, $varname, &$apiResult ) {
}
