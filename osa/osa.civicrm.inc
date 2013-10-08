<?php

/**
 * @file
 * Functions that customize CiviCRM pages and forms
 */
require_once 'osa.inc';
require_once 'osa.group_class.inc';
require_once 'osa.teacher.inc';

/**
 * Override what is on the Contact Dashboard to make a 'Family Profile' page
 *
 * @param object $page
 *   The CRM_Contact_Page_View_UserDashBoard object being rendered.
 */
function osa_civicrm_pageRun_CRM_Contact_Page_View_UserDashBoard(&$page) {
  require_once 'api/api.php';
  
  // get current user's contact id
  $uid = _osa_get_contact_id();
  
  // reset the session if it got corrupted (opening civicrm forms with different ids)
  if ($uid != $page->_contactId) {
    $session = CRM_Core_Session::singleton();
    $session->reset();
    $session->set('userID', $uid);
  }

  // use the contact id passed in the URL, or the household of the current user
  $cid = CRM_Utils_Request::retrieve('id', 'Positive');
  if (isset($cid)) {
    $hid =_osa_getHousehold($cid);
  }
  else {
    $cid = $uid;
    $hid =_osa_getHousehold($cid);
    if (isset($hid)) {
      $cid = $hid;
    }
    else {
      CRM_Utils_System::setUFMessage(
        ts('You do not have a Family Profile. Click <b>') .
        CRM_Utils_System::href( ts('here'), 'civicrm/profile/create', 'reset=1&gid=' . OSA_PROFILE_FAMILY) .
        ts('</b> if you would like to create one.') . 
        ts('<br>Note you must create a Family Profile to register people other than yourself for Membership or Events')
      );
    }
  }

  // take the opportunity to clean house (only once a day)
  if (isset($hid)) {
    $clean_time = isset($_SESSION['osa']['manageHouseholdRelationships'][$hid]) ? $_SESSION['osa']['manageHouseholdRelationships'][$hid] : 0;
    if (($clean_time + 86400) < $_SERVER['REQUEST_TIME']) {
      _osa_manageHouseholdRelationships($hid);
      $_SESSION['osa']['manageHouseholdRelationships'][$hid] = $_SERVER['REQUEST_TIME'];
    }
  }
  
  // $cid could be:
  //   the contact id passed in the URL,
  //   the household of the current user,
  //   or the current user
  // so the get the contact object being displayed
  $pg_contact = _osa_get_contact($cid, FALSE);
  // @todo image_URL not currently returned by api
  $pg_contact['image_URL'] = CRM_Contact_BAO_Contact_Utils::getImage($pg_contact['osa_contact_type'], FALSE, $cid);
  $page->assign('contact', $pg_contact);

  // init vars & arrays
  $parents         = array();
  $students        = array();
  $teachers        = array();
  $others          = array();
  $smarty          = $page->getTemplate();
  $members         = array(); // overwrite list to display name
  $event_rows      = array();
  $contribute_rows = array();

  // get all the family members for this contact
  $family = _osa_getHouseholdMembers($cid);

  foreach ($family as $person) {
    $id = $person['contact_id'];
    
    // get membership status
    $membership = _osa_get_membership($id);
    if (isset($membership)) {
      $person['contact']['is_current_member'] = $membership['is_current_member'];
      $person['membership']['status'] = $membership['status'];
      $membership['display_name'] = $person['contact']['display_name'];
      $members[] = $membership;
    }

    // separate people by type to display in separate lists
    $contact_type = $person['contact_type'];
    if ($person['contact']['is_parent']) {
      $person['links'] = _osa_related_links('Parent', $id, $person['relationship']['id']);
      $parents[] = $person;
    }
    if ($person['contact']['is_student']) {
      $person['links'] = _osa_related_links('Student', $id, $person['relationship']['id']);
      $students[] = $person;
    }
    if (!$person['contact']['is_parent'] && !$person['contact']['is_student']) {
      $person['links'] = _osa_related_links('Other', $id, $person['relationship']['id']);
      $others[] = $person;
    }

    // append family member elements to lists in the page
    $teachers += _osa_getTeachers($person['contact']);
    $event_rows += _osa_get_participant_events($id);
    $contribute_rows += _osa_get_contributions($id);
  }

  // special processing for group classes
  osa_overwrite_class_participants($event_rows);
  
  // sort and trim $contribute_rows to a max of 20
  krsort($contribute_rows);
  $contribute_rows = array_slice($contribute_rows, 0, 20, TRUE);
  
  // (re)assign Smarty variables
  $page->assign('parents', $parents);
  $page->assign('students', $students);
  $page->assign('teachers', $teachers);
  $page->assign('others', $others);
  $page->assign('members', $members);
  $page->assign('event_rows', $event_rows);
  $page->assign('contribute_rows', $contribute_rows);
  
  // use the CiviCRM Profiles to display tombstone info
  require_once 'CRM/Profile/Page/Dynamic.php';
  $gids = array('Phone and Email' => OSA_PROFILE_PHONE_EMAIL, 'Address' => OSA_PROFILE_ADDRESS);
  $profile_elements = array();
  foreach ($gids as $title => $gid) {
    $profile = new CRM_Profile_Page_Dynamic($cid, $gid, 'Profile');
    $profile_elements[] =
      array(
        'cid'   => $cid,
        'gid'   => $gid,
        'title' => $title,
        'html'  => $profile->run(),
    );
  }
  // add button to Edit
  if (isset($hid)) {
    $profile_elements[0] +=
      array(
        'edit_cid'   => $hid,
        'edit_gid'   => OSA_PROFILE_FAMILY,
        'edit_title' => ts('Edit Family Details'),
      );
  }
  
  $page->assign('profileElements', $profile_elements);

  // override the templates used for each dashboard element
  $dashboard_elements = array();
  if (isset($hid)) {
    $dashboard_elements[] = array(
      'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Family.tpl',
      'sectionTitle' => 'Family Members',
      'sectionState' => '' // open
    );
    $dashboard_elements[] = array(
      'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Relationship.tpl',
      'sectionTitle' => 'Other Related Individual(s)',
      'sectionState' => empty($others) ? 'collapsed' : '',
    );
  }

  $dashboard_elements[] = array(
    'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Membership.tpl',
    'sectionTitle' => 'Student Member(s)',
    'sectionState' => empty($members) ? 'collapsed' : '',
  );
  $dashboard_elements[] = array(
    'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Teacher.tpl',
    'sectionTitle' => 'Private Teacher Registration(s)',
    'sectionState' => empty($teachers) ? 'collapsed' : '',
  );
  $dashboard_elements[] = array(
    'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Event.tpl',
    'sectionTitle' => 'Event Registration(s)',
    'sectionState' => empty($event_rows) ? 'collapsed' : '',
  );
  $dashboard_elements[] = array(
    'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Contribute.tpl',
    'sectionTitle' => 'Recent Payment(s)',
    'sectionState' => empty($contribute_rows) ? 'collapsed' : '',
  );
  $page->assign('dashboardElements', $dashboard_elements);

  // override the page title (do this last as generating profiles also overrides the title)
  CRM_Utils_System::setTitle($pg_contact['display_name'], $pg_contact['image_URL'] . ' ' . $pg_contact['display_name']);
}

