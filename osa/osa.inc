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
define('OSA_CONTRIB_DONATE', 3);
define('OSA_CONTRIB_ADJUST', 4);

/**
 * Define constants for CiviCRM Financial Types.
 */
define('OSA_FINTYPE_CONCERT_TICKET', 5);
define('OSA_FINTYPE_TEACHER_FEE', 6);

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
define('OSA_EVENT_MEETING', '4');
define('OSA_EVENT_CONCERT', '5');
define('OSA_EVENT_GROUP_CLASS_MASTER', '8');
define('OSA_EVENT_GROUP_CLASS_CHILD', '9');
define('OSA_EVENT_SUMMER_CAMP', '10');

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
    $custom_fields['primary_instrument'] = 'custom_1';
    $custom_fields['primary_teacher'] = 'custom_86';
    $custom_fields['second_instrument'] = 'custom_3';
    $custom_fields['second_teacher'] = 'custom_87';
    $custom_fields['third_instrument'] = 'custom_22';
    $custom_fields['third_teacher'] = 'custom_88';
    $custom_fields['member_discount'] = 'custom_7_1';
    $custom_fields['private_lesson_instrument'] = 'custom_14';
    $custom_fields['private_lesson_location'] = 'custom_15';
    $custom_fields['private_lesson_length'] = 'custom_16';
    $custom_fields['lesson_location'] = 'custom_18';
    $custom_fields['selected_teacher'] = 'custom_24';
    $custom_fields['group_class_nid'] = 'custom_25';
    $custom_fields['members_only'] = 'custom_26';
    $custom_fields['lesson_fee_level'] = 'custom_27';
    $custom_fields['lesson_level'] = 'custom_32';
    $custom_fields['additional_data'] = 'custom_37';
    $custom_fields['open_to_public'] = 'custom_46';
    $custom_fields['last_registration'] = 'custom_89';
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

  // this call is expensive so make sure it is only done once
  static $managed = array();
  if (isset($managed[$hid])) {
    return;
  }
  $managed[$hid] = TRUE;

  // get household members and divide by type
  $family = _osa_getHouseholdMembers($hid);
  $parents = array();
  $students = array();
  $others = array();

  foreach ($family as $person) {
    if ($person['contact']['osa_contact_type'] == 'Parent') {
      $parents[] = $person['contact'];
    }
    elseif ($person['contact']['osa_contact_type'] == 'Student') {
      $students[] = $person['contact'];
    }
    else {
      $others[] = $person['contact'];
    }
  }

  // make sure relationships exist between parents and parents (spousal), students (child), and others (other)
  foreach ($parents as $parent) {
    $relationships = _osa_get_relationships($parent['contact_id']);

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
    $relationships = _osa_get_relationships($student['contact_id']);

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

  // use static caching
  static $household_id = array();
  if (isset($household_id[$contact['id']])) {
    return $household_id[$contact['id']];
  }

  // find the household
  $household_id[$contact['id']] = NULL;
  if ($contact['contact_type'] == 'Household') {
    $household_id[$contact['id']] = $contact['id'];
  }
  else {
    $relationships = _osa_get_relationships($contact['id']);
    foreach ($relationships as $relationship) {
      if (($relationship['relationship_type_id'] == OSA_REL_HEAD_HOUSEHOLD) ||
          ($relationship['relationship_type_id'] == OSA_REL_HOUSEHOLD_MEMBER)) {
        $household_id[$contact['id']] = $relationship['cid'];
        break;
      }
    }
  }

  return $household_id[$contact['id']];
}

/**
 *  Create a default Household Contact
 */
