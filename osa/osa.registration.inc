<?php
/**
 * @file
 * Functions for Yearly Registration
 */
require_once 'osa.inc';

/**
 * Set the price of group classes
 * @TODO get the from DB
 */
function osa_get_groupclass_price_table() {
  $priceTable = [
    /* ecm price per term */
    'ECM_Term_1' => 175,
    'ECM_Term_2' => 145,
    'ECM_Term_3' => 110,
    'ECM_Max' => 380,  /* full year */

    /* violin group class price (includes 30 min Kodaly)*/
    'Violin_Group_Parent_Training' => 360,
    'Violin_Group_30_min' => 340,
    'Violin_Group_45_min' => 415,
    'Violin_Group_60_min' => 450,
    /* kodaly/theory price for violin group students */
    'Violin_Group_Kodaly_30_min' => 0, /* $155 */
    'Violin_Group_Kodaly_45_min' => 40,
    'Violin_Group_RCM' => 250,
    'Violin_Group_Reading' => 10, 
    /* note these look high because only one class is reduced by $155 */
    'Violin_Group_Reading_Kodaly_45_min' => 155,
    'Violin_Group_Reading_RCM' => 315,

    'Preludio' => 450,
    'Preludio_RCM' => 310,

    'Vivace' => 450,
    'Vivace_RCM' => 310,

    /* cello group class price (includes Kodaly) */
    'Cello_Group_45_min' => 400,
    'Cello_Group_60_min' => 450,
    /* kodaly/theory price for cello group students */
    'Cello_Group_Kodaly_30_min' => 0,
    'Cello_Group_Kodaly_45_min' => 0,
    'Cello_Group_RCM' => 305,

    'Chamber' => 'tbd',
    'Chamber_RCM' => 310,

    /* kodaly/theory only prices */
    'Kodaly_Theory_Kodaly_30_min' => 180,
    'Kodaly_Theory_Kodaly_45_min' => 240,
    'Kodaly_Theory_RCM' => 415,
  ];
  return $priceTable;
}

/**
 * Set the event id of group classes
 * @TODO get the events from DB
 */
function osa_get_groupclass_eventIds() {
  $eventIdTable = [
    'ECM' => 3358,
    'Violin_Group' => 3362,
//    'Preludio' => 3033, // need to remove
//    'Vivace' => 3034, // need to remove
    'Cello_Group' => 3363,
    'Chamber' => 3359,
    'Kodaly_Theory' => 3360,
  ];
  return $eventIdTable;
}

/**
 * Set the credits_required for group classes
 */
function osa_get_groupclass_credits_required() {
  $creditsRequiredTable = [
    'ECM' => 0,
    'Violin Group' => [
      'Beginner'     => 2,
      'Early Book 1' => 3,
      '30'           => 8,
      '45'           => 8,
      '60'           => 8,
    ],
    'Cello Group' => [
      'Beginner'     => 2,
      '45'           => 8,
      '60'           => 8,
    ],
    'Preludio'       => 8,
    'Vivace'         => 8,
    'Chamber'        => 0, // change to 4, when pricing is available
    'Kodaly-Theory'  => 0,
  ];
  return $creditsRequiredTable;
}

/**
 * Set the credits_required for group classes
 */
function _osa_participant_key_map() {
  $participantKeyMap = [
    'Early' => 'ECM',
    'Violin' => 'Violin Group',
    'Cello' => 'Cello Group',
    'Preludio' => 'Preludio',
    'Vivace' => 'Vivace',
    'Chamber' => 'Chamber',
    'Kodaly' => 'Kodaly-Theory',
  ];
  return $participantKeyMap;
}

/**
 * Get the volunteer credits required for this family
 * (including what's in the registration webform)
 */
