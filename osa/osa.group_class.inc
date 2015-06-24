<?php

/**
 * @file
 * Functions for Group Class functionality
 */
require_once 'osa.inc';

/**
 * Define types of Group Classes - should move to the database
 */
function osa_options_class_types() {

  $options = array();
  // $options['cello'] = 'Cello';
  $options['bass'] = 'Double Bass';
  $options['guitar'] = 'Guitar';
  $options['koday'] = 'Koday Only';
  $options['ecm'] = 'Suzuki Baby';
  $options['theory'] = 'Theory';
  $options['viace'] = 'Violin - Advanced';
  $options['violin'] = 'Violin';

  return $options;
}

function osa_options_class_sessions() {

  $options = array();
  $options[1] = 'Monday';
  $options[2] = 'Tuesday';
  $options[3] = 'Wednesday';
  $options[4] = 'Thursday';
  $options[5] = 'Friday';
  $options[6] = 'Saturday';
  $options[7] = 'Sunday';

  return $options;
}

/**
 * Modify the form the admin uses to create group classes
 */
function osa_form_alter_group_class_node_form(&$form, &$form_state) {
  
  // hide fields that are auto populated
  hide($form['field_event_summary']);
  hide($form['body']);
}

/**
 * Modify the selection options on the group class admin form
 */
function osa_field_widget_options_select_form_alter(&$element, &$form_state, $context) {

  // This code, while it works to populate the selection boxes when built, fails on validation
  // So we have hard coded the values into the form for now.
  /* populate the drop down with the class types
  if ($element['#field_name'] == 'field_class_type') {
    $element['#options'] = array('_none' => '- Select a value -') + osa_options_class_types();
  }
  */
  
  // populate the drop down with the list of events
  // only include events of type 'Group Class (Master)'
  if ($element['#field_name'] == 'field_event_master') {
    civicrm_initialize();
    $results = civicrm_api('event', 'get', array('event_type_id' => OSA_EVENT_GROUP_CLASS_MASTER, 'is_active' => TRUE, 'rowCount' => PHP_INT_MAX, 'version' => 3));
    $options = array();
    foreach ($results['values'] as $class) {
      $options[$class['id']] = $class['event_title'];
    }
    asort($options);
    $element['#options'] = array('_none' => '- Select a value -') + $options;
  }
}

/**
 * Process the saving of the group class definition, and create CiviCRM events for each date
 */
function osa_node_presave_group_class($node) {
  // get the master event
  civicrm_initialize();
  $master_event_id = $node->field_event_master['und'][0]['civicrm_reference_id'];
  $results = civicrm_api('event', 'get', array('id' => $master_event_id, 'version' => 3));
  $master_event = $results['values'][$master_event_id];

  // make a copy to use as a template
  $event = $master_event;
  $event['event_type_id'] = OSA_EVENT_GROUP_CLASS_CHILD;
  $event['is_public'] = TRUE;
  $event['is_online_registration'] = FALSE;
  unset($event['created_id']);
  unset($event['created_date']);
  unset($event['start_date']);
  unset($event['event_start_date']);
  unset($event['end_date']);
  unset($event['event_end_date']);
  $event['version'] = 3;
  
  // get existing child event ids
  $child_event_ids = isset($node->field_event_children['und'][0]['value']) ? json_decode($node->field_event_children['und'][0]['value']) : array();

  // loop through all of the session dates and create/update an event
  $session_dates = &$node->field_session_dates['und'];
  $i = 0;
  foreach ($session_dates as $session_date) {
    if (isset($child_event_ids[$i])) {
      $event['id'] = $child_event_ids[$i];
    }
    else {
      unset($event['id']);
    }
    
    $event['start_date'] = date('Y-m-d H:i:s', strtotime($session_date['value'] . ' ' . $session_date['timezone_db']));
    $event['end_date'] = date('Y-m-d H:i:s', strtotime($session_date['value2'] . ' ' . $session_date['timezone_db']));

    $results = civicrm_api('event', 'create', $event);
    $child_event_ids[$i++] = $results['id'];
  }

  // delete extra events
  if (count($child_event_ids) > count($session_dates)) {
    $lbound = count($session_dates);
    $ubound = count($child_event_ids);
    for ($i = $lbound; $i < $ubound; $i++) {
      $results = civicrm_api('event', 'delete',  array('id' => $child_event_ids[$i], 'version' => 3));
      unset($child_event_ids[$i]);
    }
  }

  // reencode the child event array
  $node->field_event_children['und'][0]['value'] = json_encode($child_event_ids);
  
  // update the master event start and end
  if (count($session_dates) > 0) {
    $last = count($session_dates) - 1;
    $group_start = date('Y-m-d H:i:s', strtotime($session_dates[0]['value'] . ' ' . $session_dates[0]['timezone_db']));
    $group_end = date('Y-m-d H:i:s', strtotime($session_dates[$last]['value2'] . ' ' . $session_dates[$last]['timezone_db']));
    
    $params = array(
      'id' => $master_event_id,
      'version' => 3,
    );
    
    if ($group_start < $master_event['start_date']) {
      $params['start_date'] = $group_start;
    }
    if ($group_end > $master_event['end_date']) {
      $params['end_date'] = $group_end;
    }

    $results = civicrm_api('event', 'create', $params);
  }
}