function _osa_createHousehold($cid = NULL, $source = 'From code') {
  // get the user's contact record
  $contact = _osa_get_contact($cid, FALSE);
  $cid = $contact['id'];

  // create a household contact
  $params = array(
    'contact_type' => 'Household',
    'household_name' => $contact['last_name'] . ' Family',
    'email' => $contact['email'],
    'source' => $source,
    'primary_contact_id' => $cid,
    'version'=> 3,
  );
  $result = civicrm_api('contact', 'create', $params);
  if ($result['is_error'] <> 0) {
    watchdog('osa', "Error creating household {$params['household_name']} for cid $cid %1", array('%1' => $result['error_message']), WATCHDOG_ERROR);
    drupal_set_message('An unexpected error has occurred. Please contact the administrator', 'error');
  }
  $hid = $result['id'];

  // get the user's address
  $result = civicrm_api('address', 'get', array('contact_id' => $cid, 'location_type_id' => 1, 'is_primary' => 1, 'version' => 3));
  if ($result['is_error'] <> 0) {
    watchdog('osa', "Error getting address for cid $cid %1", array('%1' => $result['error_message']), WATCHDOG_ERROR);
    drupal_set_message('An unexpected error has occurred. Please contact the administrator', 'error');
  }

  // set the address of the household
  if ($result['count'] == 1) {
    $address = $result['values'][$result['id']];
    // delete the user's current address so we can reset it to be shared with the household
    $result = civicrm_api('address', 'delete', array('id' => $result['id'], 'version' => 3));
  }
  else {
    $address['country_id'] = 1039;
  }
  $address['contact_id'] = $hid;
  $address['version'] = 3;
  $address['location_type_id'] = 1;
  $address['is_primary'] = 1;
  unset($address['id']);
  $result = civicrm_api('address', 'create', $address);
  if ($result['is_error'] <> 0) {
    watchdog('osa', "Error creating address for hid $hid %1", array('%1' => $result['error_message']), WATCHDOG_ERROR);
    drupal_set_message('An unexpected error has occurred. Please contact the administrator', 'error');
  }

  // share the address
  $address['master_id'] = $result['id'];
  $address['use_shared_address'] = TRUE;
  $address['contact_id'] = $cid;
  $result = civicrm_api('address', 'create', $address);

  // make the relationship to the household
  _osa_create_relationship(OSA_REL_HEAD_HOUSEHOLD, $cid, $hid, OSA_PERM_OWNED);

  // update the given contact to be a Parent
  if (is_array($contact['contact_sub_type'])) {
    $contact_sub_type = $contact['contact_sub_type'];
    $contact_sub_type[] = 'Parent';
  }
  else {
    $contact_sub_type = 'Parent';
  }
  $result = civicrm_api('contact', 'create', array('id' => $cid, 'contact_sub_type' => $contact_sub_type, 'version' => 3));
}

/**
 *  Get the all contacts related to a Household Contact
 */
function _osa_getHouseholdMembers($cid = NULL, $field = NULL) {

  // default to the logged in user
  if (!isset($cid)) {
    $cid = _osa_get_contact_id();
  }

  // get the household
  $hid = _osa_getHousehold($cid);
  if (!isset($hid)) {
    $hid = $cid;
  }

  // use static caching
  static $family = array();
  $cache_key = "hid=$hid&field=$field";
  if (isset($family[$cache_key])) {
    return $family[$cache_key];
  }
  $family[$cache_key] = array();

  // should never happen as this should only be available to authenticated users
  if (!isset($hid)) {
    return $family[$cache_key];
  }

  // get all relationships for the household/contact
  $relationships = _osa_get_relationships($hid);

  // get the contact records
  foreach ($relationships as $relationship) {
    $contact = _osa_get_contact($relationship['cid'], !isset($field));
    $contact_id = $contact['contact_id'];

    if (isset($field)) {
      $family[$cache_key][$contact_id] = $contact[$field];
    }
    else {
      // it's possible to have more than one relationship with the household
      // e.g. sometimes civi creates a household member for the head of the household
      // we want to preserve head of household relationships
      if (isset($family[$cache_key][$contact_id])) {
        if ($family[$cache_key][$contact_id]['relationship']['relationship_type_id'] == OSA_REL_HEAD_HOUSEHOLD) {
          continue;
        }
      }

      $person = array(
        'contact_id'   => $contact_id,
        'contact_type' => $contact['osa_contact_type'],
        'contact'      => $contact,
        'relationship' => $relationship,
      );
      $family[$cache_key][$contact_id] = $person;
    }
  }

  // if there are no other family members, and we just want a list, at least return the given id.
  if (empty($family[$cache_key]) && isset($field)) {
    $contact = _osa_get_contact($cid, FALSE);
    $family[$cache_key][$cid] = $contact[$field];
  }

  return $family[$cache_key];
}

/**
 *  Get the head of a contact's Household
 */
function _osa_getHeadOfHousehold($cid = NULL) {
  $head_household = NULL;

  $family_members = _osa_getHouseholdMembers($cid);
  foreach ($family_members as $family_member) {
    if ($family_member['relationship']['relationship_type_id'] == OSA_REL_HEAD_HOUSEHOLD) {
      $head_household = $family_member['contact'];
    }
  }

  return $head_household;
}

/**
 *  Is the Drupal user an OSA voting member
 *  Officially a voting member is the head of household with at least one OSA Student Membership
 */
