<?php

/**
 * @file
 * Miscellaneous functions for The Oakville Suzuki Association customizations.
 */

/**
 * Define constants for CiviCRM Profiles.
 */
define('OSA_PROFILE_ORG', 5);
define('OSA_PROFILE_FAMILY', 10);
define('OSA_PROFILE_PHONE_EMAIL', 11);
define('OSA_PROFILE_ADDRESS', 12);
define('OSA_PROFILE_PARENT', 13);
define('OSA_PROFILE_STUDENT', 14);
define('OSA_PROFILE_REL_INDIVIDUAL', 2);

/**
 * Define constants for CiviCRM Contribution Pages.
 */
define('OSA_CONTRIB_MEMBER', 1);
define('OSA_CONTRIB_TEACHER', 2);

/**
 * Define constants for CiviCRM Relationship Types.
 */
define('OSA_REL_PARENT', 1);
define('OSA_REL_SPOUSE', 2);
define('OSA_REL_SIBLING', 3);
define('OSA_REL_HEAD_HOUSEHOLD', 6);
define('OSA_REL_HOUSEHOLD_MEMBER', 7);
define('OSA_REL_OTHER', 11);
define('OSA_REL_TEACHER', 12);

/**
 * Define constants for CiviCRM Relationship Permissions.
 */
define('OSA_PERM_NONE',  0);
define('OSA_PERM_OWNER', 1);
define('OSA_PERM_OWNED', 2);
define('OSA_PERM_PEER',  3);

/**
 * Define constants for CiviCRM Membership Types.
 */
define('OSA_MEMBER_STUDENT', 1);

/**
 * Define constants for CiviCRM Event Types.
 */
define('OSA_EVENT_GROUP_CLASS_MASTER', '8');
define('OSA_EVENT_GROUP_CLASS_CHILD', '9');

/**
 * Map name to CiviCRM Profile Ids
 *
 * @param string $entity
 *   The name of the entity / contact type.
 * @return integer
 *   Default profile id.
 */
function _osa_profileId($entity) {
  static $_gids = NULL;
  
  if (!$_gids) {
    $_gids = array(
      'Family'       => OSA_PROFILE_FAMILY,
      'Parent'       => OSA_PROFILE_PARENT,
      'Student'      => OSA_PROFILE_STUDENT,
      'Individual'   => OSA_PROFILE_REL_INDIVIDUAL,
      'Organization' => OSA_PROFILE_ORG,
      'Household'    => OSA_PROFILE_FAMILY,
   );
  }
  
  return $_gids[$entity];
}

/**
 * Map name to CiviCRM Custom Data fields
 */
function _osa_field($field) {

  static $custom_fields = array();
  
  if (empty($custom_fields)) {
    $custom_fields['member_discount'] = 'custom_7_1';
    $custom_fields['lesson_location'] = 'custom_18';
    $custom_fields['selected_teacher'] = 'custom_24';
    $custom_fields['group_class_nid'] = 'custom_25';
    $custom_fields['members_only'] = 'custom_26';
    $custom_fields['lesson_fee_level'] = 'custom_27';
    $custom_fields['lesson_level'] = 'custom_32';
    $custom_fields['additional_data'] = 'custom_37';
    $custom_fields['lesson_length'] = 'price_6';
  }

  return isset($custom_fields[$field]) ? $custom_fields[$field]: NULL;
}

/**
 * Household Relationship Clean Up
 *
 * OSA is based on 'Family Profiles'. These are managed by a Household contact
 * and multiple relationships between Parents, Students and other Individuals.
 * This routine goes through the family and does its best to ensure all of the
 * relationships exist and make sense.
 *
 * @param integer $cid
 *   The contact id who's house we want to clean.
 */
