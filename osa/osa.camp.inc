<?php

/**
 * @file
 * Functions for Summer Camp functionality
 */
require_once 'osa.inc';

/**
 * Calculate the price of a camp item. Special pricing and discounts are 
 * handled by rules in the Shopping Cart
 *
 */
function osa_calc_camp_price($name, $value, $other_values) {

  // create a price table
  // @todo put this in a database or as part of the webform
  static $prices = array(
    'Early Childhood Music' => array(
      'One Week' => 60.00,
      'Both Weeks' => 110.00,
    ),
    'Junior Camp' => array(
      'One Week' => 240.00,
      'Both Weeks' => 450.00,
    ),
    'Intermediate Camp' => array(
      'One Week' => 285.00,
      'Both Weeks' => 520.00,
    ),
    'Senior Camp' => array(
      'One Week' => 335.00,
      'Both Weeks' => 610.00,
    ),
    'Leader In Training' => array(
      'One Week' => 200.00,
      'Both Weeks' => 350.00,
    ),
    'Student Leader' => array(
      'One Week' => 150.00,
      'Both Weeks' => 280.00,
    ),
    'Student Leader Guitar and Piano' => array(
      'One Week' => 100.00,
      'Both Weeks' => 180.00,
    ),
    'primary_instrument_enhancements' => array(
      'Additional 15min Private Lesson' => 60.00,
    ),
    'primary_instrument_enhancements_leaders' => array(
      '15min Private Lesson' => 60.00,
      '30min Private Lesson' => 120.00,
    ),
    'second_instrument_enhancements' => array(
      'Second Instrument 15min Private Lesson' => 60.00,
      'Second Instrument 30min Private Lesson' => 120.00,
    ),
    'chamber_group' => array(
      'Chamber Group' => 30.00,
    ),
    'masterclass_terry_durbin' => array(
      'Terry Durbin Violin Masterclass' => 45.00,
    ),
    'daycare_option' => array(
      'AM and PM Care' => 130.00,
      'AM Care' => 55.00,
      'PM Care' => 90.00,
    ),
    'lunch_option' => array(
      'Student Lunch' => 37.00,
      'Student Lunch Both Weeks' => 70.00,
      'Parent Lunch' => 37.00,
      'Parent Lunch Both Weeks' => 70.00,
    ),
    'teacher' => array(
      'Terry Durbin: 15min lessons' => 60.00,
      'Terry Durbin: 30min lessons' => 120.00,
      'TJ McAvaney: 15min lessons' => 60.00,
      'TJ McAvaney: 30min lessons' => 120.00,
    ),
  );

  // fiddle with the names / values
  if ($name == 'weeks') {
    $name = is_array($other_values['programs']['value']) ? $other_values['programs']['value'][0]: $other_values['programs']['value'];
    if ($value != 'Both Weeks') {
      $value = 'One Week';
    }
  }
  if (($name == 'primary_teacher') || ($name == 'secondary_teacher')) {
    $name = 'teacher';
  }
  else {
    $name = str_replace(array('_wk1', '_wk2'), '', $name);
  }

  // account for multi-select items (any checkbox)
  $values = is_array($value) ? $value : array($value);
  $price = 0.00;
  foreach($values as $val) {
    if (!empty($val)) {
      $val = str_replace(array(' in Week 1', ' in Week 2'), '', $val);
      $price += (isset($prices[$name][$val])) ? $prices[$name][$val] : 0.00;
    }
  }

  return $price;
}

