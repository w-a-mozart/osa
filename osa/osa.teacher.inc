<?php

/**
 * @file
 * Functions for Private Teacher functionality
 */
require_once 'osa.inc';

/**
 * Custom procesing for the Teacher payment form
 *
 * @param $form
 *   The CRM_Contribute_Form_Contribution_Main object being rendered.
 */
function osa_civicrm_buildForm_teacher_registration(&$form) {
  // populate the teacher QE Park pricing
  _osa_get_teacher_lists($contacts, $select_options, $pricelevel, $prices30min, $prices45min, $prices60min);
  $form->assign('pricelvl', json_encode($pricelevel));
  $form->assign('price30',  json_encode($prices30min));
  $form->assign('price45',  json_encode($prices45min));
  $form->assign('price60',  json_encode($prices60min));
  
  // @TODO not sure why this is needed, but form validation fails wothout it
  $form->addElement('hidden', 'amount_other', '0.01');
}

/**
 * Custom validation for the Teacher payment form
 */
function osa_civicrm_validate_teacher_registration(&$fields, &$files, &$form) {
  $errors = array();

  // custom processing for the teacher registration form
  $values = $form->_submitValues;

  // check to make sure the selected teacher teaches at the selected location
  if (isset($values[_osa_field('selected_teacher')]) &&
      isset($values[_osa_field('lesson_location')]) &&
      ($values[_osa_field('lesson_location')] == 'QE Park')) {
    $teacher = _osa_get_contact($values[_osa_field('selected_teacher')]);
    if ($teacher['uses_qep'] == '0') {
      $errors[_osa_field('lesson_location')] = ts('%1 does not teach private lessons at QE Park', array( 1 => $teacher['display_name'] ));
      $form->setElementError(_osa_field('lesson_location'), $errors[_osa_field('lesson_location')]);
    }
  }

  return empty($errors) ? true : $errors;
}

/**
 * Custom procesing for the Teacher payment confirmation form
 */
function osa_civicrm_buildForm_teacher_registration_confirm(&$form) {

  // we need to reset the amount based on the options selected
  $lesson_location = $form->_params[_osa_field('lesson_location')];
  if ($lesson_location != 'QE Park') {
    $form->_amount = 0;
    $form->_params['amount'] = 0;
    $form->assign('amount', 0);
    return;
  }

  // get the payment options values
  $fee_options = $form->_values['fee'];
  $line_item_grps = $form->_lineItem;
  $line_items = current($line_item_grps);
  $line_key = key($line_item_grps);
  foreach ($line_items as $line_item) {
    $name = $fee_options[$line_item['price_field_id']]['name'];
    $value = $fee_options[$line_item['price_field_id']]['options'][$line_item['price_field_value_id']]['name'];
    $price_set[$name] = array('id' => $line_item['price_field_value_id'], 'value' => $value);
  }

  // get the teacher payments amounts
  _osa_get_teacher_lists($contacts, $select_options, $pricelevel, $prices30min, $prices45min, $prices60min);
  $pay = array(30 => $prices30min, 45 => $prices45min, 60 => $prices60min);
  $teacher_id = $form->_params[_osa_field('selected_teacher')];
  $lesson_level = $form->_params[_osa_field('lesson_level')];
  $ann_price = $pay[$price_set['Lesson_Length']['value']][$teacher_id][$lesson_level];
  $unit_price = $ann_price;
  $num_installments = 1;
  
  // change the form to show installments
  if ($price_set['payment_option']['value'] = 'Monthly_Installments') {

    // get the number of installments & set unit price
    $total_installments = $num_installments = variable_get('osa_num_installments', 10);
    $start_date = _osa_next_payment_date(variable_get('osa_start_date', 'first day of September'), $num_installments);
    $pro_rate = variable_get('osa_prorate_teacher_pay', TRUE);
    $unit_price = ($pro_rate) ? ($ann_price / $total_installments) : ($ann_price / $num_installments);


    // this function is called by the thankyou form as well, so check before changing the button
    if ($form instanceof CRM_Contribute_Form_Contribution_Confirm) {
      $group = $form->getElement('buttons');
      $buttons = $group->getElements();
      $buttons[0]->setValue(ts('Make Installments'));
      $form->assign('button', ts('Make Installments'));
    }
  }

  // update the form variables
  $total = $num_installments * $unit_price;
  $form->_amount = $total;
  $form->_params['amount'] = $total;
  $form->_lineItem[$line_key][$price_set['Lesson_Length']['id']]['qty'] = $num_installments;
  $form->_lineItem[$line_key][$price_set['Lesson_Length']['id']]['unit_price'] = $unit_price;
  $form->_lineItem[$line_key][$price_set['Lesson_Length']['id']]['line_total'] = $num_installments * $unit_price;

  // update the template variables
  $form->assign('lineItem', $form->_lineItem);
  $form->assign('amount', $num_installments * $unit_price);
}

/**
 * Custom processing for the Teacher payments
 */
