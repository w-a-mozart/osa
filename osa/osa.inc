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
define('OSA_PROFILE_STUDENT', 11);
define('OSA_PROFILE_ADDRESS', 12);
define('OSA_PROFILE_PHONE_EMAIL', 13);
define('OSA_PROFILE_PARENT', 14);
define('OSA_PROFILE_REL_INDIVIDUAL', 16);

/**
 * Define constants for CiviCRM Contribution Pages.
 */
define('OSA_CONTRIB_MEMBER', 1);
define('OSA_CONTRIB_EVENT', 1);
define('OSA_CONTRIB_TEACHER', 2);

/**
 * Define constants for CiviCRM Relationship Types.
 */
define('OSA_REL_PARENT', 1);
define('OSA_REL_SPOUSE', 2);
define('OSA_REL_SIBLING', 3);
define('OSA_REL_HEAD_HOUSEHOLD', 6);
define('OSA_REL_HOUSEHOLD_MEMBER', 7);
define('OSA_REL_OTHER', 12);

/**
 * Define constants for CiviCRM Relationship Permissions.
 */
define('OSA_PERM_NONE',  0);
define('OSA_PERM_OWNER', 1);
define('OSA_PERM_PEER',  2);

/**
 * Map name to CiviCRM Profile Ids
 *
 * @param string $entity
 *   The name of the entity / contact type.
 * @return integer
 *   Default profile id.
 */