// special logic for teacher premiums
function _osa_camp_teacher_premiums($components, $submitted, &$component, &$form_key, &$value) {
  $teacher = $value;
  if (($teacher != 'Terry Durbin') && ($teacher != 'TJ McAvaney')) {
    return FALSE;
  }

  // get the some of the data we need
  $program = is_array($submitted[4]) ? $submitted[4][0] : $submitted[4];
  $weeks   = is_array($submitted[12]) ? $submitted[12][0] : $submitted[12];
  $week[1] = (($weeks == 'Week 1') || ($weeks == 'Both Weeks')) ? 1 : 0;
  $week[2] = (($weeks == 'Week 2') || ($weeks == 'Both Weeks')) ? 1 : 0;
  // special condition where TD is only available week 1
  $week[2] = ($teacher == 'Terry Durbin') ? 0 : $week[2];
  
  list($instrument, $other) = explode('_', $form_key, 2);
  $instrument = ($instrument == 'secondary') ? 'second' : $instrument; // why didn't I name them the same???
  $leader = (strpos($program, 'Leader') !== FALSE);

  // need to figure out lesson length in each week
  foreach ($week as $wknum => $attending) {
    $lesson_length[$wknum] = 0;
  }

  // 15min is included in primary instrument (except for Leaders)
  if (($instrument == 'primary') && !$leader) {
    foreach ($week as $wknum => $attending) {
      $lesson_length[$wknum] += 15 * $attending;
    }
  }

  // add enhancements
  $leaders = ($leader && ($instrument == 'primary')) ? 'leaders_' : '';
  foreach ($week as $wknum => $attending) {
    $keyname = "{$instrument}_instrument_enhancements_{$leaders}wk{$wknum}";
    $cid = _osa_get_webform_component_id($components, $keyname);

    $enh_str = '';
    if (isset($submitted[$cid])) {
      $enh_str = is_array($submitted[$cid]) ? $submitted[$cid][0] : $submitted[$cid];
    }

    $tmp = array();
    $enh_min = (preg_match( '/[0-9]+min/', $enh_str, $tmp) === 1) ? intval(substr($tmp[0], 0, strlen($tmp[0]) - 3)) : 0;
    $lesson_length[$wknum] += $enh_min * $attending;
  }

  // quit if no premium minutes
  $total_length = 0;
  foreach ($week as $wknum => $attending) {
    $total_length += $lesson_length[$wknum] * $attending;
  }
  if ($total_length == 0) {
    return FALSE;
  }
  
  // Hack the component values to deal with the teacher
  $component['name'] = "Teacher Premium";
  $value = array();
  foreach ($week as $wknum => $attending) {
    if ($lesson_length[$wknum] > 0) {
      $value[] = "{$teacher}: {$lesson_length[$wknum]}min lessons in Week {$wknum}";
    }
  }

  return TRUE;
}

/**
 * Modify the camp application webform
 */
function osa_form_alter_summer_camp(&$form, &$form_state) {

  // find out if this is a new or existing submission
  $path = request_path();
  $matches = array();
  if (preg_match('#node\/(?P<nid>\d+)\/submission\/(?P<sid>\d+)#', $path, $matches) !== 1) {
    // new submission, so edit the form for the end user
    _osa_form_alter_summer_camp($form, $form_state);
    // add a validation routine so we get called even when there is an error
    $form['#validate'][] = '_osa_camp_validate';
  }
  else {
    // existing submission, so make it easier on the admin editing the data
    static $first_time = TRUE;
    if ($first_time) {
      // save what we need from the current form
      $node = $form['#node'];
      $submission = $form['#submission'];

      $form = webform_client_form($form, $form_state, $node, $submission, FALSE, FALSE);
      $form['#attached']['css'][] = drupal_get_path('module', 'osa') . '/osa.camp.css';
      $form['details']['page_num']['#value'] = $form['details']['page_count']['#value'];
      unset($form['actions']['next']);
      $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
        '#weight' => 10,
      );

      $form['#validate'] = array();

      $first_time = FALSE;
    }
  }
}

function _osa_camp_validate($form, &$form_state) {
  // call the shared routine
  _osa_form_alter_summer_camp($form, $form_state);
  return $form;
}

/**
 * Modify the camp application webform to include a javascript setting
 * This setting is used to control the "running total" display
 */
function _osa_form_alter_summer_camp(&$form, &$form_state) {
  // only process once for a given page
  static $already_called = array();
  $page_num = $form_state['webform']['page_num'];
  if (isset($already_called[$page_num])) {
    return;
  }
  $already_called[$page_num] = TRUE;
  
  $settings = array();
  $values = array();
  $amount = 0.00;
  $html = '';

  // there are some pages we don't want to display the "running total" on
  $exclude_pages = array(1, 2, 5, 8, 9);
  if (array_search($page_num, $exclude_pages) === FALSE) {
    $settings['display'] = TRUE;
    
    // get the submitted values of all of the preceeding pages
    $components = $form['#node']->webform['components'];
    $submitted = is_array($form_state['storage']['submitted']) ? $form_state['storage']['submitted'] : array();
    _osa_camp_parse_submission($components, $submitted, $values, $amount, $html, 'long', $page_num);

    // set mark up to display
    $settings['markup'] = $html;
  }
  else {
    $settings['display'] = FALSE;
  }
  drupal_add_js(array('osa_camp' => $settings), 'setting');
  drupal_add_js(drupal_get_path('module', 'osa') . '/osa.camp.js');  

  // want to change the title of the page if past page 1.
  if ($page_num > 1) {
    civicrm_initialize();
    $component = &_osa_get_webform_component($form['#node'], 'camp_participant');
    $contact = _osa_get_contact($form_state['values']['submitted'][$component['cid']], FALSE);
    drupal_set_title($form['#node']->title . ' for <font color=red>' . check_plain($contact['display_name']) . '</font>', PASS_THROUGH);
  }
}