function _osa_manageHouseholdRelationships($cid) {

  // find the household
  $hid = _osa_getHousehold($cid);
  if (!isset($hid)) {
    return;
  }

  // get household members and divide by type
  $family = _osa_getHouseholdMembers($hid);
  $parents = array();
  $students = array();
  $others = array();

  foreach ($family as $person) {
    $contact = $person['contact'];
    $contact_type = isset($contact['contact_sub_type']) ? $contact['contact_sub_type'][0] : $contact['contact_type'];

    if ($contact_type == 'Parent') {
      $parents[] = $contact;
    }
    elseif ($contact_type == 'Student') {
      $students[] = $contact;
    }
    elseif ($contact_type == 'Individual') {
      $others[] = $contact;
    }
  }
  
  // make sure relationships exist between parents and parents (spousal), students (child), and others (other)
  foreach ($parents as $parent) {
    $result = civicrm_api('relationship', 'get', array('contact_id' => $parent['contact_id'], 'version' => 3));
    $relationships = $result['values'];

    //  make sure relationships exist between parents
    foreach ($parents as $spouse) {
      if ($parent['contact_id'] == $spouse['contact_id']) {
        continue;
      }
      $found = FALSE;
      foreach ($relationships as $relationship) {
        if (($spouse['contact_id'] == $relationship['contact_id_a']) ||
             ($spouse['contact_id'] == $relationship['contact_id_b'])) {
          $found = TRUE;
          break;
        }
      }
      if ($found) {
        continue;
      }

      // missing spousal relationship
      _osa_create_relationship(OSA_REL_SPOUSE, $spouse['contact_id'], $parent['contact_id'], OSA_PERM_PEER);
    }
  
    // make sure relationships exist between parents and students
    foreach ($students as $student) {
      $found = FALSE;
      foreach ($relationships as $relationship) {
        if (($student['contact_id'] == $relationship['contact_id_a']) ||
             ($student['contact_id'] == $relationship['contact_id_b'])) {
          $found = TRUE;
          break;
        }
      }
      if ($found) {
        continue;
      }

      // missing parent/child relationship
      _osa_create_relationship(OSA_REL_PARENT, $student['contact_id'], $parent['contact_id'], OSA_PERM_OWNER);
    }

    // make sure relationships exist between parents and others (used for permissions)
    foreach ($others as $other) {
      $found = FALSE;
      foreach ($relationships as $relationship) {
        if (($other['contact_id'] == $relationship['contact_id_a']) ||
             ($other['contact_id'] == $relationship['contact_id_b'])) {
          $found = TRUE;
          break;
        }
      }
      if ($found) {
        continue;
      }

      // missing parent/other relationship
      _osa_create_relationship(OSA_REL_OTHER, $other['contact_id'], $parent['contact_id'], OSA_PERM_OWNER);
    }
  }

  // make sure relationships exist between students
  foreach ($students as $student) {
    $result = civicrm_api('relationship', 'get', array('contact_id' => $student['contact_id'], 'version' => 3));
    $relationships = $result['values'];

    // make sure relationships exist between siblings
    foreach ($students as $sibling) {
      if ($student['contact_id'] == $sibling['contact_id']) {
        continue;
      }
      $found = FALSE;
      foreach ($relationships as $relationship) {
        if (($sibling['contact_id'] == $relationship['contact_id_a']) ||
             ($sibling['contact_id'] == $relationship['contact_id_b'])) {
          $found = TRUE;
          break;
        }
      }
      if ($found) {
        continue;
      }

      // missing sibling relationship
      _osa_create_relationship(OSA_REL_SIBLING, $sibling['contact_id'], $student['contact_id'], OSA_PERM_NONE);
    }
  }

  // merge duplicate misc. contacts
  foreach ($others as $other) {
    foreach ($others as $dup) {
      if ($other['contact_id'] >= $dup['contact_id']) {
        continue;
      }
    
      require_once "CRM/Dedupe/Merger.php";
      if (($other['first_name'] == $dup['first_name']) &&
           ($other['last_name']  == $dup['last_name']))  {
        CRM_Dedupe_Merger::moveContactBelongings($other['contact_id'], $dup['contact_id']);
        civicrm_api('contact', 'delete', array('contact_id' => $dup['contact_id'], 'version' => 3));
      }
    }
  }
}

/**
 *  Get the id of a Household Contact
 */