function osa_get_volunteer_credits_required($hid, $submission = null) {
  // in case this is called multiple times in one request
  static $creditsRequiredCache = [];
  if (!isset($hid))
    $hid = 0;
  if (isset($submission)) {
    $nid = $submission->nid;
    $sid = $submission->sid;
  } else {
    $nid = 0;
    $sid = 0;
  }
  $cache_key = "$hid:$nid:$sid";
  if (isset($creditsRequiredCache[$cache_key]))
    return $creditsRequiredCache[$cache_key];

  $credits_required = 0;
  $max_credits_required = 8;
  $creditsRequiredTable = osa_get_groupclass_credits_required();

  // get new registrations from webform
  if (isset($submission)) {
    $node = node_load($nid);
    $submission_data = _osa_parse_webform_submission_data($node->webform['components'], $submission->data);

    for ($idx =1; $idx <= $submission_data['number_of_students']; $idx++) {
      $group_class = $submission_data["student_{$idx}_group_class"];
      $tmp_credits = isset($creditsRequiredTable[$group_class]) ? $creditsRequiredTable[$group_class] : 0;

      if (is_array($tmp_credits)) {
        $group_level = '';
        $level_fields = [ "student_{$idx}_violin_group_saturday", "student_{$idx}_violin_group_thursday", "student_{$idx}_cello_group" ];
        foreach ($level_fields as $level_field) {
          if (!empty($submission_data[$level_field])) {
            $group_level = $submission_data[$level_field];
            break;
          }
        }
        if (strpos($group_level, 'Beginner') !== FALSE) {
          $group_level = 'Beginner';
        } elseif (strpos($group_level, 'Early Book 1') !== FALSE) {
          $group_level = 'Early Book 1';
        } else {
          if (preg_match('/\((.*) min/', $group_level, $minutes)) {
            $group_level = $minutes[1];
          }
        }
        $tmp_credits = isset($tmp_credits[$group_level]) ? $creditsRequiredTable[$group_class][$group_level] : 0;
      }

      $credits_required = max($credits_required, $tmp_credits);
      if ($credits_required >= $max_credits_required) {
        break;
      }
    }
  }

  // check current registrations (if not already at max)
  if ($credits_required < $max_credits_required) {
    $familyIds = _osa_getHouseholdMembers($hid, 'id');
    $groupclass_eventIds = osa_get_groupclass_eventIds();
    $result = civicrm_api3('Participant', 'get', [
      'contact_id' => ['IN' => $familyIds],
      'event_id' => ['IN' => $groupclass_eventIds],
      'status_id' => ['IN' => ['Registered', 'Pending from pay later', 'Pending from incomplete transaction', 'On waitlist', 'Partially paid', 'Pending refund']],
    ]);

    if ($result['count'] > 0) {
      $key_map = _osa_participant_key_map();

      foreach ($result['values'] as $pid => $participant) {
        $tmp_credits = 0;
        if (preg_match('/ (.*?) /', $participant['event_title'], $matches)) {
          $title = $matches[1];
          $key = (!empty($key_map[$title])) ? $key_map[$title] : '';
          $tmp_credits = isset($creditsRequiredTable[$key]) ? $creditsRequiredTable[$key] : 0;
          if (is_array($tmp_credits)) {
            $group_level = is_array($participant['participant_fee_level']) ? implode("\n", $participant['participant_fee_level']) : $participant['participant_fee_level'];
            if (strpos($group_level, 'Beginner') !== FALSE) {
              $group_level = 'Beginner';
            } elseif (strpos($group_level, 'Early Book 1') !== FALSE) {
              $group_level = 'Early Book 1';
            } else {
              if (preg_match('/\((.*) min/', $group_level, $minutes)) {
                $group_level = $minutes[1];
              }
            }
            $tmp_credits = isset($tmp_credits[$group_level]) ? $tmp_credits[$group_level] : 0;
          }
        }

        $credits_required = max($credits_required, $tmp_credits);
        if ($credits_required >= $max_credits_required) {
          break;
        }
      }
    }
  }

  $creditsRequired[$cache_key] = min($credits_required, $max_credits_required);
  return $creditsRequired[$cache_key];
}

/**
 * Get the volunteer credits this family has already committed
 * (not including what's in the registration volunteer 'shopping cart')
 */
function osa_get_volunteer_credits_committed($hid) {
  static $creditsCommitted = [0 => 0];
  if (!isset($hid))
    $hid = 0;

  if (isset($creditsCommitted[$hid]))
    return $creditsCommitted[$hid];

  $assignments = osa_get_volunteer_assignments($hid);
  $customFieldDefs = CRM_Volunteer_BAO_Assignment::getCustomFields();
  $creditField = $customFieldDefs['credits']['custom_n'];
  $creditsCommitted[$hid] = array_sum(array_column($assignments, $creditField));

  return $creditsCommitted[$hid];
}

/**
 * Determine if this family has already paid in lieu
 */