/**
 * Cleanup the submission before saving it to the db
 */
function osa_webform_submission_presave_summer_camp(&$node, &$submission) {
  // unset the Recorder Level if Recorder Option not set
  // same with Theory & Ukulele_level
  $is_set = array(
    'recorder_level' => FALSE,
    'theory_level'   => FALSE,
    'ukulele_level'  => FALSE,
  );

  $component = &_osa_get_webform_component($node, 'programs');
  $programs = $submission->data[$component['cid']][0];
  if ($programs == 'Senior Camp') {
    $component = &_osa_get_webform_component($node, 'options_senior_program');
    $options_senior_program = $submission->data[$component['cid']];
    
    if ((isset($options_senior_program['Recorder'])) && ($options_senior_program['Recorder'] != 'NT')) {
      $is_set['recorder_level'] = TRUE;
    }
    if ((isset($options_senior_program['RCM Theory'])) && ($options_senior_program['RCM Theory'] != 'NT')) {
      $is_set['theory_level'] = TRUE;
    }
  }
  elseif ($programs == 'Intermediate Camp') {
    $component = &_osa_get_webform_component($node, 'options_intermediate_program');
    $options_intermediate_program = $submission->data[$component['cid']];
    if ((isset($options_intermediate_program['Ukulele'])) && ($options_intermediate_program['Ukulele'] != 'NT')) {
      $is_set['ukulele_level'] = TRUE;
    }
  }

  foreach ($is_set as $name => $set) {
    if (!$set) {
      $component = &_osa_get_webform_component($node, $name);
      unset($submission->data[$component['cid']]);
    }
  }
}
  
/**
 * Process the camp form after it is submitted
 * We need to create the participation record in civicrm and
 * Add the line item to the shopping cart
 */