function _osa_getHousehold($cid = NULL) {

  // get the contact record
  $contact = _osa_get_contact($cid, FALSE);
  if (!isset($contact)) {
    return NULL;
  }
  
  // find the household
  $hid = NULL;
  if ($contact['contact_type'] == 'Household') {
    $hid = $cid;
  }
  else {
    $result = civicrm_api('relationship', 'get', array('contact_id' => $cid, 'version' => 3));
    $relationships = $result['values'];
    foreach ($relationships as $relationship) {
      if (($relationship['relationship_type_id'] == OSA_REL_HEAD_HOUSEHOLD) ||
          ($relationship['relationship_type_id'] == OSA_REL_HOUSEHOLD_MEMBER)) {
        $hid = $relationship['cid'];
        break;
      }
    }
  }
  
  return $hid;
}

/**
 *  Get the all contacts related to a Household Contact
 */
function _osa_getHouseholdMembers($cid = NULL, $field = NULL) {
  $family = array();

  // default to logged in user
  if (!isset($cid)) {
    $cid = _osa_get_contact_id();
  }

  // get the household
  $hid = _osa_getHousehold($cid);
  if (!isset($hid)) {
    $hid = $cid;
  }
  
  // should never happen this should only be available to authenticated users
  if (!isset($hid)) {
    CRM_Core_Error::fatal(ts("An unexpected error has occurred. Please contact the OSA and report the error."));
  }
  
  // get all relationships for the household/contact
  $result = civicrm_api('relationship', 'get', array('contact_id' => $hid, 'version' => 3));
  $relationships = $result['values'];

  // get the contact records
  foreach ($relationships as $relationship) {
    $contact = _osa_get_contact($relationship['cid'], FALSE);
    $contact_id = $contact['contact_id'];
    
    if (isset($field)) {
      $family[$contact_id] = $contact[$field];
    }
    else {

      // it's possible to have more than one relationship with the household
      // e.g. sometimes civi creates a household member for the head of the household
      // we want to perserve head of houshold relationships
      if (isset($family[$contact_id])) {
        if ($family[$contact_id]['relationship']['relationship_type_id'] == OSA_REL_HEAD_HOUSEHOLD) {
          continue;
        }
      }
      
      $contact_type = isset($contact['contact_sub_type']) ? $contact['contact_sub_type'][0] : $contact['contact_type'];
      $person = array(
        'contact_id'   => $contact_id,
        'contact_type' => $contact_type,
        'contact'      => $contact,
        'relationship' => $relationship,
        'links'        => _osa_related_links($contact_type, $contact_id, $relationship['id'])
      );
      $family[$contact_id] = $person;
    }
  }

  // if there are no other family members, and we just want a list, at least return the given id.
  if (empty($family) && isset($field)) {
    $contact = _osa_get_contact($cid, FALSE);
    $family[$cid] = $contact[$field];
  }

  return $family;
}

/**
 *  Get the teachers related to a student
 */
function _osa_getTeachers(&$student) {
  $teachers = array();

  $contact_type = isset($student['contact_sub_type']) ? $student['contact_sub_type'][0] : $student['contact_type'];
  if ($contact_type != 'Student') {
    return $teachers;
  }
  // get relationships for the student
  $result = civicrm_api('relationship', 'get', array('contact_id' => $student['id'], 'version' => 3));
  $relationships = $result['values'];

  // find the teacher relationships
  foreach ($relationships as $relationship) {
    if ($relationship['relationship_type_id'] == OSA_REL_TEACHER) {
      $teachers[$relationship['id']] = $relationship;
      $teachers[$relationship['id']]['student'] = $student;

      $today = new DateTimeEx('today');
      $end_date = new DateTimeEx($relationship['end_date']);
      
      if ($end_date < $today) {
        $teachers[$relationship['id']]['status'] = ts('Expired');
      }
      else {
        $teachers[$relationship['id']]['status'] = ($relationship['is_active']) ? ts('Active') : ts('Inactive');
      }
    }
  }

  return $teachers;
}