function osa_get_volunteer_paidInLieu($hid) {
  static $paidInLieuCache = [0 => FALSE];
  if (!isset($hid))
    $hid = 0;

  if (isset($paidInLieuCache[$hid]))
    return $paidInLieuCache[$hid];

  $schoolYear = osa_get_school_year();
  $familyIds = _osa_getHouseholdMembers($hid, 'id');
  $familyIds[] = $hid;

  $result = civicrm_api3('Contribution', 'get', [
    'financial_type_id' => 'Donation',
    'contact_id' => ['IN' => $familyIds],
    'source' => ['LIKE' => "%Volunteering {$schoolYear}%"],
    'total_amount' => ['>=' => 100],
    'contribution_status_id' => ['IN' => ["Completed", "Pending"]],
  ]);

  $paidInLieuCache[$hid] = ($result['count'] > 0);
  return $paidInLieuCache[$hid];
}


/**
 * Modify the registration application webform
 */
function osa_form_alter_registration(&$form, &$form_state) {
  // add a validation routine so we get called even when there is an error
  $form['#validate'][] = 'osa_registration_validate';

  // modify the progress bar to only show 1 student page (aka Choose Group Class)
  $page_num = $form_state['webform']['page_num'];
  $form['progressbar']['#page_labels'] = [
    'Start',
    'Select Students',
    'Choose Group Class',
    'Select Volunteer Duties',
    'Complete',
  ];
  $form['progressbar']['#page_count'] = 5;
  $form['progressbar']['#page_num']   = $page_num - min(max($page_num - 3, 0), 3);


  // nothing submitted yet, so nothing else to do
  if (empty($form['#submission'])) {
    return;
  }

  // enable CiviCRM in the webform
  civicrm_initialize();
  $manager = CRM_Core_Resources::singleton();
  $manager->addCoreResources();
  // add these as well since they are only added by default for users with 'access CiviCRM' permission
  $manager->addScriptFile('civicrm', 'packages/jquery/plugins/jquery.tableHeader.js', -9910, 'html-header');
  $manager->addScriptFile('civicrm', 'packages/jquery/plugins/jquery.notify.min.js', -9909, 'html-header');

  /* set vars to be used in javascript */ 
  $vars = [
    'priceTable'           => osa_get_groupclass_price_table(),
    'eventIdTable'         => osa_get_groupclass_eventIds(),
    'schoolDates'          => osa_get_school_dates(),
  ];
  $manager->addVars('osa', $vars);

  // add custom JS/CSS to form
  $osaPath = drupal_get_path('module', 'osa');
  $form['#attached']['css'][] = $osaPath . '/osa.registration.css';
  $form['#attached']['js'][] = $osaPath . '/osa.registration.js';
  $form['#attached']['js'][] = [
    'type' => 'inline',
    'data' => 'var priceTable = CRM.vars.osa.priceTable;
               var eventIdTable = CRM.vars.osa.eventIdTable;
               var schoolDates = CRM.vars.osa.schoolDates;',
  ];

  // get a short cut to the webform components
  $components = $form['#node']->webform['components'];

  // set the form_state to indicate if we need to refresh the student info from civicrm
  // it can change whenever the user selects students (ie submits page 2), or if a partially-completed form was loaded
  if ((!$form_state['osa']) || (($page_num == 2) && ($form_state['input']['details']['page_num'] == $page_num))) {
    // assume yes for all
    $form_state['osa']['reset_contact'] = array_fill(1, 4, TRUE);

    // if the user just selected/re-selected students
    if (!empty($form_state['values']['submitted']['students']))
      $students = array_values(array_filter($form_state['values']['submitted']['students']));
    // a partially-completed form was loaded
    else
      $students = $form_state['values']['submitted'][_osa_get_webform_component_id($components, 'students')];
    
    // we don't need to reload, if the student matches, and has been previously submitted
    $idx = 1;
    while (($civicrm_comp_id = _osa_get_webform_component_id($components, "civicrm_{$idx}_contact_1_contact_existing")) !== FALSE) {
      $student_id = (!empty($students[$idx-1])) ? $students[$idx-1] : 0;
      $saved_id = (!empty($form_state['storage']['submitted'][$civicrm_comp_id])) ? $form_state['storage']['submitted'][$civicrm_comp_id] : 0;
      $was_submitted = !empty($form_state['storage']['submitted'][_osa_get_webform_component_id($components, "civicrm_{$idx}_contact_1_gender_id")]);

      if ($student_id != $saved_id) {
        $form_state['storage']['submitted'][$civicrm_comp_id] = $student_id;
        $form_state['values']['submitted'][$civicrm_comp_id]  = $student_id;
        $form_state['civicrm']['ent']['contact'][$idx]['id']  = 0;
      } else {
        if ($was_submitted)
          $form_state['osa']['reset_contact'][$idx] = FALSE;
      }
      $idx++;
    }
  }

  // use javascript to set/reset the contact info from civicrm
  if (($page_num >= 3) && ($page_num <= 6) && ($form_state['input']['details']['page_num'] != $page_num)) {
    $idx = $page_num - 2;
    if ($form_state['osa']['reset_contact'][$idx]) {
      drupal_add_js('jQuery(document).ready(function () { jQuery("[name*=\'_contact_existing]\']").trigger(\'change\'); });', 'inline');
      $form_state['osa']['reset_contact'][$idx] = FALSE;
    }
  }
}

