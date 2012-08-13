<?php

/**
 * @file
 * Functions that customize CiviCRM pages and forms
 */
require_once 'osa.inc';

/**
 * Override what is on the Contact Dashboard to make a 'Family Profile' page
 *
 * @param object $page
 *   The CRM_Contact_Page_View_UserDashBoard object being rendered.
 */
function osa_civicrm_page_CRM_Contact_Page_View_UserDashBoard(&$page) {
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

  // take the opportunity to clean house
  if (isset($hid)) {
    _osa_manageHouseholdRelationships($hid);
  }
  
  // $cid could be:
  //   the contact id passed in the URL,
  //   the household of the current user,
  //   or the current user
  // so the get the contact object being displayed
  $pgContact = _osa_get_contact($cid, FALSE);
  $pgContactType = isset($pgContact['contact_sub_type']) ? $pgContact['contact_sub_type'][0] : $pgContact['contact_type'];
  // @todo image_URL not currently returned by api
  $pgContact['image_URL'] = CRM_Contact_BAO_Contact_Utils::getImage($pgContactType, FALSE, $cid);
  $page->assign('contact', $pgContact);

  // init vars & arrays
  $parents         = array();
  $students        = array();
  $teachers        = array();
  $others          = array();
  $smarty          = $page->getTemplate();
  $members         = array(); // overwrite list to display name
  $event_rows      = $smarty->get_template_vars('event_rows');
  $contribute_rows = $smarty->get_template_vars('contribute_rows');
  $groupIn         = $smarty->get_template_vars('groupIn');

  // get all the family members for this contact
  $family = _osa_getHouseholdMembers($cid);

  foreach ($family as $person) {

    // get membership status
    $membership = _osa_get_membership($person['contact_id']);
    if (isset($membership)) {
      $person['contact']['is_current_member'] = $membership['is_current_member'];
      $person['membership']['status'] = $membership['status'];
      $membership['display_name'] = $person['contact']['display_name'];
      $members[] = $membership;
    }

    // separate people by type to display in separate lists
    $contact_type = $person['contact_type'];
    if ($contact_type == 'Parent') {
      $parents[] = $person;
    }
    elseif ($contact_type == 'Student') {
      $students[] = $person;
    }
    elseif ($contact_type == 'Teacher') {
      $teachers[] = $person;
    }
    elseif ($contact_type == 'Household') {
      // do nothing
    }
    else {
      $others[] = $person;
    }

    // append family member elements to lists in the page
    if ($person['contact_id'] != $page->_contactId) {
      $events = _osa_get_participant_events($person['contact_id']);
      if (isset($events)) {
        $event_rows = $event_rows + $events;
      }

      $contributions = _osa_get_contributions($person['contact_id']);
      if (isset($contributions)) {
        $contribute_rows = $contribute_rows + $contributions;
      }
    }
  }

  // (re)assign Smarty varibles
  $page->assign('parents', $parents);
  $page->assign('students', $students);
  $page->assign('teachers', $teachers);
  $page->assign('others', $others);
  $page->assign('members', $members);
  $page->assign('event_rows', $event_rows);
  $page->assign('contribute_rows', $contribute_rows);
  $page->assign('groupIn', $groupIn);
  
  // use the CiviCRM Profiles to display tombstone info
  require_once 'CRM/Profile/Page/Dynamic.php';
  $gids = array('Phone and Email' => OSA_PROFILE_PHONE_EMAIL, 'Address' => OSA_PROFILE_ADDRESS);
  $profileElements = array();
  foreach ($gids as $title => $gid) {
    $profile = new CRM_Profile_Page_Dynamic($cid, $gid, 'Profile');
    $profileElements[] =
      array(
        'cid'   => $cid,
        'gid'   => $gid,
        'title' => $title,
        'html'  => $profile->run(),
    );
  }
  $page->assign('profileElements', $profileElements);

  // override the templates used for each dashboard element
  $dashboardElements = array();
  if (isset($hid)) {
    $dashboardElements[] = array(
      'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Family.tpl',
      'sectionTitle' => 'Family Members',
      'sectionState' => 'open'
    );
    $dashboardElements[] = array(
      'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Relationship.tpl',
      'sectionTitle' => 'Other Related Individual(s)',
      'sectionState' => empty($others) ? 'closed' : 'open',
    );
  }

  $dashboardElements[] = array(
    'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Membership.tpl',
    'sectionTitle' => 'Membership Registration(s)',
    'sectionState' => empty($members) ? 'closed' : 'open',
  );
  $dashboardElements[] = array(
    'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Teacher.tpl',
    'sectionTitle' => 'Teacher Registration(s)',
    'sectionState' => empty($teachers) ? 'closed' : 'open',
  );
  $dashboardElements[] = array(
    'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Event.tpl',
    'sectionTitle' => 'Event Registration(s)',
    'sectionState' => empty($event_rows) ? 'closed' : 'open',
  );
  $dashboardElements[] = array(
    'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Contribute.tpl',
    'sectionTitle' => 'Recent Payment(s)',
    'sectionState' => empty($contribute_rows) ? 'closed' : 'open',
  );
  $dashboardElements[] = array(
    'templatePath' => 'CRM/Contact/Page/View/UserDashBoard/Group.tpl',
    'sectionTitle' => 'Mailing List(s)',
    'sectionState' => empty($groupIn) ? 'closed' : 'open',
  );
  $page->assign('dashboardElements', $dashboardElements);

  // override the page title (do this last as generating profiles also ovrides the title)
  CRM_Utils_System::setTitle($pgContact['display_name'], $pgContact['image_URL'] . ' ' . $pgContact['display_name']);
}