function _osa_profileId($entity) {
  static $_gids = null;
  
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

  foreach($family as $person) {
    $contact = $person['contact'];
    $contactType = isset($contact['contact_sub_type']) ? $contact['contact_sub_type'][0] : $contact['contact_type'];

    if ($contactType == 'Parent') {
      $parents[] = $contact;
    }
    elseif ($contactType == 'Student') {
      $students[] = $contact;
    }
    elseif ($contactType == 'Individual') {
      $others[] = $contact;
    }
  }
  
  // make sure relationships exist between parents and parents (spousal), students (child), and others (other)
  foreach($parents as $parent) {
    $result = civicrm_api('relationship', 'get', array('contact_id' => $parent['contact_id'], 'version' => 3));
    $relationships = $result['values'];

    //  make sure relationships exist between parents
    foreach($parents as $spouse) {
      if ($parent['contact_id'] == $spouse['contact_id']) {
        continue;
      }
      $found = false;
      foreach($relationships as $relationship) {
        if (($spouse['contact_id'] == $relationship['contact_id_a']) ||
             ($spouse['contact_id'] == $relationship['contact_id_b'])) {
          $found = true;
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
    foreach($students as $student) {
      $found = false;
      foreach($relationships as $relationship) {
        if (($student['contact_id'] == $relationship['contact_id_a']) ||
             ($student['contact_id'] == $relationship['contact_id_b'])) {
          $found = true;
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
    foreach($others as $other) {
      $found = false;
      foreach($relationships as $relationship) {
        if (($other['contact_id'] == $relationship['contact_id_a']) ||
             ($other['contact_id'] == $relationship['contact_id_b'])) {
          $found = true;
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
  foreach($students as $student) {
    $result = civicrm_api('relationship', 'get', array('contact_id' => $student['contact_id'], 'version' => 3));
    $relationships = $result['values'];

    // make sure relationships exist between siblings
    foreach($students as $sibling) {
      if ($student['contact_id'] == $sibling['contact_id']) {
        continue;
      }
      $found = false;
      foreach($relationships as $relationship) {
        if (($sibling['contact_id'] == $relationship['contact_id_a']) ||
             ($sibling['contact_id'] == $relationship['contact_id_b'])) {
          $found = true;
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
  foreach($others as $other) {
    foreach($others as $dup) {
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
 *  Get the id of the related Household Contact
 */
function _osa_getHousehold($cid = null) {

  if (!isset($cid)) {
    $session = CRM_Core_Session::singleton();
    $cid = $session->get('userID');
    if (! isset($cid)) {
      return null;
    }
  }
  
  // get the contact record
  $result = civicrm_api('contact', 'get', array('id' => $cid, 'version' => 3));
  $contact = $result['values'][$result['id']];
  
  // find the household
  $hid = null;
  if ($contact['contact_type'] == 'Household') {
    $hid = $cid;
  } else {
    $result = civicrm_api('relationship', 'get', array('contact_id' => $cid, 'version' => 3));
    $relationships = $result['values'];
    foreach($relationships as $relationship) {
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
function _osa_getHouseholdMembers($cid = null, $field = null) {
  $family = array();

  // default to logged in user
  if (!isset($cid)) {
    $cid = _osa_getContactId();
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
  foreach($relationships as $relationship) {
    $result = civicrm_api('contact', 'get', array('id' => $relationship['cid'], 'version' => 3));
    $contact = $result['values'][$result['id']];
    $contact_id = $contact['contact_id'];
    
    if (isset($field)) {
      $family[$contact_id] = $contact[$field];
    } else {

      // it's possible to have more than one relationship with the household
      // e.g. sometimes civi creates a household member for the head of the household
      // we want to perserve head of houshold relationships
      if (isset($family[$contact_id])) {
        if ($family[$contact_id]['relationship']['relationship_type_id'] == OSA_REL_HEAD_HOUSEHOLD) {
          continue;
        }
      }
      
      $contactType = isset($contact['contact_sub_type']) ? $contact['contact_sub_type'][0] : $contact['contact_type'];
      $person = array(
        'contact_id'   => $contact_id,
        'contact_type' => $contactType,
        'contact'      => $contact,
        'relationship' => $relationship,
        'links'        => _osa_related_links($contactType, $contact_id, $relationship['id'] )
      );
      $family[$contact_id] = $person;
    }
  }
  
  return $family;
}

/**
 * Return links for related contacts.
 *
 * @param string $contactType
 *   Type of the related contact.
 * @param integer $cid
 *   Id of the related contact.
 * @param integer $rid
 *   Relationship Id of the contact to household
 * @return
 *   links accociated to the given contact.
 */
function _osa_related_links($contactType, $cid, $rid) {
  $links = array();

  if ($contactType == 'Student') {
    $links['member'] = array(
      'name'  => ts('Register Membership'),
      'url'   => 'civicrm/contribute/transact',
      'qs'    => 'reset=1&id=' . OSA_CONTRIB_MEMBER . "&cid=$cid",
      'title' => ts("Register/Renew Membership")
    );
    $links['event'] = array(
      'name'  => ts('Register Event'),
      'url'   => 'civicrm/contribute/transact',
      'qs'    => 'reset=1&id=' . OSA_CONTRIB_EVENT . "&cid=$cid",
      'title' => ts("Register for Event")
    );
    $links['teacher'] = array(
      'name'  => ts('Register Teacher'),
      'url'   => 'civicrm/contribute/transact',
      'qs'    => 'reset=1&id=' . OSA_CONTRIB_TEACHER . "&cid=$cid",
      'title' => ts("Register/Renew with a Teacher")
    );
  }
  
  $links += array (
    CRM_Core_Action::VIEW => array(
      'name'  => ts('View'),
      'url'   => 'civicrm/user',
      'qs'  => "reset=1&id=$cid",
      'title' => ts("View $contactType")
    ),
    CRM_Core_Action::UPDATE => array(
      'name'  => ts('Edit'),
      'url'   => 'civicrm/profile/edit',
      'qs'  => 'reset=1&snippet=1&context=boxload&gid=' . _osa_profileId($contactType) . "&id=$cid",
      'class' => 'box-load',
      'title' => ts("Edit $contactType")
    ),
    CRM_Core_Action::DELETE => array(
      'name'  => ts('Delete'),
      'url'   => 'civicrm/contact/view/rel',
      'qs'  => "action=delete&reset=1&cid=$cid&id=$rid",
      'extra' => 'onclick = "if (confirm(\'Are you sure you want to delete this relationship?\')) this.href+=\'&amp;confirmed=1\'; else return false;"',
      'title' => ts("Delete $contactType")
    )
  );

  return CRM_Core_Action::formLink($links, null, array());
}

/**
 * Add a hidden Household ID field to a CiviCRM Profile Form in order to
 * create relationships when submitted.
 *
 * @param object $form
 *   The CRM_Profile_Form_Edit object being modified.
 * @param integer $hid
 *   The contact id of the household.
 */
function _osa_addHouseholdField(&$form, $hid) {

  // get the profile id & title
  $gid = $form->getVar('_gid');
  $ufGroup = new CRM_Core_DAO_UFGroup();
  $ufGroup->id = $gid;
  if (! $ufGroup->find(true)) {
    CRM_Core_Error::fatal();
  }
  
  // create the field used by the CRM/Profile/Form/Dynamic.tpl template
  $fieldName = 'osa_hid';
  $field = array(
    $fieldName => array (
      'name' => $fieldName,
      'group_id' => $gid,
      'groupTitle' => $ufGroup->title,
      'title' => '',
    )
  );
  $form->_fields[$fieldName] = $field;

  // add an element to the QuickForm
  $form->addElement('hidden', $fieldName, $hid);
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
function _osa_addRelationshipField(&$form, $cid, $required = true) {

  // get the profile id & title
  $gid = $form->getVar('_gid');
  $ufGroup = new CRM_Core_DAO_UFGroup();
  $ufGroup->id = $gid;
  if (! $ufGroup->find(true)) {
    CRM_Core_Error::fatal();
  }
  
  // create the field used by the CRM/Profile/Form/Dynamic.tpl template
  $label = ts('Relationship Type');
  $fieldName = 'rel_id';
  $field = array(
    $fieldName => array (
      'name' => $fieldName,
      'group_id' => $gid,
      'groupTitle' => $ufGroup->title,
      'title' => $label,
    )
  );
    
  // add it to the begining of the list (display first)
  $form->_fields = $field + $form->_fields;

  // get the valid relationship types for this contact
  $rTypes = CRM_Contact_BAO_Relationship::getContactRelationshipType($cid);
  $hhVal  = OSA_REL_HEAD_HOUSEHOLD . '_b_a';
  unset($rTypes[$hhVal]); // head of household is reserved for user who created it
  $options = array('' => ts('- select -')) + $rTypes; // include a 'none' option

  // add an element to the QuickForm
  $form->add('select', $fieldName, $label, $options, '', $required);
  if ($required) {
    $form->addRule($fieldName, ts('%1 is a required field.', array(1 => $label)), 'required');
  }
}

/**
 * Get the Relationship Type field value from a CiviCRM Profile Form
 *
 * @param object $form
 *   The CRM_Profile_Form_Edit object being modified.
 */
function _osa_getRelationshipField(&$form) {
  
  $fieldName = 'rel_id';
  $selVal = CRM_Utils_Array::value($fieldName, $_POST);
  if (!isset($selVal)) {
    return null;
  }
  
  // relationship types are in the form 999_b_a
  $tmp = explode('_', $selVal);
  return $tmp[0];
}

/**
 * Wrapper of CiviCRM API to create relationships
 *
 * @param integer $relationship_type_id
 * @param integer $contact_id_a
 * @param integer $contact_id_b
 * @param integer $permission - defines permissions from b_a
 */
function _osa_create_relationship($relationship_type_id, $contact_id_a, $contact_id_b, $permission = OSA_PERM_OWNER) {
  // build the relationship entity parameters
  $is_permission_a_b = ($permission == OSA_PERM_PEER) ? 1 : 0;
  $is_permission_b_a = ($permission == OSA_PERM_NONE) ? 0 : 1;

  $params = array(
    'relationship_type_id' => $relationship_type_id,
    'contact_id_a'         => $contact_id_a,
    'contact_id_b'         => $contact_id_b,
    'is_active'            => 1,
    'start_date'           => date("Ymd", time()),
    'end_date'             => null,
    'is_permission_a_b'    => $is_permission_a_b,
    'is_permission_b_a'    => $is_permission_b_a,
    'version'              => '3',
  );

  // Call the CiviCRM API to create the relationship
  $result = civicrm_api('relationship', 'create', $params);
  if ($result['is_error']) {
    CRM_Core_Error::debug_log_message(
      "Error: " .
      $result['error_message'] .
      ts("\nfailed to create relationship type_id=%1 between cid_a=%2 and cid_b=%3",
        array(1 => $relationship_type_id, 2 => $contact_id_a, 3 => $contact_id_b)));
    CRM_Core_Error::fatal();
  }
}

/**
 * Wrapper of CiviCRM API to get membership
 *
 * @param integer $contact_id
 */
function _osa_get_membership($contact_id) {
  // build the  parameters
  $params = array(
    'contact_id' => $contact_id,
    'version'    => '3',
 );
  
  // Call the CiviCRM API to get the membership
  $result = civicrm_api('membership', 'get', $params);
  $membership = ($result['count'] > 0) ? $result['values'][$result['id']] : null;

  if (! isset($membership)) {
    return $membership;
  }
  
  // FIX: api doesn't return everything
  $result = CRM_Member_BAO_Membership::getStatusANDTypeValues($membership['id']);
  $result = $result[ $membership['id'] ];
  $membership['status'] = $result['status'];
  $membership['is_current_member'] = $result['is_current_member'];

  $renewPageId = CRM_Member_BAO_Membership::getContributionPageId($membership['id']);
  if (!$renewPageId) {
    require_once 'CRM/Core/BAO/Setting.php';
    $renewPageId = CRM_Core_BAO_Setting::getItem(CRM_Core_BAO_Setting::MEMBER_PREFERENCES_NAME, 'default_renewal_contribution_page');
  }
  $membership['renewPageId'] = $renewPageId ;

  return $membership;
}

/**
 * Wrapper of CiviCRM API to get contributions
 *
 * @param integer $contact_id
 */
function _osa_get_contributions($contact_id) {
  // build the  parameters
  $params = array(
    'contact_id' => $contact_id,
    'sort' => 'receive_date desc',
    'rowCount' => 6,
    'version'    => '3',
 );

  // Call the CiviCRM API to get the contributions
  $result = civicrm_api('contribution', 'get', $params);
  if ($result['is_error']) {
    CRM_Core_Error::debug_log_message(
      "Error: " .
      $result['error_message'] .
      ts("\nfailed to get contributions for contact_id=%1",
        array(1 => $contact_id)));
  }
  return $result;
}

/**
 * 
 */
function _osa_appendTemplateVar(&$smarty, $varname, &$apiResult) {
  $values = ($apiResult['count'] > 0) ? $apiResult['values'] : null;
  $var = $smarty->get_template_vars($varname);
  if (isset($var)) {
    $smarty->assign($varname, (array)$var + (array)$values);
  } else {
    $smarty->assign($varname, $values);
  }
}

/**
 * Wrapper function to return current logged in user's drupal id
 */
function _osa_getUserId() {
  return CRM_Utils_System::getLoggedInUfID();
}

/**
 * Wrapper function to return current logged in user's contact id
 */
function _osa_getContactId() {
  $params = array(
    'uf_id' => _osa_getUserId(),
    'sequential' => 1,
    'version' => 3,
  );

  $result = civicrm_api( 'UFMatch','get',$params );
  return $result['values'][0]['contact_id'];
}