function osa_webform_submission_insert_summer_camp($node, $submission) {
  civicrm_initialize();

  // create the values array & line item description
  $values = array();
  $desc   = '';
  $amount = 0.00;
  _osa_camp_parse_submission($node->webform['components'], $submission->data, $values, $amount, $desc, 'short');

  // need this year's event ids
  $params = array(
    'event_type_id' => OSA_EVENT_SUMMER_CAMP,
    'filter.start_date_low' => date('Y0101000000'),
    'version' => 3,
  );
  $result = civicrm_api('event', 'get', $params);
  $events = array();
  foreach ($result['values'] as $event_id => $event) {
    if (strpos($event['title'], 'Week 1') !== FALSE) {
      $events['Week 1'] = $event_id;
    }
    elseif (strpos($event['title'], 'Week 2') !== FALSE) {
      $events['Week 2'] = $event_id;
    }
    else {
      $events['Both Weeks'] = $event_id;
    }
  }

  // create a pending participant record
  $contact_id = $values['camp_participant']['value'];
  $event_id   = $events[$values['weeks']['value']];
  
  $params = array(
    'contact_id' => $contact_id,
    'event_id' => $event_id,
    'fee_level' => $values['programs']['value'],
    'fee_amount' => $amount,
    'fee_currency' => 'CAD',
    'status_id' => 6,
    'role_id' => 1,
    'register_date' => date('Y-m-d H:i:s'),
    'source' => 'Online Camp Registration',
    'version' => 3,
  );
  $result = civicrm_api('participant', 'create', $params);
  if ($result['is_error'] <> 0) {
    watchdog('osa', "Error creating Camp Participant for cid $contact_id %1", array('%1' => $result['error_message']), WATCHDOG_ERROR);
    drupal_set_message('An unexpected error has occured. Please contact the administrator', 'error');
  }
  $participant_id = $result['id'];

  $params = array(
    'entity_id' => $participant_id,
    'custom_78' => $submission->sid,
    'custom_79' => $GLOBALS['base_url'] . "/node/{$node->nid}/submission/{$submission->sid}",
    'version' => 3,
  );
  $result = civicrm_api('CustomValue', 'Create', $params);
  
  $params = array(
    'contact_id' => $contact_id,
    'financial_type_id' => 10,
    'contribution_status_id' => 2,
    'payment_instrument_id' => 1,
    'amount_level' => $values['programs']['value'],
    'total_amount' => $amount,
    'net_amount' => $amount,
    'source' => 'Online Camp Registration',
    'version' => 3,
  );  
  $result = civicrm_api('contribution', 'create', $params);
  if ($result['is_error'] <> 0) {
    watchdog('osa', "Error creating Camp Contribution for cid $contact_id %1", array('%1' => $result['error_message']), WATCHDOG_ERROR);
    drupal_set_message('An unexpected error has occured. Please contact the administrator', 'error');
  }
  $contribution_id = $result['id'];

  $params = array( 
    'participant_id' => $participant_id,
    'contribution_id' => $contribution_id,
    'version' => 3,
  );
  $result = civicrm_api('participant_payment', 'create', $params);
  if ($result['is_error'] <> 0) {
    watchdog('osa', "Error creating participant_payment for cid $contact_id, participant_id $participant_id, contribution_id $contribution_id %1", array('%1' => $result['error_message']), WATCHDOG_ERROR);
    drupal_set_message('An unexpected error has occured. Please contact the administrator', 'error');
  }
  
  // add it to the drupal cart
  // @see civicrm_commerce_line_item_add_new
  $product = commerce_product_load_by_sku('CiviCRM');
  $product_wrapper = entity_metadata_wrapper('commerce_product', $product);
  $product_wrapper->title = 'Summer Camp';
  $product_wrapper->commerce_price->amount = 1;
  $product_wrapper->commerce_price->currency_code = 'CAD';
  $line_item = commerce_product_line_item_new($product, 1, 0, get_object_vars($submission), 'civi_event');
  $line_item_wrapper = entity_metadata_wrapper('commerce_line_item', $line_item);
  $line_item_wrapper->line_item_label = $desc;
  $line_item_wrapper->civicrm_commerce_amount = $amount;
  $line_item_wrapper->civicrm_commerce_currency_code = 'CAD';
  $line_item_wrapper->civicrm_commerce_contribution_id = $contribution_id;
  $line_item_wrapper->civicrm_commerce_contact_id = $contact_id;
  $line_item_wrapper->civicrm_commerce_event_id = $event_id;
  $line_item_wrapper->civicrm_commerce_participant_id = $participant_id;
  commerce_cart_product_add(_osa_getUserId(), $line_item, FALSE);
}

/**
 * Process the camp registration values
 *
 * @param $components
 *   The webform components to be parsed
 * @param $submitted
 *   All the values submitted for a form, keyed by the component IDs. This may be
 *   pulled from $form_state['values']['submitted'] or $submission->data.
 * @param $values
 *   Output parameter. Returns the component value and price, keyed by form_key
 * @param $total
 *   Output parameter. Total cost of the camp fees.
 * @param $html
 *   Output parameter. A summary of the submission as HTML
 * @param $style
 *   Flag indicating what should be included in the HTML.
 * @param $page_num
 *   Only parse components upto the given page number on the form.
 *
 */
function _osa_camp_parse_submission($components, $submitted, &$values, &$total, &$html, $style, $page_num = 999) {

  $total = 0.00;
  $ignore_types = array('pagebreak', 'markup', 'fieldset', 'textarea', 'grid', 'textfield',);
  $ignore_elements = array('caregiver_present', 'primary_instrument_level', 'add_a_second_instrument', 'second_instrument_level', 'ukulele_level', 'recorder_level', 'theory_level',);
  $rows = array();

  // loop through each component we care about and get it's value and price
  uasort($components, 'drupal_sort_weight');
  foreach ($components as $id => $component) {
    if (($component['page_num'] < $page_num) && (array_search($component['type'], $ignore_types) === FALSE) && (array_search($component['form_key'], $ignore_elements) === FALSE)) {
      $form_key = $component['form_key'];
      $value = NULL;
      if (isset($submitted[$id])) {
        if (is_array($submitted[$id]) && count($submitted[$id]) == 1) {
          $value = $submitted[$id][0];
        }
        else {
          $value = $submitted[$id];
        }
      }
      if (!empty($value) && ($value != 'NA')) {
        // nasty override logic for teacher premiums
        if (($form_key == 'primary_teacher') || ($form_key == 'secondary_teacher')) {
          if (!_osa_camp_teacher_premiums($components, $submitted, $component, $form_key, $value)) {
            continue;
          }
        }

        $values[$form_key] = array(
          'id' => $id,
          'value' => $value,
          'price' => osa_calc_camp_price($form_key, $value, $values),
        );
        $total += $values[$form_key]['price'];
        $rows = array_merge($rows, _osa_camp_display_component($component, $values[$form_key], $style, $values));
      }       
    }
  }
  
  // include the total in the results if asked
  if ($style == 'long') {
    $component['form_key'] = 'total';
    $component['name'] = ' ';
    $rows = array_merge($rows, _osa_camp_display_component($component, array('value' => 'Total', 'price' => $total), $style, $values));
  }

  // render the html
  $html = htmlspecialchars_decode(theme('table', array('rows' => $rows, 'attributes' => array('class' => 'osa-camp-submission', 'style' => 'width: 100%;'))));
}
  