function osa_isVotingMember($uid = NULL) {
  civicrm_initialize();
  $cid = _osa_get_contact_id($uid);
  $head = _osa_getHeadOfHousehold($cid);
  if ($head['contact_id'] <> $cid) return FALSE;

  $family = _osa_getHouseholdMembers($cid, 'display_name');
  $cids = array_keys($family);

  foreach ($cids as $contact_id) {
    $membership = _osa_get_membership($contact_id);
    $is_member = isset($membership) ? $membership['is_current_member'] : FALSE;
    if ($is_member) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
 *  Return the contacts Name in Quickbooks
 */
function _osa_qb_name($cid = NULL) {
  civicrm_initialize();

  // Use the Household as the Customer in QB
  $family_id = _osa_getHousehold($cid);
  if ($family_id) {
    $family = _osa_get_contact($family_id, FALSE);
  }
  else {
    $family_id = 1;
    $family['display_name'] = 'Anonymous';
  }

  return "{$family['display_name']} <{$family_id}>";
}

/**
 *  Get the teachers related to a student
 */
function _osa_getTeachers(&$student) {

  // use static caching
  static $teachers = array();
  $cache_key = $student['id'];
  if (isset($teachers[$cache_key])) {
    return $teachers[$cache_key];
  }
  $teachers[$cache_key] = array();

  // get relationships for the student
  $relationships = _osa_get_relationships($student['id']);

  // find the teacher relationships
  foreach ($relationships as $relationship) {
    if ($relationship['relationship_type_id'] == OSA_REL_TEACHER) {

      // only show the lastest for each student/teacher relationship
      $teacher_key = "{$cache_key}:{$relationship['cid']}";
      if (isset($teachers[$cache_key][$teacher_key])) {
        if ($teachers[$cache_key][$teacher_key]['start_date'] > $relationship['start_date']) {
          continue;
        }
      }

      $relationship['school_year'] = _osa_school_year($school_start_date, $school_end_date, $relationship['end_date']);
      $relationship['lesson_instrument'] = $relationship[_osa_field('private_lesson_instrument')];
      $relationship['lesson_location'] = $relationship[_osa_field('private_lesson_location')];
      $relationship['lesson_length'] = $relationship[_osa_field('private_lesson_length')];

      if ($relationship['end_date'] < date('Y-m-d')) {
        $relationship['status'] = ts('Expired');
      }
      else {
        $relationship['status'] = ($relationship['is_active']) ? ts('Active') : ts('Inactive');
      }
      
      $teachers[$cache_key][$teacher_key] = $relationship;
      $teachers[$cache_key][$teacher_key]['student'] = $student;
    }
  }

  return $teachers[$cache_key];
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
 *   links associated to the given contact.
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
      'url'   => CRM_Utils_System::absoluteURL("group_class_selection/$cid"),
      'qs'    => '',
      'title' => ts("Register for Group Class")
    );
    $links['event'] = array(
      'name'  => ts('Event'),
      'url'   => CRM_Utils_System::absoluteURL("events/$cid"),
      'qs'    => '',
      'title' => ts("Register for an Event")
    );
/*
    $links['teacher'] = array(
      'name'  => ts('Teacher'),
      'url'   => 'civicrm/contribute/transact',
      'qs'    => 'reset=1&id=' . OSA_CONTRIB_TEACHER . "&cid=$cid",
      'title' => ts("Register with a Private Teacher")
    );
*/
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
      'url'   => '#',
      'extra' => " onclick=\"if (confirm('Are you sure you want to delete this {$contact_type}?')) { cj('html').addClass('wait'); CRM.api('relationship', 'delete', {'id': {$rid}}, {success: function(){cj('html').removeClass('wait');history.go(0);}}); } return false;\"",
      'title' => ts("Delete $contact_type")
    ),
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
    $name => array(
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

  // add it to the beginning of the list (display first)
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
  $form->assign('fields', $form->_fields);
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
      if (isset($form->_values['event']['title'])) {
        $msg = ts('Registration for <b>%1</b> is only open to <b>OSA Members</b>. Please register for membership prior to registering for this program.', array(1 => $form->_values['event']['title']));
      }
      elseif ($form->_id == OSA_CONTRIB_TEACHER) {
        $msg = ts('You must be an <b>OSA Member</b> to register with an <b>OSA Teacher</b>.');
      }
      else {
        $msg = ts('You must have a current <b>OSA Member</b> in your family to perform this action.');
      }
      drupal_set_message($msg, 'warning');
    }
  }

  asort($family);
  $family = array(0 => ' - select - ') + $family;
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
function _osa_get_contact_id($uid = NULL) {
  civicrm_initialize();
  
  if (!isset($uid)) {
    $uid = _osa_getUserId();
  }
  
  if ($uid == 0) {
    return 0;
  }

  // use static caching
  static $cids = NULL;
  if (isset($cids[$uid])) {
    return $cids[$uid];
  }

  $result = civicrm_api('UFMatch', 'get', array('uf_id' => $uid, 'sequential' => 1, 'version' => 3));
  $cids[$uid] = isset($result['values'][0]['contact_id']) ? $result['values'][0]['contact_id'] : 0;

  return $cids[$uid];
}