/**
 * Remove the links to child events when cloning the group class
 */
function osa_clone_node_alter_group_class(&$node, $context) {
  $node->field_event_children['und'][0]['value'] = json_encode(array());
}

/**
 * Modify the webform the end-users use to select a group class
 */
function osa_form_alter_webform_client_form_6(&$form, &$form_state) {

  // set the name and id of the student
  civicrm_initialize();
  $cid = $_GET['cid'];
  $contact = _osa_get_contact($cid, FALSE);
  $form['submitted']['cid']['#value'] = $cid;
  $form['submitted']['display_name']['#markup'] = str_replace('%display_name%', $contact['display_name'], $form['submitted']['display_name']['#markup']);
  
  // get the list of defined group classes (nodes)
  $nodes = &_osa_get_class_nodes($cid);

  // populate the class description, with the event descriptions
  $descs = array();
  foreach ($nodes as $class) {
    $event_id = $class->field_event_master['und'][0]['civicrm_reference_id'];
    $results = civicrm_api('event', 'get', array('id' => $event_id, 'version' => 3));
    $event = $results['values'][$event_id];
    $descs[$event_id] = <<<EOD
<div id=group-class-desc-$event_id class="event-info" >
  <h3 class="crm-section event_title-section">{$event['title']}</h3>
  <div class="crm-section event_summary-section">{$event['summary']}</div>
  <div class="crm-section event_description-section summary">{$event['description']}</div>
</div>
EOD;
  }

  // create the javascript to show/hide the class descriptions
  $form['submitted']['desc_fieldset']['#attributes']['style'] = 'visibility:hidden;';
  $descs_json = drupal_json_encode($descs);
  $js = <<<EOD
function set_class_description() {
  var descs = $descs_json;
  var group_type = jQuery("#edit-submitted-class-type").val();
  var cbox = "#edit-submitted-" + group_type + "-fieldset-" + group_type;
  var event = jQuery( cbox ).val();
  if (descs[event] == undefined) {
    jQuery('.webform-component--desc-fieldset').css('visibility', 'hidden');
  }
  else {
    jQuery('.webform-component--desc-fieldset--class-description').html(descs[event]);
    jQuery('.webform-component--desc-fieldset').css('visibility', 'visible');
  }
}
jQuery(document).ready(function() {
  jQuery("select").change(function(){
    set_class_description();
  });
}); 
EOD;

  drupal_add_js($js, 'inline');

  // set the default values for the group class lists
  $types = osa_options_class_types();
  foreach ($types as $type => $label) {
    if (is_array($form['submitted']["{$type}_fieldset"][$type]['#options'])) {
      reset($form['submitted']["{$type}_fieldset"][$type]['#options']);
      $first_option = key($form['submitted']["{$type}_fieldset"][$type]['#options']);
      $form['submitted']["{$type}_fieldset"][$type]['#default_value'] = isset($first_option) ? $first_option : '';
    }
  }
}

/**
 * Redirect the webform submission to the right Event
 */
