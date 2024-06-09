<?php

/**
 * @file
 * Functions that customize CiviCRM pages and forms
 */
require_once 'osa.inc';
require_once 'osa.group_class.inc';

/**
 * Override what is on the Contact Dashboard to make a 'Family Profile' page
 *
 * @param object $page
 *   The CRM_Contact_Page_View_UserDashBoard object being rendered.
 */
function osa_civicrm_pageRun_CRM_Contact_Page_View_UserDashBoard(&$page) {
  views_add_js('ajax');
  views_add_js('ajax_view');
  
  drupal_add_js(drupal_get_path('module', 'osa') . '/osa.family.js');
  drupal_add_css(drupal_get_path('module', 'osa') . '/osa.family.css');
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
  // also see custom_php/CRM/Profile/Form/Edit.php
  // because we need to set the _postURL and it is protected
  $context = CRM_Utils_Request::retrieve('context', 'String');
  if ($context == 'boxload') {
  }

  // make sure we don't loose the profile id
  if ($gid) {
    $form->addElement('hidden', 'gid', $gid);
    $form->set('gid', $gid);
  }
}

/**
 * Add custom validation to CiviCRM Profile based forms
 */
function osa_civicrm_validateForm_CRM_Profile_Form_Edit(&$fields, &$files, &$form, &$errors) {
  if (!isset($errors)) $errors = array();
  $errors = array_merge($errors, osa_civicrm_validate_CRM_Profile_Form_Edit($fields, $files, $form));
}

