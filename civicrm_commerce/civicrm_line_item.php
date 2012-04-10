<?php
/**
 * @file 
 * line items for CiviCRM items
 * - modified from line_item_example.module
 */
require_once 'civicrm_commerce.inc';

/**
 * Implements hook_commerce_line_item_type_info().
 *
 * - add line item types for memberships, events, and other contributions
 * - also add support for member discounts on events (this might be better in a general discount module)
 *
 * @see hook_commerce_line_item_type_info().
 * @see http://www.drupalcommerce.org/specification/info-hooks/line-item
 *
 */
function civicrm_commerce_commerce_line_item_type_info() {

    $line_item_types['civi_membership'] = array(
        'name' => t('Membership'),
        'description' => t('A line item type, for CiviCRM Memberships'),
        // we use a generic "CiviCRM" product
        'product' => TRUE,
        'add_form_submit_value' => t('Add Membership'),
        'base' => 'civicrm_commerce_line_item',
    );
    
    $line_item_types['civi_event'] = array(
        'name' => t('Event'),
        'description' => t('A line item type, for CiviCRM Events'),
        'product' => TRUE,
        'add_form_submit_value' => t('Add Event'),
        'base' => 'civicrm_commerce_line_item',
    );
    
    $line_item_types['civi_contribution'] = array(
        'name' => t('Contribution'),
        'description' => t('A line item type, for CiviCRM Contributions'),
        'product' => TRUE,
        'add_form_submit_value' => t('Add Contribution'),
        'base' => 'civicrm_commerce_line_item',
    );

    $line_item_types['civi_member_discount'] = array(
        'name' => t('Member Discount'),
        'description' => t('A line item type, for CiviCRM Member Discounts'),
        'product' => FALSE,
        'add_form_submit_value' => t('Add Member Discount'),
        'base' => 'civicrm_commerce_line_item',
    );
    
    return $line_item_types;
}


/**
 * Configure each line item with required fields.
 *
 * This function is called by the line item module when it is enabled or this
 * module is enabled.
 *
 * @param $line_item_type
 *   The info array of the line item type being configured.
 *
 * @see commerce_product_line_item_configuration()
 */
function civicrm_commerce_line_item_configuration( $line_item_type ) {
    $type = $line_item_type['type'];

    // every line item except the discount needs a product reference & product prices
    if ( $type != 'civi_member_discount' ) {
        commerce_product_line_item_configuration( $line_item_type );
    }

    // add the type specific fields to the line item to link to the CiviCRM items
    switch ($type) {
    case "civi_membership":
        $field_names = array( 'contribution_id', 'amount', 'currency_code', 'contact_id', 'membership_id', 'mem_type_id' );
    break;
    case "civi_event":
        $field_names = array( 'contribution_id', 'amount', 'currency_code', 'contact_id', 'event_id', 'participant_id' );
    break;
    case "civi_contribution":
        $field_names = array( 'contribution_id', 'amount', 'currency_code', 'contact_id' );
    break;
    case "civi_member_discount":
        $field_names = array( 'amount', 'currency_code' );
    break;
    }
    
    if ( isset($field_names) ) {
        _civicrm_commerce_create_fields( $field_names, 'commerce_line_item', $type );
    }
}

/**
 * Returns a title for this line item.
 */
function civicrm_commerce_line_item_title( $line_item ) {
    $type = $line_item->type;

    switch ($type) {

    case "civi_membership":
        $title = t('Membership');
    break;
    case "civi_event":
        $title = t('Event');
    break;
    case "civi_contribution":
        $title = t('Contribution');
    break;
    case "civi_member_discount":
        $title = t('Member Discount');
    break;
    default:
        $title = t('Unknown');
    break;
    }

    return( $title );
}

/**
 * Returns the elements necessary to add a line item through a line item manager widget (on the order form).
 */
