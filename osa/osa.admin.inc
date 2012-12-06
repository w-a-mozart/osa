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

  $form['osa']['teacher_pay'] = array(
    '#type' => 'fieldset',
    '#title' => t('Teacher payment settings'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['osa']['teacher_pay']['osa_start_date'] = array(
    '#type' => 'textfield',
    '#title' => t('Start of year'),
    '#default_value' => variable_get('osa_start_date', 'first day of September'),
    '#description' => t('Set start date of the school year & first payment date. See PHP Supported Date and Time Formats (http://www.php.net/manual/en/datetime.formats.php)'),
    '#required' => TRUE,
  );

  $form['osa']['teacher_pay']['osa_num_installments'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of installments'),
    '#default_value' => variable_get('osa_num_installments', 10),
    '#required' => TRUE,
    '#description' => t('Total number of installment payments / months in the school year'),
    '#required' => TRUE,
  );

  $num_installments = variable_get('osa_num_installments', 10);
  $start_date = _osa_next_payment_date(variable_get('osa_start_date', 'first day of September'), $num_installments);
  
  $form['osa']['teacher_pay']['osa_prorate_teacher_pay'] = array(
    '#type' => 'radios',
    '#title' => t('Pro-rate teacher pay'),
    '#options' => array(
      FALSE => t('Charge full annual amount'),
      TRUE => t('Pro-rate based on number of months since start date'),
    ),
    '#default_value' => variable_get('osa_prorate_teacher_pay', TRUE),
    '#required' => TRUE,
    '#description' => t('Pro-rate teacher pay by %1 / %2 of annual pay', array('%1' => $num_installments, '%2' => variable_get('osa_num_installments', 10))),
  );

  $form['osa']['teacher_pay']['osa_teacher_pay_override'] = array(
    '#type' => 'textarea',
    '#title' => t('Teacher pay overrides'),
    '#default_value' => variable_get('osa_teacher_pay_override'),
    '#description' => '<p>' . t('This field can be used to override the teacher fee for a specific student.') . '<br />' .
                      t('Enter one value per line, in the format student-contact-id|total-amount') . '<br />' .
                      t('E.g. 123|987.65  will override the total teacher fee for student 123 to $987.65') . '</p>',
  );

  return system_settings_form($form);
}