function osa_civicrm_alterPaymentProcessorParams_teacher_registration($paymentObj, &$rawParams, &$cookedParams) {

  // convert to installments
  // if ($rawParams['price_1'] == 'Monthly_Installments') {
  if ($rawParams['price_1'] == 3) {
    // get the number of installments
    $total_installments = $num_installments = variable_get('osa_num_installments', 10);
    $start_date = _osa_next_payment_date(variable_get('osa_start_date', 'first day of September'), $num_installments);

    // determine the amount
    $pro_rate = variable_get('osa_prorate_teacher_pay', TRUE);
    $ann_price = $rawParams['amount'];
    $unit_price = ($pro_rate) ? ($ann_price / $total_installments) : ($ann_price / $num_installments);
    $cookedParams['ssl_amount'] = $unit_price;

    // set the Elavon installment parameters
    $cookedParams['ssl_transaction_type'] = 'ccaddinstall';
    $cookedParams['ssl_billing_cycle'] = 'MONTHLY';
    $cookedParams['ssl_end_of_month'] = 'N';
    $cookedParams['ssl_next_payment_date'] = $start_date->format('m/d/Y');
    $cookedParams['ssl_total_installments'] = $num_installments;
    $cookedParams['ssl_skip_payment'] = 'N';
  }
}

/**
 * Custom processing after the Teacher payment confirmation form is submitted
 */
function osa_civicrm_postProcess_teacher_registration_confirm(&$form) {

  // we track the student/teachers using relationships
  $contact_id_a = $form->_params['contact_id'];
  $contact_id_b = $form->_params[_osa_field('selected_teacher')];
  
  // figure out the end date of the relationship
  $num_installments = variable_get('osa_num_installments', 10);
  $start_date = _osa_next_payment_date(variable_get('osa_start_date', 'first day of September'), $num_installments);
  $end_date = new DateTimeEx($start_date);
  $tmp = $num_installments - 1;
  $end_date->modify("last day of +$tmp months");

  // create a relationship between the student and the teacher
  $lesson_length = array(11 => '30 minutes', 12 => '45 minutes', 13 => '60 minutes');
  $params = array(
    'contact_id_a'         => $contact_id_a,
    'contact_id_b'         => $contact_id_b,
    'relationship_type_id' => OSA_REL_TEACHER,
    'is_active'            => 1,
    'start_date'           => $start_date->format("Ymd"),
    'end_date'             => $end_date->format("Ymd"),
    'custom_14'            => $form->_params['custom_19'], // instrument
    'custom_15'            => $form->_params['custom_18'], // location
    'custom_16'            => $lesson_length[$form->_params['price_6']], // lesson length
    'version'              => '3',
  );

  // Call the CiviCRM API to create the relationship
  $result = civicrm_api('relationship', 'create', $params);
  if ($result['is_error']) {
    watchdog('osa', 'Error: %1\nfailed to create student/teacher relationship between cid_a=%2 and cid_b=%3', 
             array(1 => $result['error_message'], 2 => $contact_id_a, 3 => $contact_id_b,),
             WATCHDOG_ERROR);
  }
}

/**
 * Customize the Teacher payments thank you form
 */
function osa_civicrm_buildForm_teacher_registration_thankyou(&$form) {
  // the confim and thankyou forms show identical information
  osa_civicrm_buildForm_teacher_registration_confirm($form);
}

/**
 * Helper to get teacher private lesson pay for QEP
 *
 * @param integer $contact_id
 *   contact who's events we're to get
 */
function _osa_get_teacher_lists( &$contacts, &$select_options, &$pricelevel, &$prices30min, &$prices45min, &$prices60min) {

  static $cache = array();
  
  if (empty($cache)) {
    $cache['contacts'] = array();
    $cache['select_options'] = array();
    $cache['pricelevel'][0] = array(0 => 'Standard');
    $cache['prices30min'][0] = array(0 => 0);
    $cache['prices45min'][0] = array(0 => 0);
    $cache['prices60min'][0] = array(0 => 0);

    $result = civicrm_api('contact', 'get',  array('contact_sub_type' => 'Teacher', 'option.sort' =>'sort_name', 'version' => 3));
    if ($result['is_error']) {
      watchdog('osa', 'Error: %1\nfailed to get teacher contacts', array(1 => $result['error_message'],), WATCHDOG_ERROR);
    }

    foreach ( $result['values'] as $teacher_id => $contact ) {
      $teacher = _osa_get_contact($teacher_id);
      $cache['contacts'][$teacher_id] = $teacher;
      $cache['select_options'][$teacher_id] = $teacher['display_name'];
      if ($teacher['uses_qep']) {
        $cache['pricelevel'][$teacher_id] = $teacher['lesson_fee_level'];
        $cache['prices30min'][$teacher_id] = $teacher['Price_for_30_minutes'];
        $cache['prices45min'][$teacher_id] = $teacher['Price_for_45_minutes'];
        $cache['prices60min'][$teacher_id] = $teacher['Price_for_60_minutes'];
      }
    }
  }
  
  $contacts = $cache['contacts'];
  $select_options = $cache['select_options'];
  $pricelevel = $cache['pricelevel'];
  $prices30min = $cache['prices30min'];
  $prices45min = $cache['prices45min'];
  $prices60min = $cache['prices60min'];
}