/**
 * Return links for related contacts.
 *
 * @param string $contact_type
 *   Type of the related contact.
 * @param integer $cid
 *   Id of the related contact.
 * @param integer $rid
 *   Relationship Id of the contact to household
 * @return
 *   links accociated to the given contact.
 */
function _osa_related_links($contact_type, $cid, $rid) {
  $links = array();

  if ($contact_type == 'Student') {
    $links['member'] = array(
      'name'  => ts('Membership'),
      'url'   => 'civicrm/contribute/transact',
      'qs'    => 'reset=1&id=' . OSA_CONTRIB_MEMBER . "&cid=$cid",
      'title' => ts("Register/Renew Membership")
    );
    $links['group'] = array(
      'name'  => ts('Group'),
      'url'   => 'form/group-class-selection',
      'qs'    => "cid=$cid",
      'title' => ts("Register for Group Class")
    );
    $links['event'] = array(
      'name'  => ts('Event'),
      'url'   => CRM_Utils_System::absoluteURL("events/$cid"),
      'qs'    => '',
      'title' => ts("Register for an Event")
    );
    $links['teacher'] = array(
      'name'  => ts('Teacher'),
      'url'   => 'civicrm/contribute/transact',
      'qs'    => 'reset=1&id=' . OSA_CONTRIB_TEACHER . "&cid=$cid",
      'title' => ts("Register with a Teacher")
    );
  }
  
  $links += array(
    CRM_Core_Action::UPDATE => array(
      'name'  => ts('Edit'),
      'url'   => 'civicrm/profile/edit',
      'qs'  => 'reset=1&snippet=1&context=boxload&gid=' . _osa_profileId($contact_type) . "&id=$cid",
      'class' => 'box-load',
      'title' => ts("Edit $contact_type")
    ),
    CRM_Core_Action::DELETE => array(
      'name'  => ts('Delete'),
      'url'   => 'civicrm/contact/view/rel',
      'qs'  => "action=delete&reset=1&cid=$cid&id=$rid",
      'extra' => 'onclick = "if (confirm(\'Are you sure you want to delete this relationship?\')) this.href+=\'&amp;confirmed=1\'; else return FALSE;"',
      'title' => ts("Delete $contact_type")
    )
  );

  return CRM_Core_Action::formLink($links, NULL, array(), 'More');
}

/**
 * Add a field to a CiviCRM Profile Form.
 *
 * @param object $form
 *   The CRM_Profile_Form_Edit object being modified.
 * @param integer $type
 *   The type field to add.
 * @param integer $name
 *   The name of the field.
 * @param integer $value
 *   The default value of the field.
 * @param integer $label
 *   Optional label to display.
 * @param boolean $required
 *   Add required validation rule.
 * @param integer $position
 *   The position of the field in the form. Defaults to the bottom.
 * @param integer $options
 *   Optional list for selection type fields.
 */
function _osa_addField(&$form, $type, $name, $value, $label = NULL, $required = FALSE, $position = 0, $options = NULL ) {

  // get the profile id & title
  $gid = $form->getVar('_gid');
  $uf_group = new CRM_Core_DAO_UFGroup();
  $uf_group->id = $gid;
  if (!$uf_group->find(TRUE)) {
    CRM_Core_Error::fatal();
  }
  
  // create the field used by the CRM/Profile/Form/Dynamic.tpl template
  $field = array(
    $field_name => array(
      'name' => $name,
      'group_id' => $gid,
      'groupTitle' => $uf_group->title,
      'title' => '',
    )
  );
  
  if ($position <= 0) {
    $form->_fields[$name] = $field;
  }

  // add an element to the QuickForm
  $form->addElement($type, $name, $value);

  // add a rule for required fields
  if ($required) {
    $form->addRule($name, ts('%1 is a required field.', array(1 => $label)), 'required');
  }
}

/**
 * Add a Relationship Type select field to the top of a CiviCRM Profile Form
 *
 * @param object $form
 *   The CRM_Profile_Form_Edit object being modified.
 * @param integer $cid
 *   The contact id being related to.
 * @param boolean $required
 *   Add required validation rule.
 */