/**
 * Add custom processing when generating CiviCRM Profile based forms
 *
 * @param $form
 *   The CRM_Profile_Form_Edit object being rendered.
 */
function osa_civicrm_buildForm_CRM_Profile_Form_Edit(&$form) {
  
  $gid = $form->getVar('_gid');
  $mode = $form->getVar('_mode');
  
  // set defaults in the create family profile form
  if ($gid == OSA_PROFILE_FAMILY) {
    if ($mode == CRM_Profile_Form::MODE_CREATE) {
      $contact = _osa_get_contact(NULL, FALSE);
      // catch this error
      if ($contact['id'] == 1) {
        watchdog('osa', 'Family for admin??<pre>' . print_r(debug_backtrace(), TRUE) . '</pre>', NULL, WATCHDOG_ERROR);
        $session = CRM_Core_Session::singleton();
        $session->reset();
      }
      else {
        $defaults = array(
          'household_name' => $contact['last_name'] . ' Family',
          'email-Primary' => $contact['email'],
        );
        if (isset($contact['phone'])) {
          $defaults['phone-Primary-1'] = $contact['phone'];
        }
        if (isset($contact['street_address'])) {
          $defaults['street_address-1'] = $contact['street_address'];
        }
        if (isset($contact['city'])) {
          $defaults['city-1'] = $contact['city'];
        }
        if (isset($contact['state_province_id'])) {
          $defaults['state_province-1'] = $contact['state_province_id'];
        }
        if (isset($contact['country_id'])) {
          $defaults['country-1'] = $contact['country_id'];
        }
        if (isset($contact['postal_code'])) {
          $defaults['postal_code-1'] = $contact['postal_code'];
        }
        $form->setDefaults($defaults);
      }
    }
    // make sure we go back to the proper page
    $form->removeElement('postURL');
    $form->removeElement('cancelURL');
    $form->addElement('hidden', 'postURL', $_SERVER['HTTP_REFERER']);
    $form->addElement('hidden', 'cancelURL', CRM_Utils_System::url('civicrm/user', 'reset=1'));
    
    // hack a token (the school year) into the form message
    $yr = _osa_school_year($school_start_date, $school_end_date);
    reset($form->_fields);
    $first_field = key($form->_fields);
    $form->_fields[$first_field]['groupHelpPre'] = str_replace('%school_year%', $yr, $form->_fields[$first_field]['groupHelpPre']);
    $form->assign('fields', $form->_fields);    
  }
  // add a relationship type selection to the Related Individual profile form
  elseif ($gid == OSA_PROFILE_REL_INDIVIDUAL) {
    if ($mode == CRM_Profile_Form::MODE_CREATE) {
      _osa_addRelationshipField($form, $_GET['hid']);
    }
  }

  // customize profile forms displayed in a colorbox
  $context = CRM_Utils_Request::retrieve('context', 'String');
  if ($context == 'boxload') {
    // add required css and js
    global $base_url;
//    $pkg_path = $base_url . '/sites/all/modules/civicrm/packages/jquery/';
    $pkg_path = 'sites/all/modules/civicrm/packages/jquery/';
    $css_files = array(
      $pkg_path . 'jquery-ui-1.9.0/css/smoothness/jquery-ui-1.9.0.custom.min.css',
    );
    $js_files = array(
      $pkg_path . 'jquery-ui-1.9.0/js/jquery-ui-1.9.0.custom.min.js',
      $pkg_path . 'plugins/jquery.timeentry.pack.js',
      $pkg_path . 'plugins/jquery.chainedSelects.js',
    );

    $out = '';
    foreach ($css_files as $css_file) {
//      $out .= '<link type="text/css" rel="stylesheet" href="' . $css_file . '" media="all" />';
      drupal_add_css($css_file);
    }
    $out .= '<script type="text/javascript">var cj = jQuery;</script>';
    foreach ($js_files as $js_file) {
//      $out .= '<script type="text/javascript" src="' . $js_file . '"></script>';
      drupal_add_js($js_file);
    }
//    echo $out;
    
    // make sure colorbox forms return to the current page
    $form->removeElement('postURL');
    $form->removeElement('cancelURL');
    $form->addElement('hidden', 'postURL', $_SERVER['HTTP_REFERER']);
    $form->addElement('hidden', 'cancelURL', $_SERVER['HTTP_REFERER']);
    $button_group = $form->getElement('buttons');
    $buttons = $button_group->getElements();
    $cancelButton = $buttons[1];
    $cancelButton->setType('button');
    $cancelButton->_attributes['onclick'] = 'jQuery.colorbox.close();';
  }
  
  // make sure we don't loose the profile id
  if ($gid) {
    $form->addElement('hidden', 'gid', $gid);
    $form->set('gid', $gid);
  }
}