function osa_civicrm_validate_CRM_Profile_Form_Edit(&$fields, &$files, &$form) {
  $errors = array();
  $gid = $form->getVar('_gid');

  // validate the Student Details Profile
  if ($gid == _osa_profileId('Student')) {
    // ensure a Teacher is entered for each instrument selected
    $names = array('primary', 'second', 'third');
    $f_ids = array();
    foreach ($names as $name) {
      $f_ids[] = array(
        'instrument' => _osa_field("{$name}_instrument"),
        'teacher' => _osa_field("{$name}_teacher"),
      );
    }
    foreach ($f_ids as $f_id) {
      if (isset($fields[$f_id['instrument']]) && !empty($fields[$f_id['instrument']])) {
        if (isset($fields[$f_id['teacher']]) && empty($fields[$f_id['teacher']])) {
          $errors[$f_id['teacher']] = ts('Teacher is a required field for the selected instrument.');
          $form->setElementError($f_id['teacher'], $errors[$f_id['teacher']]);
        }
      }
    }
  }
  
  return $errors;
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

  // most of our contributions are from authenticated users,
  // so remove the email field - must include it in a Profile
  $elementName = "email-{$form->_bltID}";
  if ($form->elementExists($elementName))
    $form->removeElement($elementName);
  
  $form->setDefaults(array('payment_processor' => $form->_paymentProcessor['id'] ));

  // check if this is not the membership form (we only use one for memberships)
  if ($form->_id != OSA_CONTRIB_MEMBER) {
    // all other contributions can be made on behalf of a family member
    // so add a drop down of family members
    _osa_addFamilyField($form, ($form->_id == OSA_CONTRIB_TEACHER));

    // custom processing for the teacher registration form
    if ($form->_id == OSA_CONTRIB_TEACHER) {
      osa_civicrm_buildForm_teacher_registration($form);
    }
    // custom processing for the donation forms
    elseif (($form->_id == OSA_CONTRIB_DONATE) || ($form->_id == OSA_CONTRIB_ADJUST)) {
      drupal_add_css('.discount-info { display: none; }', array('type' => 'inline'));
    }
  }
  // membership form
  else {
    // add next year's school year to the title
    $membership = _osa_get_membership($form->_membershipContactID);
    if (isset($membership['end_date']) && $membership['is_current_member']) {
      $end_date = new DateTimeEx($membership['end_date']);
    }
    else {
      $membershipDates = _osa_get_membership_dates();
      $end_date = new DateTimeEx($membershipDates['end_date']);
      // need a date in the middle of the school year as the membership end date (August) is after the school end date (June)
      $end_date->modify('-6 months');
    }
    $school_year = _osa_school_year($school_start_date, $school_end_date, $end_date->format('Y-m-d'));
    $form->_values['title'] = trim("$school_year {$form->_values['title']}");
    drupal_set_title($form->_values['title']);
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
function osa_civicrm_validateForm_CRM_Contribute_Form_Contribution_Main(&$fields, &$files, &$form, &$errors) {
  if (!isset($errors)) $errors = array();
  $errors = array_merge($errors, osa_civicrm_validate_CRM_Contribute_Form_Contribution_Main($fields, $files, $form));
}

function osa_civicrm_validate_CRM_Contribute_Form_Contribution_Main(&$fields, &$files, &$form) {
  $errors = array();
  
  // custom processing for the membership registration form
  if ($form->_id == OSA_CONTRIB_MEMBER) {
    // validate the Student Info profile (not sure why this isn't automatic)
    $form->setVar('_gid', _osa_profileId('Student'));
    $tmp = osa_civicrm_validate_CRM_Profile_Form_Edit($fields, $files, $form);
    if (is_array($tmp)) {
      $errors += $tmp;
    }
  }

  // custom processing for the teacher registration form
  if ($form->_id == OSA_CONTRIB_TEACHER) {
    $tmp = osa_civicrm_validate_teacher_registration($fields, $files, $form);
    if (is_array($tmp)) {
      $errors += $tmp;
    }
  }

  return $errors;
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
 * Add custom processing when generating Contribution Thank you page
 *
 * @param $form
 *   The CRM_Contribute_Form_Contribution_Confirm object being rendered.
 */
function osa_civicrm_buildForm_CRM_Contribute_Form_Contribution_ThankYou(&$form) {
  global $base_url;

  // except for membership, we need the logged in user to be the contact
  if ($form->_id == OSA_CONTRIB_MEMBER) {
    $contact = _osa_get_contact($form->_membershipContactID);
    drupal_set_message("<br /><span style=\"font-size:120%; color:#B22222;\"><em>It is <strong>highly recommended</strong> that all registered OSA students <strong>participate in group classes</strong>.</em></span><br /><br />
                        Click one of the links below to sign-up.<br />
                        You can also return to your <a href=\"{$base_url}/civicrm/user?reset=1\">Family Profile</a> to register another child, or click <a href=\"{$base_url}/checkout\">here</a> to complete your payment.");

    drupal_goto("{$base_url}/group_class_selection/{$form->_membershipContactID}");
  }
  else {
    $contact = _osa_get_contact($form->_contactID);
  }
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
          if (substr($custom_data['fields'][$fld_key]['field_value'], 0, 2) == 'No') {
            unset($event_custom_data[$cd_key]);
          }
          else {
            $event_custom_data[$cd_key][$ecd_key]['fields'][$fld_key]['field_title'] = ts('Restricted to OSA Members Only');
            $event_custom_data[$cd_key][$ecd_key]['fields'][$fld_key]['field_value'] = '';
          }
        }
        if ($custom_data['fields'][$fld_key]['field_title'] == 'Open to General Public') {
          if (substr($custom_data['fields'][$fld_key]['field_value'], 0, 3) == 'Yes') {
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
}

/**
 * Add custom validation to Event registration forms
 */
function osa_civicrm_validateForm_CRM_Event_Form_Registration_Register(&$fields, &$files, &$form, &$errors) {
  if (!isset($errors)) $errors = array();
  $errors = array_merge($errors, osa_civicrm_validate_CRM_Event_Form_Registration_Register($fields, $files, $form));
}

function osa_civicrm_validate_CRM_Event_Form_Registration_Register(&$fields, &$files, &$form) {
  $errors = array();

  // if we added a Family member selection box
  if (isset($fields['contact_id'])) {
    // ensure a contact_id was selected
    if ($fields['contact_id'] == '0') {
      $errors['contact_id'] = ts('Family Member is a required field.');
      $form->setElementError('contact_id', $errors['contact_id']);
    }
    // add the selected contact_id to the request, so that the participant record gets created properly
    $_REQUEST['cid'] = $fields['contact_id'];
  }

  return $errors;
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

  
/* No longer needed, because we can configure the Event to disable the Confirmation page
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

    // Redirect to thank you page
    $qfKey = $form->controller->_key;
    CRM_Utils_System::redirect(CRM_Utils_System::url('civicrm/event/register', "_qf_ThankYou_display=true&qfKey=$qfKey", TRUE, NULL, FALSE));
  }
*/
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
  // add processing for Volunteers
  if (class_exists('CRM_Volunteer_BAO_Project')) {
    $projects = CRM_Volunteer_BAO_Project::retrieve(array('entity_id' => $form->_eventId, 'entity_table' => 'civicrm_event', 'is_active' => 1,));

    if (count($projects) && CRM_Core_Permission::check('register to volunteer')) {
      $project = current($projects);
      $volunteer['url'] = CRM_Utils_System::url('civicrm/volunteer/signup', array('reset' => 1, 'vid' => $project->id), FALSE, NULL, TRUE, TRUE);
      $volunteer['button_text'] = ts('Volunteer Now');
      $volunteer['text'] = ts('<strong>We need your help!</strong> Events like this cannot be successful without volunteers.');
      $form->assign('volunteer', $volunteer);
    }
  }
}

/**
 * There is no nice CiviCRM way to display the participant details to the participant
 */
function osa_civicrm_participant_view() {
  $pid = $_REQUEST['id'];
  $snippet = $_REQUEST['snippet'];

  // make sure this user can view this record
  civicrm_initialize();
  $participant = _osa_get_participant($pid);
  if ((_osa_get_contact_id() != $participant['contact_id']) && !CRM_Contact_BAO_Contact_Permission::allow($participant['contact_id'])) {
    CRM_Core_Error::fatal(ts('You do not have permission to view this participant'));
  }

  $html = _osa_civicrm_participant_theme($pid, 'long');

  // output either the raw html or return of a full themed page
  if ($snippet) {
    $html = '<div class="crm-container" bgColor="white">' . $html . '<br/><a class="button" href="#" onclick="jQuery.colorbox.close();" style="margin: 5px"> OK </a><br/></div>';
    print $html;
    die();
  }
  return $html;
}

/**
 * generate an html table with the participant data
 */
function _osa_civicrm_participant_theme($pid, $style) {
  $html = '';
  $rows = array();

  // get all of the data associated with the participant
  civicrm_initialize();
  $template = CRM_Core_Smarty::singleton();

  $participant = _osa_get_participant($pid);
  $event = civicrm_api('Event', 'getsingle', array('id' => $participant['event_id'], 'version' => 3,));
  $line_items = CRM_Price_BAO_LineItem::getLineItems($pid);

  $profiles = array();
  $result = civicrm_api('UFJoin', 'get', array('entity_table' => 'civicrm_event', 'module' => 'CiviEvent', 'entity_id' => $participant['event_id'], 'version' => 3));
  foreach($result['values'] as $id => $ufjoin) {
    $group_id = $ufjoin['uf_group_id'];
    if (($group_id == 29) || ($group_id == 34)) continue; // Participant Info
    $profiles[$group_id] = civicrm_api('UFGroup', 'getsingle', array('id' => $group_id,'version' => 3,));
    $custom_display = CRM_Event_BAO_Event::buildCustomDisplay($group_id, 'custom', $participant['contact_id'], $template, $pid, FALSE, TRUE);
    $profiles[$group_id]['fields'] = isset($custom_display[0][0]) ? $custom_display[0][0] : array();
  }

  // Event/Participant Info Section
  $display = array(
    'Event' => $event['title'],
    'Date' => _osa_format_event_date($event['start_date'], $event['end_date']),
    'Participant' => $participant['display_name'],
  );
  if ($event['event_type_id'] == OSA_EVENT_GROUP_CLASS_MASTER) {
    unset($display['Date']);
  }
  if ($style == 'long') {
    $display += array(
      'Registered Date' => _osa_format_event_date($participant['participant_register_date']),
      'Status' => $participant['participant_status'],
    );
  }
  // create the table rows
  foreach ($display as $name => $val) {
    $rows[] = array(
      'data' => array(
        array('data' => $name,),
        array('data' => $val),
      ),
    );
  }

  // when style is long, add a header row a separate table for each subsection of data 
  if ($style == 'long') {
    $header = array(
      array('data' => 'Participant Details', 'colspan' => 2),
    );
    $html .= htmlspecialchars_decode(theme('table', array('header' => $header, 'rows' => $rows, 'sticky' => FALSE, 'no_striping' => TRUE,)));
    $rows = array();
  }

   // custom processing for Summer Camp
  if ($event['event_type_id'] == OSA_EVENT_SUMMER_CAMP) {
    $sid = $participant['submission_id'];
    module_load_include('inc', 'webform', 'includes/webform.submissions');
    $submissions = webform_get_submissions(array('sid' => $sid));
    $node = node_load($submissions[$sid]->nid);
    $values = array();
    $amount = 0.00;
    $camp_html = '';
    _osa_camp_parse_submission($node->webform['components'], $submissions[$sid]->data, $values, $amount, $camp_html, $style);
    $html .= $camp_html;
  }
  else {
    // Pricing Section
    foreach ($line_items as $line_item) {
      if ($line_item['field_title'] == 'Fees' || $line_item['field_title'] == 'Fee(s)') {
        $label = $line_item['label'];
      }
      else {
        $label = $line_item['field_title'] . ' - ' . $line_item['label'];
      }
      if ($line_item['qty'] > 1) {
        $label .= " (x {$line_item['qty']})";
      }
      
      $rows[] = array(
        'data' => array(
          array('data' => $label,),
          array('data' => ((float)$line_item['line_total'] > 0.001) ? '$ ' . $line_item['line_total'] : '', 'style' => 'text-align: right;'),
        ),
      );
    }
    
    if ($style == 'long') {
      $html .= '<br/>';
      $header = array(
        array('data' => 'Event Fee(s)', 'colspan' => 2),
      );
      if ((float)$participant['participant_fee_amount'] > 0.001) {
        $rows[] = array(
          'data' => array(
            array('data' => 'Total Amount', ),
            array('data' => '$ ' . $participant['participant_fee_amount'], 'style' => 'text-align: right;'),
          ),
          'style' => 'font-weight: bold; background-color: transparent;',
        );
      }
      $html .= htmlspecialchars_decode(theme('table', array('header' => $header, 'rows' => $rows, 'sticky' => FALSE, 'no_striping' => TRUE,)));
      $rows = array();
    }
    
    // Show custom data elements
    foreach ($profiles as $gid => $profile) {
      $has_values = FALSE;
      foreach ($profile['fields'] as $fname => $fvalue) {
        if (!empty($fvalue)) {
          $has_values = TRUE;
          $rows[] = array(
            'data' => array(
              array('data' => $fname,),
              array('data' => is_array($fvalue) ? implode(', ', $fvalue) : $fvalue,),
            ),
          );
        }
      }
      if ($has_values && ($style == 'long')) {
        $html .= '<br/>';
        $header = array(
          array('data' => $profile['title'], 'colspan' => 2),
        );
        $html .= htmlspecialchars_decode(theme('table', array('header' => $header, 'rows' => $rows, 'sticky' => FALSE, 'no_striping' => TRUE,)));
        $rows = array();
      }
    }
    // if we haven't generated the table yet
    if (!empty($rows)) {
      $html .= htmlspecialchars_decode(theme('table', array('header' => $header, 'rows' => $rows, 'sticky' => FALSE, 'no_striping' => TRUE,)));
    }
  }

  return $html;
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
    if (($fieldID == 24) || ($fieldID == 86) || ($fieldID == 87) || ($fieldID == 88) || ($fieldID == 175)) {
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
//      uasort($cache[$fieldID]['detail'], function ($a, $b) {return strcmp($a['label'], $b['label']);});
//      asort($cache[$fieldID]['std']);
      
      // add a non-OSA teacher
      $value = 'nonOSA';
      $label = 'Non-OSA teacher';
      $cache[$fieldID]['detail']['teacher_' . $value] = array(
        'id'    => 'teacher_' . $value,
        'value' => $value,
        'label' => $label,
      );
      $cache[$fieldID]['std'][$value] = $label;

      $cache[24] = $cache[$fieldID];
      $cache[86] = $cache[$fieldID];
      $cache[87] = $cache[$fieldID];
      $cache[88] = $cache[$fieldID];
    }
    // Group Node selection
    elseif ($fieldID == 25) {
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
    elseif ($fieldID == 32) {
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

  if (empty($options)) {
    $options = array();
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

/**
 * For webform_civicrm forms handle wait lists
 * moved to Rules
function _osa_webform_submission_presave_enable_waitlist(&$node, &$submission) {

  // get the event id
  $submission_data = _osa_parse_webform_submission_data($node->webform['components'], $submission->data);
  $form_key = 'civicrm_1_participant_1_participant_event_id';
  if (empty($submission_data[$form_key]))
    return;
  list($event_id) = explode('-', $submission_data[$form_key]);

  // check if the event is now full
  civicrm_initialize();
  $full = CRM_Event_BAO_Participant::eventFull($event_id);
  if (empty($full))
    return;
  $component = _osa_get_webform_component($node, $form_key);

  // if the event is full, put participant on the wait list
}
*/

/**
 *
function osa_webform_submission_presave_graduation(&$node, &$submission) {
  _osa_webform_submission_presave_enable_waitlist($node, $submission);
}
*/