function _osa_addRelationshipField(&$form, $cid, $required = TRUE) {

  // get the profile id & title
  $gid = $form->getVar('_gid');
  $uf_group = new CRM_Core_DAO_UFGroup();
  $uf_group->id = $gid;
  if (! $uf_group->find(TRUE)) {
    CRM_Core_Error::fatal();
  }
  
  // create the field used by the CRM/Profile/Form/Dynamic.tpl template
  $label = ts('Relationship Type');
  $field_name = 'rel_id';
  $field = array(
    $field_name => array(
      'name' => $field_name,
      'group_id' => $gid,
      'groupTitle' => $uf_group->title,
      'title' => $label,
    )
  );
    
  // add it to the begining of the list (display first)
  $form->_fields = $field + $form->_fields;

  // get the valid relationship types for this contact
  $r_types = CRM_Contact_BAO_Relationship::getContactRelationshipType($cid);
  $hh_val  = OSA_REL_HEAD_HOUSEHOLD . '_b_a';
  unset($r_types[$hh_val]); // head of household is reserved for user who created it
  $options = array('' => ts('- select -')) + $r_types; // include a 'none' option

  // add an element to the QuickForm
  $form->add('select', $field_name, $label, $options, '', $required);
  if ($required) {
    $form->addRule($field_name, ts('%1 is a required field.', array(1 => $label)), 'required');
  }
}

/**
 * Get the Relationship Type field value from a CiviCRM Profile Form
 *
 * @param object $form
 *   The CRM_Profile_Form_Edit object being modified.
 */
function _osa_getRelationshipField(&$form) {
  
  $field_name = 'rel_id';
  $sel_val = CRM_Utils_Array::value($field_name, $_POST);
  if (!isset($sel_val)) {
    return NULL;
  }
  
  // relationship types are in the form 999_b_a
  $tmp = explode('_', $sel_val);
  return $tmp[0];
}

/**
 * Add a Family Member selection field to forms
 *
 * @param object $form
 *   The Form object being modified.
 * @param boolean $members_only
 *   Only include family members that are OSA members.
 */
function _osa_addFamilyField(&$form, $members_only = FALSE) {

  $family = _osa_getHouseholdMembers(NULL, 'display_name');

  if ($members_only) {
    $form->assign('members_only', TRUE);
    $cids = array_keys($family);
    foreach ($cids as $contact_id) {
      $membership = _osa_get_membership($contact_id);
      $is_member = isset($membership) ? $membership['is_current_member'] : FALSE;
      if (!$is_member) {
        unset($family[$contact_id]);
      }
    }
    if (empty($family)) {
      CRM_Core_Error::statusBounce(ts('Registration for <b>%1</b> is only open to OSA Members.', array(1 => $form->_values['event']['title'])), $_SERVER['HTTP_REFERER']);
    }
  }

  asort($family);
  $family = array( ' - select - ' ) + $family;
  $form->addElement('select', 'contact_id', ts('Family Member'), $family);
  $cid = CRM_Utils_Request::retrieve('cid', 'Positive');
  if (!isset($cid)) {
    $session = CRM_Core_Session::singleton();
    $cid = $session->get('userID');
  }
  $form->setDefaults(array('contact_id' => $cid));
  $form->addRule('contact_id', ts('Family Member is a required field.'), 'required');
  $form->assign('onbehalfFamily', TRUE);
}

/**
 * Wrapper function to return current logged in user's drupal id
 */
function _osa_getUserId() {
  return $GLOBALS['user']->uid;
}

/**
 * Wrapper function to return current logged in user's contact id
 */
function _osa_get_contact_id() {
  $params = array(
    'uf_id' => _osa_getUserId(),
    'sequential' => 1,
    'version' => 3,
  );

  $result = civicrm_api('UFMatch', 'get', $params);
  return $result['values'][0]['contact_id'];
}

/**
 * Wrapper function to return a contact record
 */