/**
 * Add custom post processing to CiviCRM Profile based forms
 *
 * @param $form
 *   The CRM_Profile_Form_Edit object being processed.
 */
function osa_civicrm_postProcess_CRM_Profile_Form_Edit(&$form) {

  $gid  = $form->getVar('_gid');
  $mode = $form->getVar('_mode');
  $cid  = $form->getVar('_id'); // contact created/updated by profile form
  $hid  = CRM_Utils_Array::value('osa_hid', $_POST);

  // if we just created a new contact, setup the relationships, etc.
  if ($mode == CRM_Profile_Form::MODE_CREATE) {

    // default to the current users house if it wasn't passed and we need it
    if ((!$hid) && (($gid == OSA_PROFILE_PARENT) || ($gid == OSA_PROFILE_STUDENT) || ($gid == OSA_PROFILE_REL_INDIVIDUAL))) {
      $hid = _osa_getHousehold();
    }
  
    // if we are passed a household id, add the new contact to the house
    if (isset($hid)) {
      $type = OSA_REL_HOUSEHOLD_MEMBER;
      if ($gid == OSA_PROFILE_REL_INDIVIDUAL) {
        $type = _osa_getRelationshipField($form);
      }

      $contact = _osa_get_contact($cid, FALSE);
      $perm    = ($contact['osa_contact_type'] == 'Parent') ? OSA_PERM_OWNED : OSA_PERM_NONE;
      _osa_create_relationship($type, $cid, $hid, $perm);

      // set up all of the other relationships
      _osa_manageHouseholdRelationships($hid);
    }

    // if we just created the household itself, set current user to be the head
    if ($gid == OSA_PROFILE_FAMILY) {
      $hid = $cid;
      $cid = _osa_get_contact_id();
      _osa_create_relationship(OSA_REL_HEAD_HOUSEHOLD, $cid, $hid, OSA_PERM_OWNED);
      
      // update the current contact to be a Parent
      $result = civicrm_api('contact', 'create', array('id' => $cid, 'contact_sub_type' => 'Parent', 'version' => 3));
    }
    
    // set the family member's address to share the household address
    if ((isset($hid)) && ($gid != OSA_PROFILE_REL_INDIVIDUAL)) {
      $result = civicrm_api('address', 'get', array('contact_id' => $hid, 'location_type_id' => 1, 'version' => 3));
      if ($result['count'] > 0) {
        $house_address_id = $result['id'];
        $shared_address = $result['values'][$house_address_id];
        $shared_address['master_id'] = $house_address_id;
        $shared_address['use_shared_address'] = TRUE;
        $shared_address['contact_id'] = $cid;
        $shared_address['version'] = 3;

        $result = civicrm_api('address', 'get', array('contact_id' => $cid, 'location_type_id' => 1, 'version' => 3));
        if ($result['count'] > 0) {
          $result = civicrm_api('address', 'delete', array('id' => $result['id'], 'version' => 3));
        }

        unset($shared_address['id']);
        $result = civicrm_api('address', 'create', $shared_address);
      }
    }
  }
  elseif (($mode == CRM_Profile_Form::MODE_CREATE) || ($mode == CRM_Profile_Form::MODE_EDIT)) {
    // update the family profile to keep track of when it was last edited so we can prompt each year
    if ($gid == OSA_PROFILE_FAMILY) {
      $result = civicrm_api('contact', 'create', array('id' => $cid, _osa_field('last_registration') => date('Y-m-d'), 'version' => 3));
    }
  }
}

