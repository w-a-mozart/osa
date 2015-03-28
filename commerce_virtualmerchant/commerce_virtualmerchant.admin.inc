<?php

/**
 * @file
 * Administration form for the Commerce Converge Virtual Merchant Form.
 */
require_once 'commerce_virtualmerchant.inc';

/**
 * Menu callback; Displays the administration settings for Converge Virtual Merchant Form.
 */
function commerce_virtualmerchant_admin_settings() {
  $form = array();

  $form['commerce_virtualmerchant'] = array(
    '#type' => 'fieldset',
    '#title' => t('Converge Virtual Merchant settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $form['commerce_virtualmerchant']['commerce_virtualmerchant_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => variable_get('commerce_virtualmerchant_title', 'Converge Virtual Merchant'),
    '#description' => t('Override the display title of the Converge Virtual Merchant Form Method.'),
  );
  
  $form['commerce_virtualmerchant']['commerce_virtualmerchant_server'] = array(
    '#type' => 'radios',
    '#title' => t('Converge Server'),
    '#options' => array(
      0 => t('Test / Demo'),
      1 => t('Production / Live'),
      2 => t('None (reply with dummy acceptance)'),
    ),
    '#default_value' => variable_get('commerce_virtualmerchant_server', 0),
    '#required' => TRUE,
  );

  $form['commerce_virtualmerchant']['commerce_virtualmerchant_merchant_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Merchant ID'),
    '#default_value' => variable_get('commerce_virtualmerchant_merchant_id', ''),
    '#required' => TRUE,
  );

  $form['commerce_virtualmerchant']['commerce_virtualmerchant_user_id'] = array(
    '#type' => 'textfield',
    '#title' => t('User ID'),
    '#default_value' => variable_get('commerce_virtualmerchant_user_id', ''),
    '#required' => TRUE,
  );

  $form['commerce_virtualmerchant']['commerce_virtualmerchant_pin'] = array(
    '#type' => 'textfield',
    '#title' => t('PIN'),
    '#default_value' => variable_get('commerce_virtualmerchant_pin', ''),
    '#required' => TRUE,
  );

  $form['commerce_virtualmerchant']['vmform'] = array(
    '#type' => 'fieldset',
    '#title' => t('Converge Virtual Merchant Form Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  
  $form['commerce_virtualmerchant']['vmform']['commerce_virtualmerchant_form_height'] = array(
    '#type' => 'textfield',
    '#title' => t('Form Height'),
    '#description' => t('Height of the Converge Virtual Merchant Payment Form.'),
    '#default_value' => variable_get('commerce_virtualmerchant_form_height', '300px'),
  );

  $form['commerce_virtualmerchant']['vmform']['commerce_virtualmerchant_form_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Form Width'),
    '#description' => t('Width of the Converge Virtual Merchant Payment Form.'),
    '#default_value' => variable_get('commerce_virtualmerchant_form_width', '100%'),
  );

  $form['commerce_virtualmerchant']['ssl'] = array(
    '#type' => 'fieldset',
    '#title' => t('Converge SSL Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['commerce_virtualmerchant']['ssl']['commerce_virtualmerchant_verify_peer'] = array(
    '#type' => 'checkbox',
    '#title' => t('Validate Converge SSL Certificate'),
    '#default_value' => variable_get('commerce_virtualmerchant_verify_peer', 0),
  );

  $form['commerce_virtualmerchant']['ssl']['commerce_virtualmerchant_cert_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Certificate'),
    '#description' => t('Path to the Converge SSL Certificate cacert.pem file.'),
    '#default_value' => variable_get('commerce_virtualmerchant_cert_path', ''),
  );

  return system_settings_form($form);
}