function civicrm_commerce_line_item_add_form( $element, &$form_state ) {

    $form = array();
    $type = $form_state['input']['commerce_line_items']['und']['actions']['line_item_type'];

    $instances = field_info_instances('commerce_line_item', $type );
    
    foreach( $instances as $instance => $info ) {
        if ( substr($instance, 0, 16) == 'civicrm_commerce' ) {
            $form[$instance] = array(
                '#type' => 'textfield',
                '#title' => $info['label'],
                '#description' => $info['description'],
                '#size' => 60,
                '#maxlength' => 255,
            );
        }
    }

    if ( array_key_exists( 'civicrm_commerce_currency_code', $form ) ) {
        $form['civicrm_commerce_currency_code']['#default_value'] = commerce_default_currency();
    }
    
    return $form;
}

/**
 * Adds the selected information to a line item added via a line item manager widget (on the admin order page).
 *
 * @param $line_item
 *   The newly created line item object.
 * @param $element
 *   The array representing the widget form element.
 * @param $form_state
 *   The present state of the form upon the latest submission.
 * @param $form
 *   The actual form array.
 *
 * @return
 *   NULL if all is well or an error message if something goes wrong.
 */
function civicrm_commerce_line_item_add_form_submit( &$line_item, $element, &$form_state, $form ) {

    $type = $line_item->type;
    $amount = $form_state['input']['commerce_line_items']['und']['actions']['civicrm_commerce_amount'];
    $currency_code = $form_state['input']['commerce_line_items']['und']['actions']['civicrm_commerce_currency_code'];
    $title = civicrm_commerce_line_item_title( $line_item );

    if ( $type == 'civi_member_discount' ) {
        $amount = $amount * -1;
    }
    
    // Set field information.
    $line_item->line_item_label = $title;
    $line_item_wrapper = entity_metadata_wrapper( 'commerce_line_item', $line_item );
    $line_item->commerce_unit_price = array( 'und' => array( '0' => array( 'amount' => $amount * 100, 'currency_code' => $currency_code ) ) );

    if ( !is_null( $line_item_wrapper->commerce_unit_price->value() ) ) {
        // Add the base price to the components array.
        if ( !commerce_price_component_load( $line_item_wrapper->commerce_unit_price->value(), 'base_price' ) ) {
            $line_item_wrapper->commerce_unit_price->data = commerce_price_component_add( $line_item_wrapper->commerce_unit_price->value(),
                                                                                          'base_price',
                                                                                          $line_item_wrapper->commerce_unit_price->value(),
                                                                                          TRUE
                                                                                        );
        }
    }
}

/**  
 * Call by the Payment Processor to add new line items to the Drupal Commerce Cart for later checkout
 *  
 * @param array $params  name value pair of contribution data
 *  
 * @return void  
 * @access public 
 *  
 */  
function civicrm_commerce_line_item_add_new( &$params, $component ) {

    // parse the params
    $contributionID   = CRM_Utils_Array::value( 'contributionID', $params );
    $amount           = CRM_Utils_Array::value( 'amount', $params );
    $currency_code    = CRM_Utils_Array::value( 'currencyID', $params );
    $contactID        = CRM_Utils_Array::value( 'contactID', $params );
    $membershipID     = CRM_Utils_Array::value( 'membershipID', $params );
    $eventID          = CRM_Utils_Array::value( 'eventID', $params );
    $participantID    = CRM_Utils_Array::value( 'participantID', $params );

    // load the "dummy" civicrm product
    $product = commerce_product_load_by_sku( CIVICRM_COMMERCE_PRODUCT_SKU );

    // determine the line item type to create
    if ( $component == "contribute" ) {
        if ( $membershipID ) {
            $type = 'civi_membership';
            $label = CRM_Utils_Array::value( 'contributionType_name', $params );
        } else {
            $type = 'civi_contribution';
            $label = CRM_Utils_Array::value( 'contributionType_name', $params );
        }
    } elseif ( $component == "event" ) {
        $type = 'civi_event';
        $label = CRM_Utils_Array::value( 'item_name', $params );
    } else {
        CRM_Core_Error::fatal( ts( 'Unknown shopping cart item.' ) );
    }
    
    // change it's price to ensure a base price is added to the line item
    // this will be overwritten by the pricing rule
    $product_wrapper = entity_metadata_wrapper( 'commerce_product', $product );
    $product_wrapper->title = $label;
    $product_wrapper->commerce_price->amount = $amount;
    $product_wrapper->commerce_price->currency_code = $currency_code;

    // create the line item and set its attributes
    $line_item = commerce_product_line_item_new( $product, 1, 0, $params, $type );
    $line_item_wrapper = entity_metadata_wrapper( 'commerce_line_item', $line_item );

    $line_item_wrapper->line_item_label = $label;
    $line_item_wrapper->civicrm_commerce_amount = $amount;
    $line_item_wrapper->civicrm_commerce_currency_code = $currency_code;

    if ( $contributionID ) {
        $line_item_wrapper->civicrm_commerce_contribution_id = $contributionID;
    }
    if ( $membershipID ) {
        $line_item_wrapper->civicrm_commerce_membership_id = $membershipID;
    }
    if ( $contactID ) {
        $line_item_wrapper->civicrm_commerce_contact_id = $contactID;
    }
    if ( $eventID ) {
        $line_item_wrapper->civicrm_commerce_event_id = $eventID;
    }
    if ( $participantID ) {
        $line_item_wrapper->civicrm_commerce_participant_id = $participantID;
    }

    // add it to the current user's cart
    $uid  = CRM_Utils_System::getLoggedInUfID();
    commerce_cart_product_add( $uid, $line_item, FALSE );
}