/**
 * Add custom processing when generating CiviCRM Contribution based forms
 *
 * @param $form
 *   The CRM_Contribute_Form_Contribution_Main object being rendered.
 */
function osa_civicrm_buildForm_CRM_Contribute_Form_Contribution_Main(&$form) {

  // starting with version 4.2 this function gets called once to build the form, and once to build the payments section
  // nothing to do if building the payments section
  if (isset($_GET['snippet']) && ($_GET['snippet'] == 4)) {
    return;
  }

  // all of our contributions are from authenticated users,
  // so remove the email for anonymous users
  $bltID = $form->_bltID;
  $form->removeElement("email-$bltID");
  
  $form->setDefaults(array('payment_processor' => $form->_paymentProcessor['id'] ));

  // check if this is the membership form (we only use one for memberships)
  if ($form->_id == OSA_CONTRIB_MEMBER) {
    // make sure that the family profile is updated during this school year
    $update_required = TRUE;
    $school_start_date = NULL;
    $school_end_date = NULL;
    $yr = _osa_school_year($school_start_date, $school_end_date);
    $cid = $_REQUEST['cid'];
    $hid = _osa_getHousehold($cid);
    $family = _osa_get_contact($hid);
    
    if (isset($family['Last_Registration_Date'])) {
      $last_registration = new DateTimeEx($family['Last_Registration_Date']);
      if ($last_registration >= $school_start_date->modify('-6 weeks')) {
        $update_required = FALSE;
      }
    }
    
    if ($update_required) {
      // use javascript to ensure HTTP_REFERER is set
      CRM_Utils_System::jsRedirect(CRM_Utils_System::url('civicrm/profile/edit', 'reset=1&gid=' . OSA_PROFILE_FAMILY . "&id=$hid"));
      return;
    }
  }
  else {
    // all other contributions can be made on behalf of a family member
    // so add a drop down of family members
    _osa_addFamilyField($form, ($form->_id == OSA_CONTRIB_TEACHER));

    // custom processing for the teacher registration form
    if ($form->_id == OSA_CONTRIB_TEACHER) {
      osa_civicrm_buildForm_teacher_registration($form);
    }
    // custom processing for the misc payment forms
    elseif (($form->_id == OSA_CONTRIB_DONATE) || ($form->_id == OSA_CONTRIB_ADJUST)) {
      drupal_add_css('.discount-info { display: none; }', array('type' => 'inline'));
    }
  }

  // fix the button label
  $group = $form->getElement('buttons');
  $buttons = $group->getElements();
  if ($form->_paymentProcessor['payment_processor_type'] == 'drupalcommerce') {
    $buttons[0]->setValue(ts('Add to Cart'));
  }
  else {
    $group = $form->getElement('buttons');
    $buttons = $group->getElements();
    $buttons[0]->setValue(ts('Next') . ' >>');
  }
}

/**
 * Add custom validation to CiviCRM Contribution based forms
 */
function osa_civicrm_validate_CRM_Contribute_Form_Contribution_Main(&$fields, &$files, &$form) {
  $errors = array();
  
  // custom processing for the teacher registration form
  if ($form->_id == OSA_CONTRIB_TEACHER) {
    $tmp = osa_civicrm_validate_teacher_registration($fields, $files, $form);
    if (is_array($tmp)) {
      $errors += $tmp;
    }
  }

  return empty($errors) ? TRUE : $errors;
}

/**
 * Add custom post processing to CiviCRM Contribution based forms
 *
 * @param $form
 *   The CRM_Contribute_Form_Contribution_Main object being processed.
 */
function osa_civicrm_postProcess_CRM_Contribute_Form_Contribution_Main(&$form) {

  // check if we are using the Drupal Commerce Cart
  if ($form->_paymentProcessor['payment_processor_type'] == 'drupalcommerce') {
    // since we use a Cart, we can disable the confirmation forms
    $confirm_form =& $form->controller->_pages['Confirm'];
    $confirm_form->preProcess();

    // Hack to tell form validator that confirmation page is valid
    $data =& $form->controller->container();
    $data['valid']['Confirm'] = 1;

    // Redirect to thank you page
    $confirm_form->postProcess();
    $qfKey = $form->controller->_key;
    CRM_Utils_System::redirect(CRM_Utils_System::url('civicrm/contribute/transact', "_qf_ThankYou_display=1&qfKey=$qfKey", TRUE, NULL, FALSE));
  }
}