/**
 * Load CiviVolunteer in the webform
 *
 * This function is called from the PHP mark-up in the volunteer_duties_form component
 */
function osa_volunteer_duties_form($uid, $nid, $sid) {

  // load the CiviVolunteer Angular app in the webform
  civicrm_initialize();
  $loader = new \Civi\Angular\AngularLoader();
  $loader->setModules(['volunteer']);
  $loader->load();
  $manager = CRM_Core_Resources::singleton();
  $manager->addSetting(['crmApp' => ['defaultRoute' => '/volunteer/opportunities',]]);

  // get the data previously submitted
  module_load_include('inc', 'webform', 'includes/webform.submissions');
  $submission = webform_get_submission($nid, $sid);
  $node = node_load($nid);
  $volunteer_duties = _osa_get_webform_component_id($node->webform['components'], 'volunteer_duties');

  /* set vars to be used in javascript */ 
  $cid = _osa_get_contact_id($uid);
  $hid = _osa_getHousehold($cid);
  $vars = [
    'totalCreditsRequired' => osa_get_volunteer_credits_required($hid, $submission),
    'creditsPrevSelected'  => osa_get_volunteer_credits_committed($hid),
    'needsFromSavedForm'   => (!empty($submission->data[$volunteer_duties][0])) ? $submission->data[$volunteer_duties][0] : '',
    'paidInLieu'           => osa_get_volunteer_paidInLieu($hid),
  ];
  $manager->addVars('osa', $vars);
}

/**
 * Process the form after it is submitted
 * We need to:
 * - create the membership and contribution record in civicrm
 * - create the participant and contribution record in civicrm for group class
 * - create the participant and contribution record in civicrm for kodaly/theory class
 * - add a line items to the shopping cart
 * - process volunteer sign ups
 */