function _osa_get_contact($cid = NULL, $custom = TRUE) {

  if (!isset($cid)) {
    $cid = _osa_get_contact_id();
  }

  // get the contact record
  $result = civicrm_api('contact', 'get',  array('id' => $cid, 'version' => 3));
  if (($result['is_error'] == 0) && ($result['count'] == 1)) {
    $contact = $result['values'][$cid];
  }
  else {
    return NULL;
  }

  // fill in any custom fields
  if ($custom) {
    static $custom_fields = NULL;
    if (!isset($custom_fields)) {
      $result = civicrm_api('CustomField', 'get',  array('version' => 3));
      $custom_fields = $result['values'];
    }
    
    $result = civicrm_api('CustomValue', 'get',  array('entity_id' => $cid, 'entity_type' => 'Contact', 'version' => 3));
    if ($result['is_error'] == 0) {
      foreach ($result['values'] as $fid => $fattribs) {
        $fname = $custom_fields[$fid]['name'];
        $ignore = array('entity_id' => 0, 'latest' => 0, 'id' => 0,);
        $values = array_diff_key($fattribs, $ignore);
        if (isset($values[0])) {
          $contact[$fname] = $values[0];
        }
        else {
          $contact[$fname] = array();
          foreach ($values as $vid => $value) {
            $contact[$fname][$vid] = $value;
          }
        }
      }
    }
  }

  return $contact;
}

/**
 * Wrapper of CiviCRM API to create relationships
 *
 * @param integer $relationship_type_id
 *   relationship type
 * @param integer $contact_id_a
 *   contact a
 * @param integer $contact_id_b
 *   contact b
 * @param integer $permission
 *   defines permissions from b_a
 */
function _osa_create_relationship($relationship_type_id, $contact_id_a, $contact_id_b, $permission = OSA_PERM_OWNER) {

  if ($contact_id_a == 1 || $contact_id_b == 1) {
    watchdog('ahh', "relationship with admin?? relationship_type_id=$relationship_type_id, contact_id_a=$contact_id_a, contact_id_b=$contact_id_b");
    watchdog('ahh', '<pre>' . print_r(debug_backtrace(), TRUE) . '</pre>');
    return;
  }
  
  // build the relationship entity parameters
  $is_permission_a_b = (($permission == OSA_PERM_OWNED) || ($permission == OSA_PERM_PEER)) ? 1 : 0;
  $is_permission_b_a = (($permission == OSA_PERM_NONE) || ($permission == OSA_PERM_OWNED)) ? 0 : 1;

  $params = array(
    'relationship_type_id' => $relationship_type_id,
    'contact_id_a'         => $contact_id_a,
    'contact_id_b'         => $contact_id_b,
    'is_active'            => 1,
    'start_date'           => date("Ymd", time()),
    'end_date'             => NULL,
    'is_permission_a_b'    => $is_permission_a_b,
    'is_permission_b_a'    => $is_permission_b_a,
    'version'              => '3',
  );

  // Call the CiviCRM API to create the relationship
  $result = civicrm_api('relationship', 'create', $params);
  if ($result['is_error']) {
    watchdog('osa', 'Error: %1\nfailed to create relationship type_id=%2 between cid_a=%3 and cid_b=%4', 
             array(1 => $result['error_message'], 2 => $relationship_type_id, 3 => $contact_id_a, 4 => $contact_id_b),
             WATCHDOG_ERROR);
    CRM_Core_Error::fatal();
  }
}

/**
 * Wrapper of CiviCRM API to get OSA membership
 *
 * @param integer $contact_id
 *   contact who's membership status to get
 */
function _osa_get_membership($contact_id, $membership_type_id = OSA_MEMBER_STUDENT) {
  // build the  parameters
  $params = array(
    'contact_id' => $contact_id,
    'version'    => '3',
 );
  
  // Call the CiviCRM API to get the membership
  $result = civicrm_api('membership', 'get', $params);
  foreach ($result['values'] as $values) {
    if ($values['membership_type_id'] == $membership_type_id) {
      $membership = $values;
      break;
    }
  }
  
  if (!isset($membership)) {
    return NULL;
  }
  
  // FIX: api doesn't return everything
  $result = CRM_Member_BAO_Membership::getStatusANDTypeValues($membership['id']);
  $result = $result[ $membership['id'] ];
  $membership['status'] = $result['status'];
  $membership['is_current_member'] = $result['is_current_member'];

  $renew_page_id = CRM_Member_BAO_Membership::getContributionPageId($membership['id']);
  if (!$renew_page_id) {
    require_once 'CRM/Core/BAO/Setting.php';
    $renew_page_id = CRM_Core_BAO_Setting::getItem(CRM_Core_BAO_Setting::MEMBER_PREFERENCES_NAME, 'default_renewal_contribution_page');
  }
  $membership['renewPageId'] = $renew_page_id ;

  return $membership;
}

