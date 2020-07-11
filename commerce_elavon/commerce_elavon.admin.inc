<?php

/**
 * @file
 * Administration form for the Commerce Elavon Payment gateway.
 */
require_once 'commerce_elavon.inc';

/**
 * Menu callback; Displays the administration settings for Elavon Payment.
 */
function commerce_elavon_admin_settings() {
  $form = array();

  $form['commerce_elavon'] = array(
    '#type' => 'fieldset',
    '#title' => t('Elavon Payment settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['commerce_elavon']['commerce_elavon_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => variable_get('commerce_elavon_title', 'Elavon'),
    '#description' => t('Override the display title of the Elavon Payment Method.'),
  );
  
  $form['commerce_elavon']['commerce_elavon_server'] = array(
    '#type' => 'radios',
    '#title' => t('Elavon Server'),
    '#options' => array(
      0 => t('Test / Demo'),
      1 => t('Production / Live'),
      2 => t('None (reply with dummy acceptance)'),
    ),
    '#default_value' => variable_get('commerce_elavon_server', 0),
    '#required' => TRUE,
  );

  $form['commerce_elavon']['commerce_elavon_merchant_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Merchant ID'),
    '#default_value' => variable_get('commerce_elavon_merchant_id', ''),
    '#required' => TRUE,
  );

  $form['commerce_elavon']['commerce_elavon_user_id'] = array(
    '#type' => 'textfield',
    '#title' => t('User ID'),
    '#default_value' => variable_get('commerce_elavon_user_id', ''),
    '#required' => TRUE,
  );

  $form['commerce_elavon']['commerce_elavon_pin'] = array(
    '#type' => 'textfield',
    '#title' => t('PIN'),
    '#default_value' => variable_get('commerce_elavon_pin', ''),
    '#required' => TRUE,
  );

  $form['commerce_elavon']['commerce_elavon_optional_fields'] = array(
    '#type' => 'checkboxes',
    '#title' => 'Optional Form Elements',
    '#options' => commerce_elavon_form_fields(),
    '#default_value' => variable_get('commerce_elavon_optional_fields', array()),
    '#description' => t('Select optional fields to display on the credit card form.'),
  );
  
  module_load_include('inc', 'commerce_payment', 'includes/commerce_payment.credit_card');
  $form['commerce_elavon']['commerce_elavon_credit_card_types'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Credit Card Types'),
    '#options' => commerce_payment_credit_card_types(),
    '#default_value' => variable_get('commerce_elavon_credit_card_types', array()),
    '#description' => t('Select supported credit card types.'),
  );

  $form['commerce_elavon']['ssl'] = array(
    '#type' => 'fieldset',
    '#title' => t('Elavon SSL Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['commerce_elavon']['ssl']['commerce_elavon_verify_peer'] = array(
    '#type' => 'checkbox',
    '#title' => t('Validate Elavon SSL Certificate'),
    '#default_value' => variable_get('commerce_elavon_verify_peer', 0),
  );

  $form['commerce_elavon']['ssl']['commerce_elavon_cert_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Certificate'),
    '#description' => t('Path to the Elavon SSL Certificate cacert.pem file.'),
    '#default_value' => variable_get('commerce_elavon_cert_path', ''),
  );

  return system_settings_form($form);
}