/**
 * Add custom processing when generating Contribution Confirmation form
 *
 * @param $form
 *   The CRM_Contribute_Form_Contribution_Confirm object being rendered.
 */
function osa_civicrm_buildForm_CRM_Contribute_Form_Contribution_Confirm(&$form) {

  if (isset($form->_params['contact_id'])) {
    $cid = $form->_params['contact_id'];
    $contact = _osa_get_contact($cid, FALSE);
    $contact['contact_type'] = $contact['osa_contact_type'];

    $form->assign('contact', $contact);
    $form->assign('onbehalfFamily', TRUE);
    _osa_addField($form, 'hidden', "contact_id", $cid);
  }
  
  // custom processing for the teacher registration form
  if ($form->_id == OSA_CONTRIB_TEACHER) {
    osa_civicrm_buildForm_teacher_registration_confirm($form);
  }
}

/**
 * Add custom processing when processing the Contribution Confirmation form
 *
 * @param $form
 *   The CRM_Contribute_Form_Contribution_Confirm object being rendered.
 */
function osa_civicrm_postProcess_CRM_Contribute_Form_Contribution_Confirm(&$form) {
  // custom processing for the teacher registration form
  if ($form->_id == OSA_CONTRIB_TEACHER) {
    osa_civicrm_postProcess_teacher_registration_confirm($form);
  }
}

/**
 * Add custom processing when generating Contribution Thankyou page
 *
 * @param $form
 *   The CRM_Contribute_Form_Contribution_Confirm object being rendered.
 */
function osa_civicrm_buildForm_CRM_Contribute_Form_Contribution_ThankYou(&$form) {

  // we need the logged in contact
  $contact = _osa_get_contact();
  $form->assign('contact', $contact);
  $form->assign('cid', $contact['id']);
  $form->assign('email', $contact['email']);
  
  // custom processing for the teacher registration form
  if ($form->_id == OSA_CONTRIB_TEACHER) {
    osa_civicrm_buildForm_teacher_registration_thankyou($form);
  }
}

/**
 * Customize the Event Info page
 *
 * @param object $page
 *   The CRM_Event_Page_EventInfo object being rendered.
 */
function osa_civicrm_pageRun_CRM_Event_Page_EventInfo(&$page) {

  // get the custom data fields we want to customize
  $template = &$page->getTemplate();
  $event_custom_data = &$template->_tpl_vars['viewCustomData'];
  $cd_keys = array_keys($event_custom_data);
  
  foreach ($cd_keys as $cd_key) {
    $custom_data = current($event_custom_data[$cd_key]);
    $ecd_key = key($event_custom_data[$cd_key]);

    // hide zero discounts
    if ($custom_data['name'] == 'Discounts') {
      $fld_keys = array_keys($custom_data['fields']);
      foreach ($fld_keys as $fld_key) {
        if (($custom_data['fields'][$fld_key]['field_title'] == 'Member Discount') &&
            ($custom_data['fields'][$fld_key]['field_value'] == '0.00')) {
          unset($event_custom_data[$cd_key]);
          break;
        }
      }
    }

    // process custom fields we created for all events
    if ($custom_data['name'] == 'Event_Details') {
      $event_custom_data[$cd_key][$ecd_key]['title'] = '';
      $fld_keys = array_keys($custom_data['fields']);
      
      foreach ($fld_keys as $fld_key) {
        // show/hide 'Members Only' flag and 'Open to Public' flag
        if ($custom_data['fields'][$fld_key]['field_title'] == 'Members Only') {
          if ($custom_data['fields'][$fld_key]['field_value'] == "No\n") {
            unset($event_custom_data[$cd_key]);
          }
          else {
            $event_custom_data[$cd_key][$ecd_key]['fields'][$fld_key]['field_title'] = ts('Restricted to OSA Members Only');
            $event_custom_data[$cd_key][$ecd_key]['fields'][$fld_key]['field_value'] = '';
          }
        }
        if ($custom_data['fields'][$fld_key]['field_title'] == 'Open to General Public') {
          if ($custom_data['fields'][$fld_key]['field_value'] == "Yes\n") {
            $open_to_public = TRUE;
            $page->assign('open_to_public', TRUE);
          }
          else {
            $open_to_public = FALSE;
          }
          unset($event_custom_data[$cd_key]);
        }
      }
    }
  }

  // we have to set permissions to allow anonymous registration, but really we want it for only some events
  $allow_registration = $template->get_template_vars('allowRegistration');
  if ($allow_registration) {
    $uid = _osa_getUserId();
    if (!($uid > 0) && !$open_to_public) {
      $page->assign('allowRegistration', FALSE);
    }
  }

  // customize for group classes
  $event = $template->get_template_vars('event');
  if ($event['event_type_id'] == OSA_EVENT_GROUP_CLASS_MASTER) {
    osa_civicrm_pageRun_group_class_info($page);
  }
}