/**
 * Add custom procesing when generating CiviCRM Profile based forms
 *
 * @param $form
 *   The CRM_Profile_Form_Edit object being rendered.
 */
function osa_civicrm_buildForm_CRM_Profile_Form_Edit(&$form) {
  
  $gid = $form->getVar('_gid');
  $mode = $form->getVar('_mode');
  
  // if we are passed a household id add it to the form
  $hid = CRM_Utils_Request::retrieve('osa_hid', 'Positive');
  if (isset($hid)) {
    _osa_addField($form, 'hidden', 'osa_hid', $hid);
  }
  
  // set defaults in the create family profile form
  if ($gid == OSA_PROFILE_FAMILY) {
    if ($mode == CRM_Profile_Form::MODE_CREATE) {
      $contact = _osa_get_contact();
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
  // add a relationship type selection to the Related Individual profile form
  elseif ($gid == OSA_PROFILE_REL_INDIVIDUAL) {
    if ($mode == CRM_Profile_Form::MODE_CREATE) {
      _osa_addRelationshipField($form, $hid);
    }
  }

  // add CSS and javascript to profile forms in a colorbox
  $context = CRM_Utils_Request::retrieve('context', 'String');
  if ($context == 'boxload') {
    $smarty = CRM_Core_Smarty::singleton();
    $smarty->display('CRM/common/jquery.tpl');
    $smarty->display('CRM/common/commonCSS.tpl');
    $smarty->display('osa/boxload.tpl');
  }
}

/**
 * Add custom post procesing to CiviCRM Profile based forms
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

    // if we are passed a household id, add the new contact to the house
    if (isset($hid)) {
      $type = OSA_REL_HOUSEHOLD_MEMBER;
      if ($gid == OSA_PROFILE_REL_INDIVIDUAL) {
        $type = _osa_getRelationshipField($form);
      }

      $contact     = _osa_get_contact($cid, FALSE);
      $contact_type = isset($contact['contact_sub_type']) ? $contact['contact_sub_type'][0] : $contact['contact_type'];
      $perm        = ($contact_type == 'Parent') ? OSA_PERM_OWNED : OSA_PERM_NONE;
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
        $houseAddressId = $result['id'];
        $sharedAddress = $result['values'][$houseAddressId];
        $sharedAddress['master_id'] = $houseAddressId;
        $sharedAddress['use_shared_address'] = TRUE;
        $sharedAddress['contact_id'] = $cid;
        $sharedAddress['version'] = 3;

        $result = civicrm_api('address', 'get', array('contact_id' => $cid, 'location_type_id' => 1, 'version' => 3));
        if ($result['count'] > 0) {
          $result = civicrm_api('address', 'delete', array('id' => $result['id'], 'version' => 3));
        }

        unset($sharedAddress['id']);
        $result = civicrm_api('address', 'create', $sharedAddress);
      }
    }
  }
}

/**
 * Add custom procesing when generating CiviCRM Contribution based forms
 *
 * @param $form
 *   The CRM_Contribute_Form_Contribution_Main object being rendered.
 */
function osa_civicrm_buildForm_CRM_Contribute_Form_Contribution_Main(&$form) {

  // all of our contributions are from authenticated users,
  // so remove the email for anonymous users
  $bltID = $form->_bltID;
  $form->removeElement("email-$bltID");
  
  // check if this is the membership form (we only use one for memberships)
  if ($form->_id == OSA_CONTRIB_MEMBER) {
    return;
  }

  // all other contributions can be made on behalf of a family member
  // so add a drop down of family members
  $family = _osa_getHouseholdMembers(NULL, 'display_name');
  asort($family);
  $family = array( ' - select - ' ) + $family;
  $form->addElement('select', 'contact_id', ts('Family Member'), $family);
  $form->addRule('contact_id', ts('Please select a family member.'), 'required');
  $session = CRM_Core_Session::singleton();
  $form->setDefaults(array('contact_id' => $session->get('userID')));
  $form->assign('onbehalfFamily', TRUE);

  // custom  processing for the teacher registration form
  if ($form->_id == OSA_CONTRIB_TEACHER) {
    // populate the teacher QE Park pricing
    _osa_get_teacher_lists( $contacts, $select_options, $prices30min, $prices45min, $prices60min);
    $form->assign('price30', json_encode($prices30min));
    $form->assign('price45', json_encode($prices45min));
    $form->assign('price60', json_encode($prices60min));
    
    // @TODO not sure why this is needed, but form validation fails wothout it
    $form->addElement('hidden', 'amount_other', '0.01');
  }

  // if using the direct payment, remove the unused fields
  if ($form->_paymentProcessor['payment_processor_type'] != 'drupalcommerce') {
    $remove_fields = array(
      'billing_first_name',
      'billing_middle_name',
      'billing_last_name',
      "billing_street_address-$bltID",
      "billing_city-$bltID",
      "billing_state_province_id-$bltID",
      "billing_postal_code-$bltID",
      "billing_country_id-$bltID",
      'cvv2',
    );
    foreach ($remove_fields as $field) {
      unset($form->_fields[$field]);
      $form->removeElement($field);
    }

    $group = $form->getElement('buttons');
    $buttons = $group->getElements();
    $buttons[0]->setValue(ts('Confirm') . ' >>');
  }
}

/**
 * Add custom validation to CiviCRM Contribution based forms
 *
 */
function osa_civicrm_validate_CRM_Contribute_Form_Contribution_Main(&$fields, &$files, &$form) {
  $errors = array();
  
  // custom processing for the teacher registration form
  if ($form->_id == OSA_CONTRIB_TEACHER) {
    $values = $form->_submitValues;

    // check to make sure the selected teacher teaches at the selected location
    if (isset($values['custom_24']) && isset($values['custom_18']) && ($values['custom_18'] == 'QE Park')) {
      $teacher = _osa_get_contact($values['custom_24']);
      if ($teacher['uses_qep'] == '0') {
        $errors['custom_18'] = ts('%1 does not teach private lessons at QE Park', array( 1 => $teacher['display_name'] ));
        $form->setElementError('custom_18', $errors['custom_18']);
      }
    }
  }
  
  return empty($errors) ? true : $errors;
}

/**
 * Add custom post procesing to CiviCRM Contribution based forms
 *
 * @param $form
 *   The CRM_Contribute_Form_Contribution_Main object being processed.
 */
function osa_civicrm_postProcess_CRM_Contribute_Form_Contribution_Main(&$form) {

  // check if we are using the Drupal Commerce Cart
  if ($form->_paymentProcessor['payment_processor_type'] == 'drupalcommerce') {
    // since we use a Cart, we can disable the confirmation forms
    $confirmForm =& $form->controller->_pages['Confirm'];
    $confirmForm->preProcess();

    // Hack to tell form validator that confirmation page is valid
    $data =& $form->controller->container();
    $data['valid']['Confirm'] = 1;

    // Redirect to thank you page
    $confirmForm->postProcess();
    $qfKey = $form->controller->_key;
    CRM_Utils_System::redirect(CRM_Utils_System::url('civicrm/contribute/transact', "_qf_ThankYou_display=1&qfKey=$qfKey", TRUE, NULL, FALSE));
  }
}

/**
 * Add custom procesing when generating CiviCRM Event Registration forms
 *
 * @param $form
 *   The CRM_Event_Form_Registration_Register object being rendered.
 */
function osa_civicrm_buildForm_CRM_Event_Form_Registration_Register(&$form) {
  // remove the email field to identify participant
  $bltID = $form->_bltID;
  $form->removeElement("email-$bltID");
  
  // registrations can be made on behalf of a family member
  // so add a drop down of family members
  $family = _osa_getHouseholdMembers(NULL, 'display_name');
  asort($family);
  $family = array(' - select - ') + $family;
  $form->addElement('select', 'contact_id', ts('Family Member'), $family);
  $form->addRule('contact_id', ts('Please select a family member.'), 'required');
  $session = CRM_Core_Session::singleton();
  $form->setDefaults(array('contact_id' => $session->get('userID')));
  $form->assign('onbehalfFamily', TRUE);
}

/**
 * Add custom post procesing to CiviCRM Event Registration forms
 *
 * @param $form
 *   The CRM_Event_Form_Registration_Register object being processed.
 */
function osa_civicrm_postProcess_CRM_Event_Form_Registration_Register(&$form) {

  // check if we are using the Drupal Commerce Cart
  if ($form->_paymentProcessor['payment_processor_type'] == 'drupalcommerce') {
    // since we use a Cart, we want to disable the confirmation form
    $confirmForm =& $form->controller->_pages['Confirm'];

    // first set the contact id to the selected family member
    $data =& $form->controller->container();
    $contact_id = $data['values']['Register']['contact_id'];
    $params = $confirmForm->get('params');
    $params[0]['contact_id'] = $contact_id;
    $confirmForm->set('params', $params);;

    // build the confirmation form to make sure all of the values are set
    $confirmForm->preProcess();
    $confirmForm->buildQuickForm();

    // tell form validator that confirmation page is valid
    $data['valid']['Confirm'] = 1;
    $confirmForm->postProcess();

    // Redirect to thank you page
    $qfKey = $form->controller->_key;
    CRM_Utils_System::redirect(CRM_Utils_System::url('civicrm/event/register', "_qf_ThankYou_display=true&qfKey=$qfKey", TRUE, NULL, FALSE));
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
    if (isset($values['price_6']) && isset($values['custom_24']) && isset($values['custom_18']) && ($values['custom_18'] == 'QE Park')) {
      // set the amounts for each lesson length based on the teacher
      $teacher = _osa_get_contact($values['custom_24']);
      $amount[6]['options'][11]['amount'] = $teacher['annual_fee_30'];
      $amount[6]['options'][12]['amount'] = $teacher['annual_fee_45'];
      $amount[6]['options'][13]['amount'] = $teacher['annual_fee_60'];
    }
  }
}

/**
 * Implements hook_civicrm_customFieldOptions() to dynamically populate custom selection boxes
 */
function osa_civicrm_customFieldOptions($fieldID, &$options, $detailedFormat = false ) {

  // teachers selection
  if ($fieldID == 24) {
    // populate the teacher list
    _osa_get_teacher_lists( $contacts, $select_options, $prices30min, $prices45min, $prices60min);
    if ($detailedFormat) {
      foreach ($select_options as $value => $label) {
        $options['teacher_' . $value] = array(
          'id'    => 'teacher_' . $value,
          'value' => $value,
          'label' => $label,
        );
      }
    }
    else {
      $options += $select_options;
    }
  }
}

/**
 * Add custom procesing when generating CiviCRM Email forms
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