/**
 * Wrapper function to return a contact record
 */
function _osa_get_contact($cid = NULL, $custom = TRUE) {

  if (!isset($cid)) {
    $cid = _osa_get_contact_id();
  }

  // use static caching
  static $contact = array();
  $cache_key = "cid=$cid&custom=$custom";
  if (isset($contact[$cache_key])) {
    return $contact[$cache_key];
  } elseif (!$custom && isset($contact["cid=$cid&custom=1"])) {
    return $contact["cid=$cid&custom=1"];
  }

  // get the contact record
  $result = civicrm_api('contact', 'get',  array('id' => $cid, 'version' => 3));
  if (($result['is_error'] == 0) && ($result['count'] == 1)) {
    $contact[$cache_key] = $result['values'][$cid];
  }
  else {
    return NULL;
  }

  // fill in any custom fields
  if ($custom) {
    static $custom_fields = NULL;
    if (!isset($custom_fields)) {
      $result = civicrm_api('CustomField', 'get',  array('rowCount' => PHP_INT_MAX, 'version' => 3));
      $custom_fields = $result['values'];
    }

    $result = civicrm_api('CustomValue', 'get',  array('entity_id' => $cid, 'entity_type' => 'Contact', 'version' => 3));
    if ($result['is_error'] == 0) {
      foreach ($result['values'] as $fid => $fattribs) {
        $fname = $custom_fields[$fid]['name'];
        $ignore = array('entity_id' => 0, 'latest' => 0, 'id' => 0);
        $values = array_diff_key($fattribs, $ignore);
        if (isset($values[0])) {
          $contact[$cache_key][$fname] = $values[0];
        }
        else {
          $contact[$cache_key][$fname] = array();
          foreach ($values as $vid => $value) {
            $contact[$cache_key][$fname][$vid] = $value;
          }
        }
      }
    }
  }

  // add helper code to handle multiple sub types (Parent/Teachers, Parent/Students)
  $contact[$cache_key]['is_parent']  = FALSE;
  $contact[$cache_key]['is_student'] = FALSE;
  $contact[$cache_key]['is_teacher'] = FALSE;
  if (empty($contact[$cache_key]['contact_sub_type'])) {
    $contact[$cache_key]['osa_contact_type'] = $contact[$cache_key]['contact_type'];
  }
  elseif (count($contact[$cache_key]['contact_sub_type']) == 1) {
    $contact[$cache_key]['osa_contact_type'] = $contact[$cache_key]['contact_sub_type'][0];
    $type_key = 'is_' . strtolower($contact[$cache_key]['contact_sub_type'][0]);
    $contact[$cache_key][$type_key] = TRUE;
  }
  else {
    foreach ($contact[$cache_key]['contact_sub_type'] as $sub_type ) {
      $type_key = 'is_' . strtolower($sub_type);
      $contact[$cache_key][$type_key] = TRUE;
    }
    if ($contact[$cache_key]['is_parent']) {
      $contact[$cache_key]['osa_contact_type'] = 'Parent';
    }
    elseif ($contact[$cache_key]['is_student']) {
      $contact[$cache_key]['osa_contact_type'] = 'Student';
    }
    elseif ($contact[$cache_key]['is_teacher']) {
      $contact[$cache_key]['osa_contact_type'] = 'Teacher';
    }
    else {
      $contact[$cache_key]['osa_contact_type'] = $contact[$cache_key]['contact_sub_type'][0];
    }
  }

  return $contact[$cache_key];
}

/**
 * Wrapper function to return a contact's relationships
 */