/**
 * Wrapper of CiviCRM API to get contributions
 *
 * @param integer $contact_id
 *   contact who's contributions to get
 */
function _osa_get_contributions($contact_id) {
  // build the  parameters
  $params = array(
    'contact_id' => $contact_id,
    'option.sort' => 'receive_date desc',
    'rowCount' => 30,
    'version'    => '3',
 );

  // Call the CiviCRM API to get the contributions
  $result = civicrm_api('contribution', 'get', $params);
  if ($result['is_error']) {
    watchdog('osa', 'Error: %1\nfailed to get contributions for contact_id=%2', array(1 => $result['error_message'], 2 => $contact_id,), WATCHDOG_ERROR);
  }

  return $result['values'];
}

/**
 * Wrapper of CiviCRM API to get participant events
 *
 * @param integer $contact_id
 *   contact who's events we're to get
 */
function _osa_get_participant_events($contact_id) {
  // build the  parameters
  $params = array(
    'contact_id' => $contact_id,
    'version'    => '3',
 );
  
  // Call the CiviCRM API to get the events
  $result = civicrm_api('participant', 'get', $params);
  if ($result['is_error']) {
    watchdog('osa', 'Error: %1\nfailed to get events for contact_id=%2', array(1 => $result['error_message'], 2 => $contact_id,), WATCHDOG_ERROR);
  }
  
  return $result['values'];
}

/**
 * Returns a component object from a webform node
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

/**
 *  Calculate the next payment date and number of installments
 *
 * @param string $rel_start_date
 *   relative start date of the school year & first payment (ie 'first day of September')
 * @param integer $num_installments
 *   total number of installments / months in school year. Will be updated if the school year has begun.
 */
function _osa_next_payment_date($rel_start_date, &$num_installments) {

  $start_date = new DateTimeEx($rel_start_date);
  $end_date = new DateTimeEx($start_date);
  $tmp = $num_installments - 1;
  $end_date->modify("first day of +$tmp months");

  $first_month = $start_date->get('month');
  
  $today = new DateTimeEx('today');
  $current_month = $today->get('month');
  
  // adjust year to account for non-calendar school year
  if ($start_date->get('month') > 1) {
    if ($today->get('month') <= $end_date->get('month')) {
      $start_date->modify('-1 year');
      $end_date->modify('-1 year');
    }
  }

  // next payment date cannot be in the past
  if ($today > $start_date) {
    // reduce the number of installments
    $num_installments -= $today->diff($start_date)->format('%m');
    return $today;
  }
  
  return $start_date;
}

/**
 * Better DateTime class
 */
class DateTimeEx extends DateTime
{
  public static $_format = 'Y-m-d H:i:s';

  public function __toString()
  {
    return (string)parent::format(static::$_format);
  }

  public function get($component) {
    $components = date_parse($this);
    return $components[$component];
  }
}

/**
 * Find the next date from a repeating Date field
 */
function _osa_get_next_date(&$series, &$start_date, &$end_date) {
  $date = new DateTime(null, new DateTimeZone('America/Toronto'));
  $now = $date->format('Y-m-d H:i:s');

  foreach ($series as $session_date) {
    $start_date = date('Y-m-d H:i:s', strtotime($session_date['value'] . ' ' . $session_date['timezone_db']));
    $end_date = date('Y-m-d H:i:s', strtotime($session_date['value2'] . ' ' . $session_date['timezone_db']));
    if ($start_date > $now) {
      return;
    }
  }
}