function osa_webform_submission_presave_6(&$node, &$submission) {

  // get the contact id
  $component = &_osa_get_webform_component($node, 'cid');
  $cid = $submission->data[$component['cid']][0];
  
  // get the type of group class
  $component = &_osa_get_webform_component($node, 'class_type');
  $class_type = $submission->data[$component['cid']][0];

  // the other components are named after the class type
  $component = &_osa_get_webform_component($node, $class_type);
  $event_id = $submission->data[$component['cid']][0];
  $component = &_osa_get_webform_component($node, "{$class_type}_session");
  $session_id = isset($component) ? $submission->data[$component['cid']][0] : 0;

  // special hard coded process for koday included in violin group
  $component = &_osa_get_webform_component($node, "koday_level");
  $koday_level = $submission->data[$component['cid']][0];
  
  $query_options = array(
    'reset' => '1',
    'id' => $event_id,
    'cid' => $cid,
    'sid' => $session_id,
    'kl' => $koday_level,
  );
  drupal_goto('civicrm/event/register', array('query' => $query_options));
}

/**
 * Define options for the class type drop down on the class selection form
 */
function osa_webform_options_class_types($component, $flat, $arguments) {

  // get the list of group classes and make a list of types
  $cid = $_GET['cid'];
  $nodes = &_osa_get_class_nodes($cid);
  
  $options = array();
  $option_labels = osa_options_class_types();
  foreach ($nodes as $class) {
    $options[$class->field_class_type['und'][0]['value']] = $option_labels[$class->field_class_type['und'][0]['value']];
  }

  asort($options);
  return $options;
}

/**
 * Define options for the group class list (list of master events, which have schedules defined).
 */
function osa_webform_options_class_list($component, $flat, $arguments) {
  // Only include classes whos type is equal to the name (form_key) of the dropdown
  $cid = $_GET['cid'];
  $nodes = &_osa_get_class_nodes($cid, $component['form_key']);
    
  civicrm_initialize();
  $options = array();
  foreach ($nodes as $class) {
    $event_id = $class->field_event_master['und'][0]['civicrm_reference_id'];
    $results = civicrm_api('event', 'get', array('id' => $event_id, 'version' => 3));
    $event = $results['values'][$event_id];
    $options[$event_id] = $event['title'];
  }

  asort($options);
  return $options;
}

/**
 * Define options for the session list (days of the week with schedules defined).
 */
function osa_webform_options_session_list($component, $flat, $arguments) {

  // Only include classes whos type is equal to the name (form_key) of the dropdown
  $cid = $_GET['cid'];
  $nodes = &_osa_get_class_nodes($cid, strstr($component['form_key'], '_', TRUE));

  $options = array();
  $option_labels = osa_options_class_sessions();
  foreach ($nodes as $class) {
    $options[$class->field_session_id['und'][0]['value']] = $option_labels[$class->field_session_id['und'][0]['value']];
  }

  ksort($options);
  return $options;
}

/**
 * Return a list of Group Class nodes
 */
function &_osa_get_class_nodes($cid = NULL, $class_type = NULL, $event_master = NULL, $session_id = NULL) {

  static $class_nodes = array();
  $cache_key = implode('|', array($cid, $class_type, $event_master, $session_id));

  if (!array_key_exists($cache_key, $class_nodes)) {
    // get the list of group class nodes
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'node')
                            ->entityCondition('bundle', 'group_class')
                            ->propertyCondition('status', 1);

    // filter based on type
    if (isset($class_type)) {
      $query->fieldCondition('field_class_type', 'value', $class_type, '=');
    }

    // most group classes require membership so filter if the cid is past
    if (isset($cid)) {
      civicrm_initialize();
      $membership = _osa_get_membership($cid);
      $is_member = isset($membership['is_current_member']) ? $membership['is_current_member'] : FALSE;
      if (!$is_member) {
        $query->fieldCondition('field_req_membership', 'value', FALSE, '=');
      }
    }

    // filter based on master event
    if (isset($event_master)) {
      $query->fieldCondition('field_event_master', 'civicrm_reference_id', $event_master, '=');
    }

    // filter based on session (day of week)
    if (isset($session_id)) {
      $query->fieldCondition('field_session_id', 'value', $session_id, '=');
    }

    $entities = $query->execute();
    $class_nodes[$cache_key] = empty($entities) ? array() : node_load_multiple(array_keys($entities['node']));
  }

  return $class_nodes[$cache_key];
}