/**
 * Create one or more table rows for a given component
 */
function _osa_camp_display_component($component, $value, $style, $values, $format = 'html') {

  $form_key = $component['form_key'];
  $class = str_replace('_', '-', $form_key);

  // we are creating a table with a row for the title (if required) and a row for the item and price
  $rows = array();

  // create the Title row
  if ($style == 'long') {
    // if both weeks are selected don't add the title again
    if (!((substr_compare($form_key, '_wk2', -4) == 0) && (array_key_exists(substr_replace($form_key, '_wk1', -4), $values)))) {
      // remove the Week & St. Mildred's from the label
      $remove = array(' for Week 1', ' for Week 2', ' - Week 1', ' - Week 2', ' - Offered by St. Mildred\'s-Lightbourn School');
      $data = str_replace($remove, '', $component['name']);
      $rows[] = array(
        'data' => array(
          array(
            'data' => $data,
            'colspan' => '2'
          ),
        ),
        'class' => array('osa-camp-item-title', "osa-camp-$class"),
      );
    }
  }

  // display the contact name
  if ($form_key == 'camp_participant') {
    civicrm_initialize();
    $contact = _osa_get_contact($value['value'], FALSE);
    $value['value'] = $contact['display_name'];
  }

  // create the Item row
  $rows[] = array(
    'data' => array(
      array(
        'data' => is_array($value['value']) ? implode(', and ', $value['value']) : $value['value'],
        'class' => array('osa-camp-item-label'),
      ),
      array(
        'data' => ($value['price'] > 0.001) ? '$ ' . number_format($value['price'], 2) : ' ',
        'class' => array('osa-camp-item-price'),
        'style' => array('text-align: right;', 'vertical-align: top;', 'white-space: nowrap;'),
      ),
    ),
    'class' => array('osa-camp-item-content', "osa-camp-$class"),
  );

  return $rows;
}

/**
 * Implements hook_views_query_alter() to join the civicrm_participant and the webform_submissions tables
 */
function osa_views_query_alter_summer_camp_students(&$view, &$query) {
  
  // get the webform table definition
  $webform_table = $query->table_queue['webform_submissions_users'];
  
  // replace the join from the user table to the custom field table
  $new_join = new views_join();
  $new_join->construct('webform_submissions', 'civicrm_value_camp_details_18', 'submission_id_78', 'sid');
  $webform_table['join'] = $new_join;

  // move to the end
  $tmp = $query->table_queue['users_webform_submissions'];
  unset($query->table_queue['users_webform_submissions']);
  unset($query->table_queue['webform_submissions_users']);
  $query->table_queue['webform_submissions_users'] = $webform_table; 
  $query->table_queue['users_webform_submissions'] = $tmp; 
}

function osa_views_query_alter_summer_camp_student_allergies(&$view, &$query) {
  osa_views_query_alter_summer_camp_students($view, $query);
  // move to the end
  $tmp = $query->table_queue['webform_submitted_data_webform_submissions'];
  unset($query->table_queue['webform_submitted_data_webform_submissions']);
  $query->table_queue['webform_submitted_data_webform_submissions'] = $tmp; 
}

/**
 * Implements hook_views_query_alter() to join the civicrm_participant and the civicrm_contribution tables
 */
function osa_views_query_alter_extra_summer_camp_payments(&$view, &$query) {
  // replace the join
  $new_join = new views_join();
  $new_join->construct('civicrm_participant', 'civicrm_contribution', 'contact_id', 'contact_id');
  $query->table_queue['users_civicrm_uf_match__civicrm_participant']['join'] = $new_join;
}