function osa_webform_submission_update_registration($node, $submission) {
  // ignore the saving of drafts
  if ($submission->is_draft)
    return;

  civicrm_initialize();
  $submission_data = _osa_parse_webform_submission_data($node->webform['components'], $submission->data);
  $schoolYear      = osa_get_school_year();
  $schoolDates     = osa_get_school_dates();
  $membershipPrice = 95.00;
  $eventIdTable    = osa_get_groupclass_eventIds();
  $priceTable      = osa_get_groupclass_price_table();
  $customFieldMap  = osa_custom_field_map('Group_Class_Details');

  $membershipParms = ['status_id' => "Pending", 'skipStatusCal' => TRUE, 'source' => 'Online Registration', 'start_date' => $schoolDates['start']->format('Y-m-d'), 'end_date' => $schoolDates['end']->format('Y-m-d'),];
  $contributionParms = ['contribution_status_id' => 'Pending', 'payment_instrument_id' => 'Credit Card', 'source' => 'Online Registration', 'receive_date' => date('Y-m-d H:i:s')];
  $participantParms = ['status_id' => 'Pending from incomplete transaction', 'source' => 'Online Registration', 'role_id' => 'Attendee', 'register_date' => date('Y-m-d H:i:s')];
  $errMsg = 'An error has occurred processing your registration.  Please contact the <a href="mailto:registrar@oakvillesuzuki.org">Registrar</a> to complete the registration process.';

  // process each student
  for ($idx =1; $idx <= $submission_data['number_of_students']; $idx++) {
    // get the data from the webform
    $contact_id = $submission_data["civicrm_{$idx}_contact_1_contact_existing"];
    $contact = _osa_get_contact($contact_id, FALSE);
    $group_class = $submission_data["student_{$idx}_group_class"];
    $group_level = '';
    $level_fields = ["student_{$idx}_violin_group_saturday", "student_{$idx}_violin_group_thursday", "student_{$idx}_cello_group"];
    foreach ($level_fields as $level_field) {
      if (!empty($submission_data[$level_field])) {
        $group_level = $submission_data[$level_field];
        break;
      }
    }
    $theory_level = '';
    $level_fields = ["student_{$idx}_kodaly_theory_option_saturday", "student_{$idx}_kodaly_theory_option_thursday", "student_{$idx}_kodaly_theory_option_advanced"];
    foreach ($level_fields as $level_field) {
      if (!empty($submission_data[$level_field])) {
        $theory_level = $submission_data[$level_field];
        break;
      }
    }
    $theory_bundle = !empty($submission_data["student_{$idx}_kodaly_theory_option_bundle"]) ? $submission_data["student_{$idx}_kodaly_theory_option_bundle"] : '';

    // process membership
    // Note: ECM students are counted as members but don't have a membership record
    if ($group_class != 'ECM') {
      $membership = _osa_get_membership($contact_id);
      if (empty($membership)) {
        $membership = ['membership_type_id' => 1, 'contact_id' => $contact_id, 'school_year' => ''];
      }
      // update membership, if existing is not for the current year
      if ($membership['school_year'] != $schoolYear) {
        $result = civicrm_api3('Membership', 'create', array_merge($membership, $membershipParms));
        if ($result['is_error'])
          throw new CRM_Core_Exception($errMsg, 'RE0001', $result);
        else
          $membership = $result['values'][$result['id']] + ['school_year' => $schoolYear];
        
        // hack - don't know why but start_date isn't correct
        $result = civicrm_api3('Membership', 'create', ['id' => $membership['id'], 'start_date' => $schoolDates['start']->format('Y-m-d'),]);
        $membership['start_date'] = $schoolDates['start']->format('Y-m-d');
        
        $contribution = ['contact_id' => $contact_id, 'financial_type_id' => 'Member Dues', 'total_amount' => $membershipPrice, 'net_amount' => $membershipPrice,];
        $result = civicrm_api3('Contribution', 'create', array_merge($contribution, $contributionParms));
        if ($result['is_error'])
          throw new CRM_Core_Exception($errMsg, 'RE0002', $result);
        else
          $contribution = $result['values'][$result['id']];
        $result = civicrm_api3('MembershipPayment', 'create', ['contribution_id' => $contribution['id'], 'membership_id' => $membership['id'],]);
        if ($result['is_error'])
          throw new CRM_Core_Exception($errMsg, 'RE0003', $result);
        if (osa_add_to_commerce_cart('Membership', $membership, $contact, $contribution) === FALSE)
          throw new CRM_Core_Exception($errMsg, 'RE0004', array_merge($membership, $contribution));
      }
    }

    // process group class
    $event_id = 0;
    $eventPrice = 0.00;
    $price_level = [];
    $add_participant = [];
    if (!empty($submission_data["student_{$idx}_preferred_day"]))
      $add_participant[$customFieldMap['preferred_group_day']] = $submission_data["student_{$idx}_preferred_day"];
    if (!empty($submission_data["student_{$idx}_ecm_saturday_time"]))
      $add_participant[$customFieldMap['additional_data']] = 'Preferred Time : ' . $submission_data["student_{$idx}_ecm_saturday_time"];
    if (!empty($submission_data["student_{$idx}_more_info"]))
      $add_participant[$customFieldMap['more_information']] = $submission_data["student_{$idx}_more_info"];

    if ($group_class == 'ECM') {
      $event_id = $eventIdTable[$group_class];
      $price_level = [$submission_data["student_{$idx}_ecm_level"]];
      foreach ($submission_data["student_{$idx}_ecm_term"] as $ecm_term) {
        $price_key = $group_class . '_' . str_replace(' ', '_', $ecm_term);
        $eventPrice += is_numeric($priceTable[$price_key]) ? $priceTable[$price_key] : 0;
        $price_level[] = $ecm_term;
      }
      $eventPrice = min($eventPrice, $priceTable['ECM_Max']);
      
    } elseif ($group_class != 'Kodaly-Theory') {
      $key = str_replace([' ', '-'], '_', $group_class);
      $event_id = $eventIdTable[$key];
      $tmp_price_key = [$key];
      if (!empty($group_level)) {
        $price_level[] = $group_level;
        $tmp_price_key[] = _osa_registration_price_key_component($group_level);
      }
      $price_key = implode('_', $tmp_price_key);
      $eventPrice = is_numeric($priceTable[$price_key]) ? $priceTable[$price_key] : 0;
    }
    if ($event_id) {
      $fee_level = implode(', ', $price_level);
      $participant = ['event_id' => $event_id, 'contact_id' => $contact_id, 'participant_fee_level' => $fee_level, 'participant_fee_amount' => $eventPrice];
      $result = civicrm_api3('Participant', 'create', array_merge($participant, $participantParms, $add_participant));
      if ($result['is_error'])
        throw new CRM_Core_Exception($errMsg, 'RE0005', $result);
      else
        $participant = $result['values'][$result['id']];
      $contribution = ['contact_id' => $contact_id, 'financial_type_id' => 'Group Class Fee', 'amount_level' => $fee_level, 'total_amount' => $eventPrice, 'net_amount' => $eventPrice,];
      $result = civicrm_api3('Contribution', 'create', array_merge($contribution, $contributionParms));
      if ($result['is_error'])
        throw new CRM_Core_Exception($errMsg, 'RE0006', $result);
      else
        $contribution = $result['values'][$result['id']];
      $result = civicrm_api3('ParticipantPayment', 'create', ['contribution_id' => $contribution['id'], 'participant_id' => $participant['id'],]);
      if ($result['is_error'])
        throw new CRM_Core_Exception($errMsg, 'RE0007', $result);

      // *** Don't add Chamber to Shopping Cart until we have a price
      if ($group_class != 'Chamber') {
        if (osa_add_to_commerce_cart('Event', $participant, $contact, $contribution) === FALSE)
          throw new CRM_Core_Exception($errMsg, 'RE0008', array_merge($participant, $contribution));
      } else {
        civicrm_api3('Participant', 'create', ['id' => $participant['id'], 'status_id' => "Pending from pay later",]);
      }
    }

    // process kodaly/theory class
    if (!empty($theory_level)) {
      $event_id = $eventIdTable['Kodaly_Theory'];
      $price_level = [$theory_level];
      $tmp_price_key = [str_replace([' ', '-'], '_', $group_class)];
      if (($tmp_price_key[] = _osa_registration_price_key_component($theory_level)) == 'Reading') {
        if (!empty($theory_bundle)) {
          $price_level[] = $theory_bundle;
          $tmp_price_key[] = _osa_registration_price_key_component($theory_bundle);
        }
      }
      $price_key = implode('_', $tmp_price_key);
      $eventPrice = is_numeric($priceTable[$price_key]) ? $priceTable[$price_key] : 0;
      $fee_level = implode(', ', $price_level);
      $participant = ['event_id' => $event_id, 'contact_id' => $contact_id, 'participant_fee_level' => $fee_level, 'participant_fee_amount' => $eventPrice];
      $result = civicrm_api3('Participant', 'create', array_merge($participant, $participantParms, $add_participant));
      if ($result['is_error'])
        throw new CRM_Core_Exception($errMsg, 'RE0009', $result);
      else
        $participant = $result['values'][$result['id']];
      $contribution = ['contact_id' => $contact_id, 'financial_type_id' => 'Group Class Fee', 'amount_level' => $fee_level, 'total_amount' => $eventPrice, 'net_amount' => $eventPrice,];
      $result = civicrm_api3('Contribution', 'create', array_merge($contribution, $contributionParms));
      if ($result['is_error'])
        throw new CRM_Core_Exception($errMsg, 'RE0010', $result);
      else
        $contribution = $result['values'][$result['id']];
      $result = civicrm_api3('ParticipantPayment', 'create', ['contribution_id' => $contribution['id'], 'participant_id' => $participant['id'],]);
      if ($result['is_error'])
        throw new CRM_Core_Exception($errMsg, 'RE0011', $result);
      if (osa_add_to_commerce_cart('Event', $participant, $contact, $contribution) === FALSE)
        throw new CRM_Core_Exception($errMsg, 'RE0012', array_merge($participant, $contribution));
    }
  }

  // process volunteer payment in-lieu
  $cid = _osa_get_contact_id();
  $hid = _osa_getHousehold($cid);
  if (!empty($submission_data['donation_in_lieu'])) {
    $donationAmt = $submission_data['donation_amount'];
    $contribution = ['contact_id' => $hid, 'financial_type_id' => 'Donation', 'source' => "In-Lieu of Volunteering {$schoolYear}", 'total_amount' => $donationAmt, 'net_amount' => $donationAmt,];
    $result = civicrm_api3('Contribution', 'create', array_merge($contributionParms, $contribution));
    if ($result['is_error'])
      throw new CRM_Core_Exception($errMsg, 'RE0013', $result);
    else
      $contribution = $result['values'][$result['id']];
    if (osa_add_to_commerce_cart('Contribution', $contribution, $contact, $contribution) === FALSE)
      throw new CRM_Core_Exception($errMsg, 'RE0014', array_merge($participant, $contribution));
  }

  // process volunteer duties
  if (!empty($submission_data['volunteer_duties'])) {
    $needIds = explode(',', $submission_data['volunteer_duties']);
    $result = civicrm_api3('VolunteerNeed', 'get', ['id' => ['IN' => $needIds],]);
    if ($result['is_error'])
      throw new CRM_Core_Exception($errMsg, 'RE0015', $result);

    foreach ($result['values'] as $need) {
      $vol_activity = [
        'source_contact_id' => $cid,
        'volunteer_need_id' => $need['id'],
        'activity_date_time' => $need['start_time'],
        'assignee_contact_id' => $hid,
        'status_id' => 'Scheduled',
        'credits' => $need['duration'], // credits
      ];
      CRM_Volunteer_BAO_Assignment::createVolunteerActivity($vol_activity);
    }
  }
}