/**
 * Add custom processing when generating CiviCRM Event Registration forms
 *
 * @param $form
 *   The CRM_Event_Form_Registration_Register object being rendered.
 */
function osa_civicrm_buildForm_CRM_Event_Form_Registration_Register(&$form) {

  // starting with version 4.2 this function gets called once to build the form, and once to build the payments section
  // nothing to do if building the payments section
  if (isset($_GET['snippet']) && ($_GET['snippet'] == 4)) {
    return;
  }
  
  // get the details of the event
  $result = civicrm_api("event", "get", array('id' => $form->_eventId, 'version' => 3));

  // we have to set permissions to allow anonymous registration, but really we want it for only some events
  $uid = _osa_getUserId();
  $open_to_public = $result['values'][$form->_eventId][_osa_field('open_to_public')];
  if (!($uid > 0) && !$open_to_public) {
    drupal_set_message(t('You must be logged in to register for this event.'), 'warning');
    drupal_goto('user/login');
    return;
  }

  // registrations can be made on behalf of a family member
  // so if the user is logged in, add a drop down of family members
  if ($uid > 0) {
    // concerts & AGM don't require you to select a student
    $event_type = $result['values'][$form->_eventId]['event_type_id'];
    if ($event_type == OSA_EVENT_CONCERT || $event_type == OSA_EVENT_MEETING) {
      _osa_addField($form, 'hidden', "contact_id", _osa_get_contact_id());
    }
    else {
      $members_only = $result['values'][$form->_eventId][_osa_field('members_only')];
      _osa_addFamilyField($form, $members_only);
    }
    
    // add an email field, in order to pass profile validation
    _osa_addField($form, 'hidden', 'email-validate', 'TRUE');
    
    // remove the profiles related to contacts
    $smarty = $form->getTemplate();
    $custom_pre = $smarty->get_template_vars('customPre');
    if (is_array($custom_pre)) {
      foreach ($custom_pre as $field) {
        if (($field['field_type'] == 'Student') ||
            ($field['field_type'] == 'Individual') ||
            ($field['field_type'] == 'Contact')) {
          unset($form->_fields[$field['name']]);
          $form->removeElement($field['name']);
          unset($custom_pre[$field['name']]);
        }
      }
      $form->assign('customPre', $custom_pre);
    }
    
    $custom_post = $smarty->get_template_vars('customPost');
    if (is_array($custom_post)) {
      foreach ($custom_post as $field) {
        if (($field['field_type'] == 'Student') ||
            ($field['field_type'] == 'Individual') ||
            ($field['field_type'] == 'Contact')) {
          unset($form->_fields[$field['name']]);
          $form->removeElement($field['name']);
          unset($custom_post[$field['name']]);
        }
      }
      $form->assign('customPost', $custom_post);
    }
  }

  // if using the shopping cart, fix the button label
  if ($form->_paymentProcessor['payment_processor_type'] == 'drupalcommerce') {
    $group = $form->getElement('buttons');
    $buttons = $group->getElements();
    $buttons[0]->setValue(ts('Add to Cart'));
  }
  // if Elavon, remove unused fields from validation
  elseif ($form->_paymentProcessor['payment_processor_type'] == 'Elavon') {
    $bltID = $form->_bltID;
    $unused = array_flip(array("billing_city-$bltID", "billing_country_id-$bltID", 'billing_first_name', 'billing_middle_name', 'billing_last_name', "billing_postal_code-$bltID", "billing_state_province_id-$bltID", "billing_street_address-$bltID", 'cvv2'));
    $elementIndex = array_intersect_key($form->_elementIndex, $unused);
    foreach ($elementIndex as $name => $idx) {
      $form->removeElement($name);
    }
    $elementIndex = array_intersect_key($form->_duplicateIndex, $unused);
    foreach ($elementIndex as $name => $idx) {
      $form->removeElement($name);
    }
    $elementIndex = array_intersect_key($form->_paymentFields, $unused);
    foreach ($elementIndex as $name => $idx) {
      unset($form->_paymentFields[$name]);
    }
  }

  // special processing for group classes
  if ($form->_values['event']['event_type_id'] == OSA_EVENT_GROUP_CLASS_MASTER) {
    osa_civicrm_buildForm_group_class_registration($form);
  }
}

/**
 * Add custom validation to Event registration forms
 */