function _osa_get_relationships($cid = NULL) {
  if (!isset($cid)) {
    $cid = _osa_get_contact_id();
  }

  // use static caching
  static $relationships = array();
  if (isset($relationships[$cid])) {
    return $relationships[$cid];
  }

  $result = civicrm_api('relationship', 'get', array('contact_id' => $cid, 'version' => 3));
  $relationships[$cid] = $result['values'];
  
  return $relationships[$cid];
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

  // make sure we have valid ids
  if ($contact_id_a <= 1 || $contact_id_b <= 1) {
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

  // use static caching
  static $memberships = array();
  if (isset($memberships[$contact_id])) {
    return (empty($memberships[$contact_id])) ? NULL : $memberships[$contact_id];
  }
  $membership = array();
 
  // Call the CiviCRM API to get the membership
  $result = civicrm_api('membership', 'get', array('contact_id' => $contact_id, 'version' => '3'));
  foreach ($result['values'] as $values) {
    if ($values['membership_type_id'] == $membership_type_id) {
      $membership = $values;
      break;
    }
  }

  if (empty($membership)) {
    $memberships[$contact_id] = $membership;
    return NULL;
  }

  // api doesn't return everything we need
  $membership_id = $membership['id'];
  $result = CRM_Member_BAO_Membership::getStatusANDTypeValues($membership_id);
  $membership += $result[$membership_id];

  if (!$membership['is_current_member']) {
    $renew_page_id = CRM_Member_BAO_Membership::getContributionPageId($membership_id);
    if (!$renew_page_id) {
      $renew_page_id = CRM_Core_BAO_Setting::getItem(CRM_Core_BAO_Setting::MEMBER_PREFERENCES_NAME, 'default_renewal_contribution_page');
    }
    $membership['renew_href'] = CRM_Utils_System::href(ts('Renew Now'), 'civicrm/contribute/transact', array('reset' => 1, 'id' => $renew_page_id, 'cid' => $contact_id), FALSE, NULL, FALSE);
  }

  // get the school yr of this membership
  $end_date = new DateTimeEx($membership['end_date']);
  $end_date->modify('-1 year');
  $membership['school_year'] = _osa_school_year($school_start_date, $school_end_date, $end_date->format('Y-m-d'));
  
  $memberships[$contact_id] = $membership;
  return $membership;
}

/**
 * Gets start and end dates associated with a membership type
 */
function _osa_get_membership_dates($membership_type_id = OSA_MEMBER_STUDENT) {
  static $membershipDates = array();

  if (!isset($membershipDates[$membership_type_id])) {
    $membershipType = new CRM_Member_BAO_MembershipType();
    $dates = $membershipType->getDatesForMembershipType($membership_type_id);
    // dates are note returned in same format as v3 api
    foreach ($dates as $name => $date) {
      if (!empty($date)) {
        $membershipDates[$membership_type_id][$name] = substr($date, 0, 4) . '-' . substr($date, 4, 2) . '-' . substr($date, 6, 2);
      }
    }
  }

  return $membershipDates[$membership_type_id];
}

/**
 * Wrapper of CiviCRM API to get contributions
 *
 * @param integer $contact_id
 *   contact who's contributions to get
 */
function _osa_get_contributions($contact_id) {
/*
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
    watchdog('osa', 'Error: %1\nfailed to get contributions for contact_id=%2', array(1 => $result['error_message'], 2 => $contact_id), WATCHDOG_ERROR);
  }

  return $result['values'];
*/

  $contact = _osa_get_contact($contact_id, FALSE);

  $query = <<<EOD
SELECT civicrm_contribution.id AS contribution_id, civicrm_financial_type.name AS financial_type, "{$contact['sort_name']}" AS sort_name, civicrm_contribution.total_amount AS total_amount, civicrm_contribution.contribution_recur_id AS contribution_recur_id, civicrm_contribution.receive_date AS receive_date, civicrm_option_value.label AS contribution_status, civicrm_option_value2.label AS payment_instrument
FROM civicrm_contribution
JOIN civicrm_financial_type ON civicrm_contribution.financial_type_id = civicrm_financial_type.id
JOIN civicrm_option_value ON civicrm_contribution.contribution_status_id = civicrm_option_value.value AND civicrm_option_value.option_group_id = 11
JOIN civicrm_option_value civicrm_option_value2 ON civicrm_contribution.payment_instrument_id = civicrm_option_value2.value AND civicrm_option_value2.option_group_id = 10
WHERE (contact_id = $contact_id) AND (total_amount > 0)
ORDER BY civicrm_contribution.id DESC
EOD;

  $contributions = array();
  $dao = CRM_Core_DAO::executeQuery($query);
  while ($dao->fetch()) {
      $contribution['contribution_id'] = $dao->contribution_id;
      $contribution['financial_type'] = $dao->financial_type;
      $contribution['sort_name'] = $dao->sort_name;
      $contribution['total_amount'] = $dao->total_amount;
      $contribution['contribution_recur_id'] = $dao->contribution_recur_id;
      $contribution['receive_date'] = $dao->receive_date;
      $contribution['contribution_status'] = $dao->contribution_status;
      $contribution['payment_instrument'] = $dao->payment_instrument;

      if (($contribution['payment_instrument'] == 'Credit Card') && ($contribution['contribution_status'] == 'Pending')) {
        $contribution['contribution_status'] = ts('In Shopping Cart');
        $contribution['payment_instrument'] = '';
      }
      
      $contributions[$dao->contribution_id] = $contribution;
  }
  $dao->free();

  return $contributions;
}

/**
 * Wrapper function to return a participant record
 */
function _osa_get_participant($pid, $custom = TRUE) {

  if (empty($pid)) {
    return NULL;
  }

  // use static caching
  static $participant = array();
  $cache_key = "pid=$pid&custom=$custom";
  if (isset($participant[$cache_key])) {
    return $participant[$cache_key];
  }
  elseif (!$custom && isset($participant["pid=$pid&custom=1"])) {
    return $participant["pid=$pid&custom=1"];
  }

  // get the participant record
  $result = civicrm_api('participant', 'get',  array('id' => $pid, 'version' => 3));
  if (($result['is_error'] == 0) && ($result['count'] == 1)) {
    $participant[$cache_key] = $result['values'][$pid];
  }
  else {
    return NULL;
  }

  // fill in any custom fields
  if ($custom) {
    static $custom_fields = NULL;
    if (!isset($custom_fields)) {
      $result = civicrm_api('CustomField', 'get',  array('rowCount' => PHP_INT_MAX, 'version' => 3));
      $custom_fields = $result['values'];
    }

    $result = civicrm_api('CustomValue', 'get',  array('entity_id' => $pid, 'entity_table' => 'Participant', 'version' => 3));
    if ($result['is_error'] == 0) {
      foreach ($result['values'] as $fid => $fattribs) {
        $fname = $custom_fields[$fid]['name'];
        $ignore = array('entity_id' => 0, 'latest' => 0, 'id' => 0);
        $values = array_diff_key($fattribs, $ignore);
        if (isset($values[0])) {
          $participant[$cache_key][$fname] = $values[0];
        }
        else {
          $participant[$cache_key][$fname] = array();
          foreach ($values as $vid => $value) {
            $participant[$cache_key][$fname][$vid] = $value;
          }
        }
      }
    }
  }

  return $participant[$cache_key];
}

/**
 * Wrapper of CiviCRM API to get participant events
 *
 * @param integer $contact_id
 *   contact who's events we're to get
 */
function _osa_get_participant_events($contact_id, $current = TRUE) {
  // build the  parameters
  $params = array(
    'contact_id' => $contact_id,
    'version'    => '3',
 );

  // Call the CiviCRM API to get the events
  $result = civicrm_api('participant', 'get', $params);
  if ($result['is_error']) {
    watchdog('osa', 'Error: %1\nfailed to get events for contact_id = %2', array('%1' => $result['error_message'], '%2' => $contact_id), WATCHDOG_ERROR);
  }

  // "current" is participation less than one week old
  if ($current) {
    foreach ($result['values'] as $id => $value) {
      if (strtotime($value['event_end_date']) < strtotime('-1 week')) {
        unset($result['values'][$id]);
      }
    }
  }

  return $result['values'];
}

/**
 *  Get the all participants of an event related to a Household contact
 */
function _osa_get_household_participants($hid, $event_id) {
  // use static caching
  static $household_participants = array();
  $cache_key = "hid=$hid&event_id=$event_id";
  if (isset($household_participants[$cache_key])) {
    return $household_participants[$cache_key];
  }
  $household_participants[$cache_key] = array();

  // get all family members
  $family = _osa_getHouseholdMembers($hid);

  // get the participant records, for this event
  $params = array(
    'event_id' => $event_id,
    'version'  => '3',
  );
  $registered_status = array(1, 2, 5, 6, 14, 15);

  foreach ($family as $contact) {
    $params['contact_id'] = $contact['contact_id'];
    $result = civicrm_api('participant', 'get', $params);
    if ($result['is_error'] == 0) {
      foreach ($result['values'] as $pid => $participant) {
        if (in_array($participant['participant_status_id'], $registered_status)) {
          $household_participants[$cache_key][$pid] = $participant;
        }
      }
    }
  }
  
  return $household_participants[$cache_key];
}

/**
 * Returns the form_type field from the node associated with a form (i.e. a webform)
 */
function _osa_get_form_type(&$form) {
  return isset($form['#node']) ? _osa_get_node_form_type($form['#node']) : NULL;
}

/**
 * Returns the form_type field from a node
 */
function _osa_get_node_form_type(&$node) {
  if (isset($node->field_form_type)) {
    $node_wrapper = entity_metadata_wrapper('node', $node);
    return $node_wrapper->field_form_type->value();
  }
  else {
    return NULL;
  }
}

/**
 * Returns a component object from a webform node
 */
function &_osa_get_webform_component(&$node, $form_key) {

  $cid = _osa_get_webform_component_id($node->webform['components'], $form_key);
  return ($cid !== FALSE) ? $node->webform['components'][$cid] : NULL;
}

/**
 * Returns the id of a webform component given the components & form_key
 */
function _osa_get_webform_component_id($components, $form_key) {

  static $webform_component_map = array();

  $nid = isset($components[1]['nid']) ? $components[1]['nid'] : NULL;
  if (isset($nid) && isset($webform_component_map[$nid]) && isset($webform_component_map[$nid][$form_key])) {
    return $webform_component_map[$nid][$form_key];
  }

  $map = array();
  foreach ($components as $cid => $component) {
    $map[$component['form_key']] = $cid;
  }

  if (isset($nid)) {
    $webform_component_map[$nid] = $map;
  }

  return isset($map[$form_key]) ? $map[$form_key] : FALSE;
}

/**
 * Define option lists for drop downs on webforms
 */
function osa_webform_options_family() {
  $camp_participant = NULL;

  // find out if there is an existing submission
  $path = request_path();
  $matches = array();
  if (preg_match('#node\/(?P<nid>\d+)\/submission\/(?P<sid>\d+)#', $path, $matches) === 1) {
    $node = node_load($matches['nid']);
    $submission = webform_get_submission($matches['nid'], $matches['sid']);
    $cid = _osa_get_webform_component_id($node->webform['components'], 'camp_participant');
    $camp_participant = $submission->data[$cid][0];
  }

  civicrm_initialize();
  return _osa_getHouseholdMembers($camp_participant, 'display_name');
}

/**
 *  Calculate the next payment date and number of instalments
 *
 * @param string $rel_start_date
 *   relative start of the school year & first payment (ie 'first day of September')
 * @param integer $num_installments
 *   total number of instalments / months in school year. Will be updated if the school year has begun.
 * @param string $base
 *   date used for the basis of the calculation
 */
function _osa_next_payment_date($rel_start_date, &$num_installments, $base = 'today') {

  // adjust to the given base date
  $start_date = new DateTimeEx($rel_start_date);
  $today = new DateTimeEx($base);
  $start_date->setDate($today->get('year'), $start_date->get('month'), $start_date->get('day'));
  
  $end_date = new DateTimeEx($start_date);
  $tmp = $num_installments - 1;
  $end_date->modify("first day of +$tmp months");

  // adjust year to account for non-calendar school year
  if ($start_date->get('month') > 1) {
    if ($today->get('month') <= $end_date->get('month')) {
      $start_date->modify('-1 year');
      $end_date->modify('-1 year');
    }
  }

  // next payment date cannot be in the past
  if ($today > $start_date) {
    // reduce the number of instalments
    $num_installments -= $today->diff($start_date)->format('%m');
    return $today;
  }

  return $start_date;
}

/**
 *  Calculate the school year
 */
function _osa_school_year(&$school_start_date, &$school_end_date, $base = 'today') {

  // adjust to the given base date
  $school_start_date = new DateTimeEx(variable_get('osa_start_date', 'first day of September'));
  $today = new DateTimeEx($base);
  $school_start_date->setDate($today->get('year'), $school_start_date->get('month'), $school_start_date->get('day'));
  $school_end_date = new DateTimeEx($school_start_date);
  $school_end_date->modify("last day of +9 months");

  // adjust year to account for non-calendar school year
  if ($school_start_date->get('month') > 1) {
    if ($today->get('month') <= $school_end_date->get('month')) {
      $school_start_date->modify('-1 year');
      $school_end_date->modify('-1 year');
    }
  }
  
  $school_year = "{$school_start_date->get('year')} / {$school_end_date->get('year')}";
  return $school_year;
}


/**
 * Better DateTime class
 */
class DateTimeEx extends DateTime {
  public static $_format = 'Y-m-d H:i:s';

  public function __toString() {
    return (string)parent::format('Y-m-d H:i:s');
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

/**
 * Return a formatted string for the event date
 */
function _osa_format_event_date($start_date, $end_date = NULL) {
  $start = new DateTime($start_date);
  $return = $start->format('M j, Y g:i A');
  if (isset($end_date)) {
    $end = new DateTime($end_date);
    $return .= ($end->format('Ymd') == $start->format('Ymd')) ? $end->format(' - g:i A') : $end->format(' - M j, Y g:i A');
  }
  return $return;
}

/**
 * Callback for uasort()
 */
function osa_event_start_date_sort($a, $b) {
  $a_start = (is_array($a) && isset($a['event_start_date'])) ? strtotime($a['event_start_date']) : 0;
  $b_start = (is_array($b) && isset($b['event_start_date'])) ? strtotime($b['event_start_date']) : 0;
  if ($a_start == $b_start) {
    return 0;
  }
  return ($a_start < $b_start) ? -1 : 1;
}

/*
 * Helper function
 */
function _osa_object_to_array($obj) {
  $arrObj = is_object($obj) ? get_object_vars($obj) : $obj;
  foreach ($arrObj as $key => $val) {
    $val = (is_array($val) || is_object($val)) ? $this->object_to_array($val) : $val;
    $arr[$key] = $val;
  }
  return $arr;
}

/*
 * Called to create the email with eTicket attachments
 */
function osa_mail_ticket(&$message, $params) {

  $commerce_order = $params['commerce_order'];

  $message['subject'] = "Your eTicket(s) from The Oakville Suzuki Association for order #{$commerce_order->order_id}";
  $message['body'][] = <<<EOD
This email contains your <strong>eTicket(s)</strong> for order #{$commerce_order->order_id}.<br />
<br />
<span style="font-size: 150%;"><strong><em>Please print this email and bring it to the concert.</em></strong></span><br />
<br />
<br />
EOD;

  $order_wrapper = entity_metadata_wrapper('commerce_order', $commerce_order);
  foreach ($order_wrapper->commerce_line_items as $line_item_wrapper) {
    $line_item = $line_item_wrapper->value();
    $quantity = 0;

    if (($line_item->type == 'civi_event') && ($line_item->data['contributionTypeID'] == OSA_FINTYPE_CONCERT_TICKET)) {
      $quantity = $line_item->quantity;
      $event_id = $line_item_wrapper->civicrm_commerce_event_id->value();
    }
    elseif (($line_item->type == 'product') && ($line_item_wrapper->commerce_product->type->value() == 'commerce_ticket')) {
      $quantity = $line_item->quantity;
      $product = $line_item_wrapper->commerce_product->value();
      $event_id = $product->field_event[LANGUAGE_NONE][0]['civicrm_reference_id'];
    }
    for ($i = 1; $i <= $quantity; $i++) {
      $message['body'][] = _osa_create_ticket($event_id, $line_item->line_item_id, $i);
    }
  }

/*
  require_once 'CRM/Utils/PDF/Utils.php';
  $message['params']['attachments'][] = array(
    'filecontent' => CRM_Utils_PDF_Utils::html2pdf($tickets_html, 'eTicket.pdf', TRUE),
    'filename' => 'eTicket.pdf',
    'filemime' => 'application/pdf',
  );
*/

  $message['body'][] = <<<EOD
<br />
<br />
Thank you for supporting The Oakville Suzuki Association.<br />
<br />
<br />
<strong>The Oakville Suzuki Association</strong><br />
<em>"Music exists for the purpose of growing an admirable heart." - Shinichi Suzuki</em><br />
http://oakvillesuzuki.org<br />
admin@oakvillesuzuki.org<br />
EOD;

}

function _osa_create_ticket($event_id, $line_item_id, $number) {
  $ticket_number = sprintf('%05d-%05d-%03d', $event_id, $line_item_id, $number);

  // get the event details
  civicrm_initialize();
  $result = civicrm_api('event', 'get',  array('id' => $event_id, 'version' => 3));
  if (($result['is_error'] == 0) && ($result['count'] == 1)) {
    $event = $result['values'][$event_id];
  }
  else {
    return '';
  }

  // clean up for display on the tickets
  $start_date = strftime('%A %B %#d, %I:%M %p', strtotime($event['start_date']));
  require_once 'CRM/Core/BAO/Location.php';
  $location = CRM_Core_BAO_Location::getValues(array('entity_id' => $event_id, 'entity_table' => 'civicrm_event'), TRUE);
  $address = $location['address'][1];
  $address_html = $address['supplemental_address_1'] . '<br />';
  $address_html .= str_replace($address['supplemental_address_1'] . PHP_EOL, '', $address['display_text']);
//  $description = str_replace('"/', '"http://oakvillesuzuki.org/', $event['description']);

  $ticket_html = <<<EOD
<br />
<hr />
<table>
  <tbody>
    <tr>
      <td rowspan="4">
        <img alt="The Oakville Suzuki Association" src="http://oakvillesuzuki.org/osa/sites/default/files/logo.jpg" style="margin-left: 10px; margin-right: 10px;" /></td>
      <td>
        <span style="font-size: 125%; color:#0000ff;"><strong>{$event['title']}</strong></span><br />
        <br /><span style="color:#0000ff;">{$event['summary']}</span>
      </td>
    </tr>
    <tr>
      <td>
        <hr />
        $start_date
      </td>
    </tr>
    <tr>
      <td>
        <hr />
        <strong>Location: </strong>$address_html<br />
        <hr />
      </td>
    </tr>
    <tr>
      <td>
        <span style="color:#ff0000; font-size:125%;"><strong>Ticket Number: $ticket_number</strong></span></td>
    </tr>
  </tbody>
</table>
<hr />
<br />
EOD;

  return $ticket_html;
}

function osa_html_table_to_array($html) {
  $return = array();
  
  $DOM = new DOMDocument;
  $DOM->loadHTML($html);
  $trs = $DOM->getElementsByTagName('tr');
  foreach ($trs as $tr) {
    $row = array();
    foreach($tr->childNodes as $td) {
      $row[] = $td->nodeValue;
    }
    $return[] = $row;
  }
  return $return;
}