/**
 * Get the Volunteer Assignments from database
 */
function osa_get_volunteer_assignments($hid) {
  static $assignments = [];
  if (!isset($hid))
    return [];

  if (isset($assignments[$hid]))
    return $assignments[$hid];

  $schoolDates = osa_get_school_dates();
  $familyIds = _osa_getHouseholdMembers($hid, 'id');
  $familyIds[] = $hid;

  // use Activity instead of VolunteerAssignment since API sucks
  $result = civicrm_api3('Activity', 'get', [
    'activity_type_id' => "Volunteer",
    'assignee_contact_id' => ['IN' => $familyIds],
    'activity_date_time' => ['BETWEEN' => [$schoolDates['start']->format('Y-m-d'), $schoolDates['end']->format('Y-m-d')]],
    'status_id' => ['IN' => ['Scheduled', 'Available', 'Completed']],
  ]);

  if ($result['is_error'])
    return [];

  $assignments[$hid] = $result['values'];
  return $assignments[$hid];
}

/**
 * Implements hook_views_query_alter() get volunteer activities for a family
 */
function osa_views_query_alter_volunteer_commitments(&$view, &$query) {
  $family_ids = _osa_getHouseholdMembers($view->args[0], 'id');
  $family_ids[] = $view->args[0];

  // Traverse through the 'where' part of the query.
  foreach ($query->where as &$condition_group) {
    foreach ($condition_group['conditions'] as &$condition) {
      // condition to filter on contact
      if ($condition['field'] == 'civicrm_activity_contact.contact_id') {
        $condition = [
          'field' => 'civicrm_activity_contact.contact_id',
          'value' => $family_ids,
          'operator' => 'in',
        ];
      }
    }
  }
}


/**
 * helper function to create an array key for the price table
 */
function _osa_registration_price_key_component($option) {
  static $key_component = [];
  if (!empty($key_component[$option]))
    return $key_component[$option];

  $tmp_key = [];

  if (strpos($group_level, 'Parent Training') !== FALSE) {
    $tmp_key[] = 'Parent Training';
  } elseif (strpos($option, 'RCM Theory') !== FALSE) {
    $tmp_key[] = 'RCM';
  } elseif (strpos($option, 'Reading') !== FALSE) {
    $tmp_key[] = 'Reading';
  } else {
    if (strpos($option, 'Kodaly') !== FALSE) {
      $tmp_key[] = 'Kodaly';
    }
    if (preg_match('/\((.* min)/', $option, $minutes)) {
      $tmp_key[] = $minutes[1];
    }
  }

  $key_component[$option] = str_replace([' ', '-'], '_', implode('_', $tmp_key));
  return $key_component[$option];
}

/**
 * Not really a validate function
 * 
 */
function osa_registration_validate($form, &$form_state) {
  // call the alter routine to make sure the form is re-created
  osa_form_alter_registration($form, $form_state);
  return;
}