function osa_civicrm_validate_CRM_Event_Form_Registration_Register(&$fields, &$files, &$form) {
  $errors = array();

  // ensure a contact_id was selected
  if (isset($fields['contact_id']) && $fields['contact_id'] == '0') {
    $errors['contact_id'] = ts('Family Member is a required field.');
    $form->setElementError('contact_id', $errors['contact_id']);
  }

  return empty($errors) ? TRUE : $errors;
}

/**
 * Add custom post processing to CiviCRM Event Registration forms
 *
 * @param $form
 *   The CRM_Event_Form_Registration_Register object being processed.
 */
function osa_civicrm_postProcess_CRM_Event_Form_Registration_Register(&$form) {

  // if we are logged in, use the Family member selection to fill missing parameters
  $contact = _osa_get_contact();
  if ($contact) {
    $params = $form->get('params');
    $data =& $form->controller->container();
    $contact_id = $data['values']['Register']['contact_id'];
    $params[0]['contact_id'] = $contact_id;
    
    if ($form->_values['custom_pre_id']) {
      $profile_fields = CRM_Core_BAO_UFGroup::getFields($form->_values['custom_pre_id']);
      $values = array();
      CRM_Core_BAO_UFGroup::setProfileDefaults($contact_id, $profile_fields, $values);
      $params[0] += $values;
    }
    if ($form->_values['custom_post_id']) {
      $profile_fields = CRM_Core_BAO_UFGroup::getFields($form->_values['custom_post_id']);
      $values = array();
      CRM_Core_BAO_UFGroup::setProfileDefaults($contact_id, $profile_fields, $values);
      $params[0] += $values;
    }
    $bltID = $form->_bltID;
    $params[0]["email-$bltID"] = $contact['email'];
    $data['values']['Register']["email-$bltID"] = $contact['email'];
    
    $form->set('params', $params);
  }
  
  // check if we are using the Drupal Commerce Cart
  if ($form->_paymentProcessor['payment_processor_type'] == 'drupalcommerce') {
    // since we use a Cart, we want to disable the confirmation form
    $confirm_form = &$form->controller->_pages['Confirm'];

    // build the confirmation form to make sure all of the values are set
    $confirm_form->preProcess();
    $confirm_form->buildQuickForm();

    // tell form validator that confirmation page is valid
    $data =& $form->controller->container();
    $data['valid']['Confirm'] = 1;
    $confirm_form->postProcess();

    // special processing for group classes
    if ($confirm_form->_values['event']['event_type_id'] == OSA_EVENT_GROUP_CLASS_MASTER) {
      osa_civicrm_postProcess_group_class_registration($confirm_form);
    }
    
    // Redirect to thank you page
    $qfKey = $form->controller->_key;
    CRM_Utils_System::redirect(CRM_Utils_System::url('civicrm/event/register', "_qf_ThankYou_display=true&qfKey=$qfKey", TRUE, NULL, FALSE));
  }
}

/**
 * Add custom processing when generating Event Registration Confirm
 *
 * @param $form
 *   The CRM_Event_Form_Registration_Confirm object being rendered.
 */
function osa_civicrm_buildForm_CRM_Event_Form_Registration_Confirm(&$form) {
  // Mark all fields as not required
  foreach ($form->_fields as $name => $field) {
    if (isset($field['is_required']) && $field['is_required']) {
      $form->_fields[$name]['is_required'] = FALSE;
      $form->_required = array_diff($form->_required, array($name));
      unset($form->_rules[$name], $form->_errors[$name]);
    }
  }
}

/**
 * Add custom processing when generating Event Registration Thank you
 *
 * @param $form
 *   The CRM_Event_Form_Registration_ThankYou object being rendered.
 */
function osa_civicrm_buildForm_CRM_Event_Form_Registration_ThankYou(&$form) {
  // special processing for group classes
  if ($form->_values['event']['event_type_id'] == OSA_EVENT_GROUP_CLASS_MASTER) {
    osa_civicrm_buildForm_group_class_thankyou($form);
  }
}

/**
 * Implements hook_civicrm_buildAmount() to dynamically modify payment amounts
 */
function osa_civicrm_buildAmount($pageType, &$form, &$amount) {

  // custom  processing for the teacher registration form
  if (($pageType == 'contribution') && ($form->_id == OSA_CONTRIB_TEACHER)) {
    $values = $form->_submitValues;
    if (empty($values)) {
      return;
    }
    
    // check if QEP
    if (isset($values[_osa_field('lesson_length')]) &&
        isset($values[_osa_field('selected_teacher')]) &&
        isset($values[_osa_field('lesson_level')]) &&
        isset($values[_osa_field('lesson_location')]) &&
        ($values[_osa_field('lesson_location')] == 'QE Park')) {
      // set the amounts for each lesson length based on the teacher & level
      $teacher = _osa_get_contact($values[_osa_field('selected_teacher')]);
      $amount[6]['options'][11]['amount'] = $teacher[$values[_osa_field('lesson_level')]]['Price_for_30_minutes'];
      $amount[6]['options'][12]['amount'] = $teacher[$values[_osa_field('lesson_level')]]['Price_for_45_minutes'];
      $amount[6]['options'][13]['amount'] = $teacher[$values[_osa_field('lesson_level')]]['Price_for_60_minutes'];
    }
  }
}

