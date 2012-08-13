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
  $options['violin'] = 'Violin';
  $options['viace_violin'] = 'Viace Violin';
  $options['cello'] = 'Cello';
  $options['koday'] = 'Koday Only';
  $options['theory'] = 'Theory';
  $options['ecm'] = 'Suzuki Baby';

  return $options;
}

/**
 * Modify the form the admin uses to create group classes
 */
function osa_form_alter_group_class_node_form(&$form, &$form_state) {
  
  // hide fields that are auto populated
  hide($form['field_event_summary']);
  hide($form['body']);
  hide($form['field_event_children']);

}

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
    $results = civicrm_api('event', 'get', array('event_type_id' => OSA_EVENT_GROUP_CLASS_MASTER, 'version' => 3));
    $options = array();
    foreach($results['values'] as $class) {
      $options[$class['id']] = $class['event_title'];
    }
    $element['#options'] = array('_none' => '- Select a value -') + $options;
  }
}

/**
 * Process the saving of the group class, and create CiviCRM events for each date
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
  $child_event_ids = &$node->field_event_children['und'];

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
  
  // update the master event start and end
  if (count($session_dates) > 0) {
    $last = count($session_dates) - 1;
    $results = civicrm_api('event', 'create', array(
      'id' => $master_event_id,
      'start_date' => date('Y-m-d H:i:s', strtotime($session_dates[0]['value'] . ' ' . $session_dates[0]['timezone_db'])),
      'end_date' => date('Y-m-d H:i:s', strtotime($session_dates[$last]['value2'] . ' ' . $session_dates[$last]['timezone_db'])),
      'version' => 3,
    ));
  }
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
  $nodes = &_osa_get_class_nodes();

  // populate the class description, with the event descriptions
  $descs = array();
  foreach($nodes as $class) {
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
    document.getElementById('webform-component-desc-fieldset').style.visibility='hidden';
  }
  else {
    document.getElementById('webform-component-desc-fieldset--class-description').innerHTML = descs[event];
    document.getElementById('webform-component-desc-fieldset').style.visibility='visible';
  }
}
jQuery(document).ready(function() {
  jQuery("select").change(function(){
    set_class_description();
  });
}); 
EOD;

  drupal_add_js($js, 'inline');
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
 * Define types of options lists used in drop downs on the webform
 */
function osa_webform_select_options_info() {
  $items = array();

  $items['class_type'] = array(
    'title' => t('Group Class Types'), 
    'options callback' => 'osa_webform_options_class_types', 
    'file' => 'osa.group_class.inc',
  );

  $items['class_list'] = array(
    'title' => t('Group Class'), 
    'options callback' => 'osa_webform_options_class_list', 
    'file' => 'osa.group_class.inc',
  );

  return $items;
}

/**
 * Define options for the class type drop down
 */
function osa_webform_options_class_types($component, $flat, $filter, $arguments) {

  // get the list of group classes and make a list of types
  $nodes = &_osa_get_class_nodes();
  
  $options = array();
  $option_labels = osa_options_class_types();
  foreach($nodes as $class) {
    $options[$class->field_class_type['und'][0]['value']] = $option_labels[$class->field_class_type['und'][0]['value']];
  }

  return $options;
}

/**
 * Define options for the group class list.
 */
function osa_webform_options_class_list($component, $flat, $filter, $arguments) {

  // Only include classes whos type is equal to the name (form_key) of the dropdown
  $nodes = &_osa_get_class_nodes($component['form_key']);
    
  civicrm_initialize();
  $options = array();
  foreach($nodes as $class) {
    $event_id = $class->field_event_master['und'][0]['civicrm_reference_id'];
    $results = civicrm_api('event', 'get', array('id' => $event_id, 'version' => 3));
    $event = $results['values'][$event_id];
    $options[$event_id] = $event['title'];
  }

  return $options;
}

/**
 * Return a list of Group Class nodes
 */
function &_osa_get_class_nodes($class_type = NULL) {

  static $class_nodes = array();

  $cache_key = isset($class_type) ? $class_type : '*';
  
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
    
    // most group classes require membership
    civicrm_initialize();
    $cid = $_GET['cid'];
    $membership = _osa_get_membership($cid);
    $is_member = isset($membership['is_current_member']) ? $membership['is_current_member'] : FALSE;
    if (!$is_member) {
      $query->fieldCondition('field_req_membership', 'value', FALSE, '=');
    }
    
    $entities = $query->execute();
    $class_nodes[$cache_key] = empty($entities) ? array() : node_load_multiple(array_keys($entities['node']));
  }
  
  return $class_nodes[$cache_key];
}

/**
 * No longer used.
 */
function &_osa_get_webform_component(&$node, $form_key) {

  static $webform_cache = array();

  if (!array_key_exists($node->nid, $webform_cache)) {
    $webform_cache[$node->nid] = array();
    foreach ($node->webform['components'] as $id => $component) {
      $webform_cache[$node->nid][$component['form_key']] = $id;
    }
  }

  if (array_key_exists($form_key, $webform_cache[$node->nid])) {
    return $node->webform['components'][$webform_cache[$node->nid][$form_key]];
  }
  else {
    return NULL;
  }
}
