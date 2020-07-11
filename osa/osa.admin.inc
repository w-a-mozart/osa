<?php

/**
 * @file
 * Administration form for the OSA.
 */
require_once 'osa.inc';

/**
 * Menu callback; Displays the configurable settings for OSA.
 */
function osa_admin_settings() {
  $form = array();

  $form['osa'] = array(
    '#type' => 'fieldset',
    '#title' => t('OSA configuration'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['osa']['osa_start_date'] = array(
    '#type' => 'textfield',
    '#title' => t('Membership Start Date'),
    '#default_value' => variable_get('osa_start_date', 'first day of September'),
    '#description' => t('Set start date for new memberships. See PHP Supported Date and Time Formats (http://www.php.net/manual/en/datetime.formats.php)'),
    '#required' => TRUE,
  );

  $form['osa']['osa_discount_types'] = array(
    '#type' => 'textarea',
    '#title' => t('Discount types'),
    '#default_value' => variable_get('osa_discount_types'),
    '#description' => '<p>' . t('List all of the different Discount types offered.') . '<br />' .
                      t('Enter one value per line. The amount of each discount is controlled by Pricing rules') . '<br />' .
                      t('ONLY ADD TO THE END. Do not remove discount types or add new types into the middle of the list.') . '</p>',
  );
  
  return system_settings_form($form);
}
