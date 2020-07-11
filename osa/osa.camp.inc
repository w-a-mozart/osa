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
      'One Week' => 85.00,
      'Both Weeks' => 150.00,
    ),
    'Junior Program' => array(
      'One Week' => 250.00,
      'Both Weeks' => 480.00,
    ),
    'Intermediate Program' => array(
      'One Week' => 295.00,
      'Both Weeks' => 570.00,
    ),
    'Full Day Supervised' => array(
      'One Week' => 415.00,
      'Both Weeks' => 810.00,
    ),
    'Senior Program' => array(
      'One Week' => 345.00,
      'Both Weeks' => 670.00,
    ),
    'Leader In Training' => array(
      'One Week' => 190.00,
      'Both Weeks' => 360.00,
    ),
    'Student Leader' => array(
      'One Week' => 175.00,
      'Both Weeks' => 330.00,
    ),
    'Student Leader Guitar and Piano' => array(
      'One Week' => 110.00,
      'Both Weeks' => 200.00,
    ),
    'Chamber Only' => array(  /* not used in online form */
      'One Week' => 180.00,
      'Both Weeks' => 320.00,
    ),
    'primary_instrument_enhancements' => array(
      'Additional 15min Private Lesson' => 70.00,
    ),
    'primary_instrument_enhancements_leaders' => array(
      '15min Private Lesson' => 70.00,
      '30min Private Lesson' => 140.00,
    ),
    'second_instrument_enhancements' => array(
      'Second Instrument 15min Private Lesson' => 70.00,
      'Second Instrument 30min Private Lesson' => 140.00,
    ),
    'chamber_group' => array(
      'Junior Chamber Group' => 40.00,
      'Senior Chamber Group' => 50.00,
    ),
    'masterclass_terry_durbin' => array(
      'Violin Ensemble' => 60.00, // Intermediate & Advanced Violin Ensemble
    ),
    'daycare_options' => array(
      'AM Care' => 20.00,
      'PM Care' => 20.00,
    ),
    'lunch_option' => array( /* no longer used */
      'Student Lunch' => 37.00,
      'Student Lunch Both Weeks' => 70.00,
      'Parent Lunch' => 37.00,
      'Parent Lunch Both Weeks' => 70.00,
    ),
    'teacher' => array(
      'Terry Durbin: 15min lessons' => 80.00,
      'Terry Durbin: 30min lessons' => 160.00,
      'TJ McAvaney: 15min lessons' => 75.00,
      'TJ McAvaney: 30min lessons' => 150.00,
    ),
  );

  // fiddle with the names / values
  if ($name == 'weeks') {
    $name = is_array($other_values['programs']['value']) ? $other_values['programs']['value'][0]: $other_values['programs']['value'];
    if ($value != 'Both Weeks') {
      $value = 'One Week';
    }
  }
  
  if (strpos($name, '_teacher_') !== FALSE) {
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

  // figure out if this is for the primary or secondary instrument, and the week
  list($instrument, $ignore, $week, $ignore2) = explode('_', $form_key, 4);
  $instrument = ($instrument == 'secondary') ? 'second' : $instrument;
  $wktxt = str_replace('wk', 'Week ', $week);
  $wknum = str_replace('wk', '', $week);

  // get the program to determine if the student is a Leader
  $program = is_array($submitted[4]) ? $submitted[4][0] : $submitted[4];
  $leader = (strpos($program, 'Leader') !== FALSE) || ($program == 'Chamber Only');
  
  // determine the lesson length
  $lesson_length = 0;

  if (($instrument == 'primary') && !$leader) {
    // 15min is included in primary instrument (except for Leaders)
    $lesson_length += 15;
  }
  
  // add enhancements
  $leaders = ($leader && ($instrument == 'primary')) ? 'leaders_' : '';
  $keyname = "{$instrument}_instrument_enhancements_{$leaders}wk{$wknum}";
  $cid = _osa_get_webform_component_id($components, $keyname);

  $enh_str = '';
  if (isset($submitted[$cid])) {
    $enh_str = is_array($submitted[$cid]) ? $submitted[$cid][0] : $submitted[$cid];
  }

  $tmp = array();
  $enh_min = (preg_match( '/[0-9]+min/', $enh_str, $tmp) === 1) ? intval(substr($tmp[0], 0, strlen($tmp[0]) - 3)) : 0;
  $lesson_length += $enh_min;
  
  // quit if no premium minutes
  if ($lesson_length == 0) {
    return FALSE;
  }

  // Hack the component values to display the premium teacher
  $component['name'] = "Teacher Premium";
  $value = "{$teacher}: {$lesson_length}min lessons in Week {$wknum}";

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
      $form['#attached']['js'][] = array(
        'data' => "jQuery(document).ready(function () {jQuery(\"[required='required']\").removeAttr('required');});",
        'type' => 'inline');
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
  $options_grid = array();
  
  if ($programs == 'Senior Program') {
    $component = &_osa_get_webform_component($node, 'options_senior_program');
    $options_grid = $submission->data[$component['cid']];
  }
  elseif ($programs == 'Intermediate Program') {
    $component = &_osa_get_webform_component($node, 'options_intermediate_program');
    $options_grid = $submission->data[$component['cid']];
  }
  
  if ((isset($options_grid['Recorder'])) && ($options_grid['Recorder'] != 'NT')) {
    $is_set['recorder_level'] = TRUE;
  }
  if ((isset($options_grid['RCM Theory'])) && ($options_grid['RCM Theory'] != 'NT')) {
    $is_set['theory_level'] = TRUE;
  }
  if ((isset($options_grid['Ukulele'])) && ($options_grid['Ukulele'] != 'NT')) {
    $is_set['ukulele_level'] = TRUE;
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

  // create a pending participant record
  $contact_id = $values['camp_participant']['value'];
  $event_id   = $values['event_id']['value'];
  
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
    drupal_set_message('An unexpected error has occurred. Please contact the administrator', 'error');
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
    drupal_set_message('An unexpected error has occurred. Please contact the administrator', 'error');
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
    drupal_set_message('An unexpected error has occurred. Please contact the administrator', 'error');
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
  $ignore_elements = array('caregiver_present', 'primary_instrument_level_wk1', 'add_a_second_instrument_wk1', 'second_instrument_level_wk1',  'primary_instrument_level_wk2', 'add_a_second_instrument_wk2', 'second_instrument_level_wk2', 'ukulele_level', 'recorder_level', 'theory_level', 'musical_theatre', 'image_release_consent', 'first', 'referral',);
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
        if (strpos($form_key, '_teacher_') !== FALSE) {
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
        if ($form_key != 'event_id')
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

  if ($form_key == 'camp_participant') {
    civicrm_initialize();
    $contact = _osa_get_contact($value['value'], FALSE);
    $value['value'] = $contact['display_name'];
  }

  $rows = array();
  
  if (!empty($value['price']) && $value['price'] > 0.001) {
    $rows[] = array(
      'data' => array(
        array(
          'data' => is_array($value['value']) ? implode(', and ', $value['value']) : $value['value'],
          'class' => array('osa-camp-item-label'),
        ),
        array(
          'data' => '$ ' . number_format($value['price'], 2),
          'class' => array('osa-camp-item-price'),
          'style' => array('text-align: right;', 'vertical-align: top;', 'white-space: nowrap;'),
        ),
      ),
      'class' => array('osa-camp-item-content', "osa-camp-$class"),
    );
  } else {
    $rows[] = array(
      'data' => array(
        array(
          'data' => $component['name'],
          'class' => array('osa-camp-item-label'),
        ),
        array(
          'data' => is_array($value['value']) ? implode(', and ', $value['value']) : $value['value'],
          'class' => array('osa-camp-item-label'),
        ),
      ),
      'class' => array('osa-camp-item-title', "osa-camp-$class"),
    );
  }

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