/**
 * Implements hook_civicrm_customFieldOptions to dynamically populate custom selection boxes
 */
function osa_civicrm_customFieldOptions($fieldID, &$options, $detailedFormat = FALSE ) {

  static $cache = array();
  
  if (!isset($cache[$fieldID])) {
    $cache[$fieldID]['detail'] = array();
    $cache[$fieldID]['std'] = array();

    // Teachers selection
    if (($fieldID == 24) || ($fieldID == 86) || ($fieldID == 87) || ($fieldID == 88)) {
      // populate the teacher list
      _osa_get_teacher_lists($contacts, $select_options, $pricelevel, $prices30min, $prices45min, $prices60min);
      foreach ($select_options as $value => $label) {
        $cache[$fieldID]['detail']['teacher_' . $value] = array(
          'id'    => 'teacher_' . $value,
          'value' => $value,
          'label' => $label,
        );
        $cache[$fieldID]['std'][$value] = $label;
      }
      uasort($cache[$fieldID]['detail'], function ($a, $b) {return strcmp($a['label'], $b['label']);});
      asort($cache[$fieldID]['std']);
    }

    // Group Node selection
    if ($fieldID == 25) {
      // find all of the group class nodes
      $nodes = &_osa_get_class_nodes();
      foreach ($nodes as $class) {
        $cache[$fieldID]['detail']['class_' . $class->nid] = array(
          'id'    => 'class_' . $class->nid,
          'value' => $class->nid,
          'label' => $class->title,
        );
        $cache[$fieldID]['std'][$class->nid] = $class->title;
      }
      uasort($cache[$fieldID]['detail'], function ($a, $b) {return strcmp($a['label'], $b['label']);});
      asort($cache[$fieldID]['std']);
    }

    // Lesson Fee Level
    if ($fieldID == 32) {
      // populate the teacher list
      _osa_get_teacher_lists($contacts, $select_options, $pricelevel, $prices30min, $prices45min, $prices60min);
      foreach ($pricelevel as $teacher_id => $levels) {
        if (is_array($levels)) {
          foreach ($levels as $value => $label) {
            $cache[$fieldID]['detail']['pricelvl_' . $value] = array(
              'id'    => 'pricelvl_' . $value,
              'value' => $value,
              'label' => $label,
            );
            $cache[$fieldID]['std'][$value] = $label;
          }
        }
      }
      uasort($cache[$fieldID]['detail'], function ($a, $b) {return strcmp($a['label'], $b['label']);});
      asort($cache[$fieldID]['std']);
    }
  }

  if ($detailedFormat) {
    $options += $cache[$fieldID]['detail'];
  }
  else {
    $options += $cache[$fieldID]['std'];
  }
}

/**
 * Add custom processing when generating CiviCRM Email forms
 *
 * @param $form
 *   The object being rendered.
 *   Could be a CRM_Contact_Form_Task_Email, CRM_Member_Form_Task_Email
 *   CRM_Event_Form_Task_Email, CRM_Contribute_Form_Task_Email or
 *   CRM_Activity_Form_Task_Email object.
 */
function osa_civicrm_buildForm_CRM_Contact_Form_Task_EmailCommon(&$form) {

  // if there are contact's we want to email but they don't have their own
  // email address defined (i.e. Students), try to use their household's email
  if (isset($form->_allContactDetails) && isset($form->_contactDetails)) {
    $emailsNotSent = array_diff_assoc($form->_allContactDetails, $form->_contactDetails);
  }
  if (empty($emailsNotSent)) {
    return;
  }
  $smarty = $form->getTemplate();
  $suppressedEmails = $smarty->get_template_vars('suppressedEmails');
  $toArray = json_decode($smarty->get_template_vars('toContact'), TRUE);

  foreach ($emailsNotSent as $contactId => $value) {
    if ($value['do_not_email'] || CRM_Utils_Array::value('is_deceased', $value) || CRM_Utils_Array::value('on_hold', $value)) {
      continue;
    }

    // find the household
    $hid = _osa_getHousehold($contactId);
    if (isset($hid)) {
      $h_contact = _osa_get_contact($hid, FALSE);
      if (isset($h_contact['email'])) {
        $email = $h_contact['email'];
        $toArray[] = array(
          'name' => '"' . $value['sort_name'] . '" &lt;' . $email . '&gt;',
          'id'   => "$hid::{$email}",
        );
        $suppressedEmails--;
      }
    }
  }

  // quit now if there are no emails to add
  if (empty($toArray)) {
    return;
  }

  $form->assign('toContact', json_encode($toArray));
  $form->assign('suppressedEmails', $suppressedEmails);
}