/**
 * Utility function which creates a new member_discount line item 
 *
 * @param $order_id
 *   The ID of the order the line item belongs to (if available).
 *
 * @return
 *   The fully loaded line item.
 */
function _civicrm_commerce_member_discount_new( $amount, $currency_code = null, $order_id = 0 ) {
    $type = 'civi_member_discount';

    // clean-up the input parameters
    $amount = $amount * -1;
    if ( !isset( $currency_code ) ) {
        $currency_code = commerce_default_currency();
    }

    // Create the line item entity
    $line_item = entity_create( 'commerce_line_item',
        array(
            'type' => $type,
            'order_id' => $order_id,
            'quantity' => $quantity,
            'data' => $data,
        ));

    // set the line item standard attributes
    $line_item_wrapper = entity_metadata_wrapper( 'commerce_line_item', $line_item );
    $line_item_wrapper->line_item_label = t('Member Discount');
    $line_item_wrapper->commerce_unit_price->amount = $amount;
    $line_item_wrapper->commerce_unit_price->currency_code = $currency_code;
    
    // set the civi specific attributes
    $line_item_wrapper->civicrm_commerce_amount = $amount;
    $line_item_wrapper->civicrm_commerce_currency_code = $currency_code;

    // create a base price ?don't know why we need this
    if ( !is_null( $line_item_wrapper->commerce_unit_price->value() ) ) {
        if ( !commerce_price_component_load( $line_item_wrapper->commerce_unit_price->value(), 'base_price' ) ) {
            $line_item_wrapper->commerce_unit_price->data = commerce_price_component_add( $line_item_wrapper->commerce_unit_price->value(),
                                                                                          'base_price',
                                                                                          $line_item_wrapper->commerce_unit_price->value(),
                                                                                          TRUE
                                                                                        );
        }
    }
    
    return $line_item;
}

// Add a member discount for each item when required.
function civicrm_commerce_commerce_order_presave( $order ) {

/*
    
    $type_exists = FALSE;
    if (!empty($order->commerce_line_items['und'])) {
        foreach($order->commerce_line_items['und'] as $delta => $line_item_entry) {
            if ($line_item = commerce_line_item_load($line_item_entry['line_item_id'])) {
                if ($line_item->type == 'civi_member_discount') {
                    $type_exists = TRUE;
                    break;
                }
            }
        }
    }

    // If our line item is not yet in the order and the order has an ID,
    // create a line item to add to the order.
    if (!$type_exists && $order->order_id > 0) {
        $line_item = _civicrm_commerce_member_discount_add_new( $order->order_id );
        commerce_line_item_save( $line_item );
        $order->commerce_line_items['und'][] = array( 'line_item_id' => $line_item->line_item_id );
    }
*/
}
