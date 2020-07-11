<?php
/*
 +--------------------------------------------------------------------+
 | CiviCRM version 5                                                  |
 +--------------------------------------------------------------------+
 | Copyright CiviCRM LLC (c) 2004-2019                                |
 +--------------------------------------------------------------------+
 | This file is a part of CiviCRM.                                    |
 |                                                                    |
 | CiviCRM is free software; you can copy, modify, and distribute it  |
 | under the terms of the GNU Affero General Public License           |
 | Version 3, 19 November 2007 and the CiviCRM Licensing Exception.   |
 |                                                                    |
 | CiviCRM is distributed in the hope that it will be useful, but     |
 | WITHOUT ANY WARRANTY; without even the implied warranty of         |
 | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.               |
 | See the GNU Affero General Public License for more details.        |
 |                                                                    |
 | You should have received a copy of the GNU Affero General Public   |
 | License and the CiviCRM Licensing Exception along                  |
 | with this program; if not, contact CiviCRM LLC                     |
 | at info[AT]civicrm[DOT]org. If you have questions about the        |
 | GNU Affero General Public License or the licensing of CiviCRM,     |
 | see the CiviCRM license FAQ at http://civicrm.org/licensing        |
 +--------------------------------------------------------------------+
 */

/**
 *
 * @package CRM
 * @copyright CiviCRM LLC (c) 2004-2019
 */

/**
 * This class is used to build User Dashboard
 */
class CRM_Contact_Page_View_UserDashBoard extends CRM_Core_Page {
  public $_contactId = NULL;

  /**
   * Always show public groups.
   * @var bool
   */
  public $_onlyPublicGroups = TRUE;

  public $_edit = TRUE;

  // OSA added attributes
  public $_hid    = 0;
  public $_family = [];
  public $_family_members = [];

  /**
   * The action links that we need to display for the browse screen.
   *
   * @var array
   */
  public static $_links = NULL;

  /**
   * @throws Exception
   */
  public function __construct() {
    parent::__construct();

    if (!CRM_Core_Permission::check('access Contact Dashboard')) {
      CRM_Utils_System::redirect(CRM_Utils_System::url(''));
    }

    $this->_contactId = CRM_Utils_Request::retrieveValue('id', 'Positive');
    if (!$this->_contactId) {
      $this->_contactId = CRM_Utils_Request::retrieveValue('cid', 'Positive');
    }
    $userID = CRM_Core_Session::singleton()->getLoggedInContactID();

    $userChecksum = $this->getUserChecksum();
    $validUser = FALSE;
    if ($userChecksum) {
      $this->assign('userChecksum', $userChecksum);
      $validUser = CRM_Contact_BAO_Contact_Utils::validChecksum($this->_contactId, $userChecksum);
      $this->_isChecksumUser = $validUser;
    }

    if (!$this->_contactId) {
      $this->_contactId = $userID;
    }
    elseif ($this->_contactId != $userID && !$validUser) {
      if (!CRM_Contact_BAO_Contact_Permission::allow($this->_contactId, CRM_Core_Permission::VIEW)) {
        CRM_Core_Error::fatal(ts('You do not have permission to access this contact.'));
      }
      if (!CRM_Contact_BAO_Contact_Permission::allow($this->_contactId, CRM_Core_Permission::EDIT)) {
        $this->_edit = FALSE;
      }
    }
  }

  /**
   * Heart of the viewing process.
   *
   * The runner gets all the meta data for the contact and calls the appropriate type of page to view.
   */
  public function preProcess() {
    if (!CRM_Core_Session::singleton()->getLoggedInContactID()) {
      CRM_Core_Error::fatal(ts('You must be logged in to view this page.'));
    }

    $cid = $this->_contactId;
    $hid =_osa_getHousehold($cid);

    if (!isset($hid)) {
      // no household - so get the user to create one
      CRM_Utils_System::setUFMessage('Please complete the following form to begin creating your Family Profile.');
      CRM_Utils_System::redirect(CRM_Utils_System::url('civicrm/profile/create?reset=1&gid=' . _osa_profileId('Family')));
    }
    $this->hid = $hid;

    $family = _osa_get_contact($hid);
    $family['credits_required']  = osa_get_volunteer_credits_required($hid);
    $family['credits_committed'] = osa_get_volunteer_credits_committed($hid);
    $this->_family = $family;
    CRM_Utils_System::setTitle("Family Profile for - {$family['display_name']}");

    $family_members = _osa_getHouseholdMembers($hid);
    $family_ids = array_keys($family_members);
    $family_ids[] = $this->_family['contact_id'];

    $result = civicrm_api3('Membership', 'get', ['contact_id' => ['IN' => $family_ids],]);
    if (!$result['is_error']) {
      foreach ($result['values'] as $membership) {
        $membership['school_year'] = osa_get_school_year($membership['end_date']);
        $membership['status'] = CRM_Member_BAO_MembershipStatus::getMembershipStatus($membership['status_id'])['membership_status'];
        $family_members[$membership['contact_id']]['membership'] = $membership;
      }
    }

    $today = new DateTimeEx(); 
    $result = civicrm_api3('Event', 'get', ['end_date' => ['>=' => $today->format('Y-m-d')], 'event_type_id' => ['NOT IN' => ["Meeting", "Conference", "Group Class (Child)", "Group Class (Special)"]],]);
    if (!$result['is_error']) {
      $events = $result['values'];
      $event_ids = array_keys($events);
      $result = civicrm_api3('Participant', 'get', ['contact_id' => ['IN' => $family_ids], 'event_id' => ['IN' => $event_ids], 'status_id' => ['NOT IN' => ["Cancelled", "Pending from incomplete transaction", "Rejected", "Expired", "Pending refund", "Transferred",]],]);
      if (!$result['is_error']) {
        foreach ($result['values'] as $pid => $participant) {
          if ($events[$participant['event_id']]['event_type_id'] == OSA_EVENT_GROUP_CLASS_MASTER) {
            $family_members[$participant['contact_id']]['group_class'][$pid] = $participant;
          } else {
            $family_members[$participant['contact_id']]['participant'][$pid] = $participant;
          }
        }
      }
    }

    $this->_family_members = $family_members;
  }

  /**
   * Build user dashboard.
   */
  public function buildUserDashBoard() {

    $this->assign('contactId', $this->_contactId);
    $this->assign('hid', $this->_hid);
    $this->assign('family', $this->_family);

    $parents  = array_filter($this->_family_members, function($p) {return $p['contact']['osa_contact_type'] == 'Parent';});
    $students = array_filter($this->_family_members, function($p) {return $p['contact']['osa_contact_type'] == 'Student';});
    $others   = array_filter($this->_family_members, function($p) {return ($p['contact']['osa_contact_type'] != 'Parent') && ($p['osa_contact_type'] != 'Student');});

    $today = new DateTimeEx();
    foreach ($students as &$student) {
      $dob = new DateTimeEx($student['contact']['birth_date']);
      $age = $dob->diff($today);
      $years = ($age->y > 0) ? (($age->y > 1) ? "{$age->y} years " : '1 year ') : '';
      $months = ($age->m > 0) ? (($age->m > 1) ? "{$age->m} months " : '1 month ') : '';
      $student['contact']['age'] = $years . $months;
    }
    
    $this->assign('parents', $parents);
    $this->assign('students', $students);
    $this->assign('others', $others);
  }

  /**
   *
   */
  public function postProcess() {
    // take the opportunity to clean house (only once a day)
    if (isset($this->hid)) {
      $last_update = $this->get("household_lastupdate_{$this->hid}");
      if (empty($last_update) || ($_SERVER['REQUEST_TIME'] > ($last_update + 86400))) {
        _osa_manageHouseholdRelationships($hid);
        $this->set("household_lastupdate_{$this->hid}", $_SERVER['REQUEST_TIME']);
      }
    }
  }

  /**
   * Perform actions and display for user dashboard.
   */
  public function run() {
    $this->preProcess();
    $this->buildUserDashBoard();
    $this->postProcess();
    return parent::run();
  }

  /**
   * Get action links.
   *
   * @return array
   *   (reference) of action links
   */
  public static function &links() {
    if (!(self::$_links)) {
      $disableExtra = ts('Are you sure you want to disable this relationship?');

      self::$_links = [
        CRM_Core_Action::UPDATE => [
          'name' => ts('Edit Contact Information'),
          'url' => 'civicrm/contact/relatedcontact',
          'qs' => 'action=update&reset=1&cid=%%cbid%%&rcid=%%cid%%',
          'title' => ts('Edit Contact Information'),
        ],
        CRM_Core_Action::VIEW => [
          'name' => ts('Dashboard'),
          'url' => 'civicrm/user',
          'class' => 'no-popup',
          'qs' => 'reset=1&id=%%cbid%%',
          'title' => ts('View Contact Dashboard'),
        ],
      ];

      if (CRM_Core_Permission::check('access CiviCRM')) {
        self::$_links += [
          CRM_Core_Action::DISABLE => [
            'name' => ts('Disable'),
            'url' => 'civicrm/contact/view/rel',
            'qs' => 'action=disable&reset=1&cid=%%cid%%&id=%%id%%&rtype=%%rtype%%&selectedChild=rel&context=dashboard',
            'extra' => 'onclick = "return confirm(\'' . $disableExtra . '\');"',
            'title' => ts('Disable Relationship'),
          ],
        ];
      }
    }

    // call the hook so we can modify it
    CRM_Utils_Hook::links('view.contact.userDashBoard',
      'Contact',
      CRM_Core_DAO::$_nullObject,
      self::$_links
    );
    return self::$_links;
  }

  /**
   * Get the user checksum from the url to use in links.
   *
   * @return string
   */
  protected function getUserChecksum() {
    $userChecksum = CRM_Utils_Request::retrieve('cs', 'String', $this);
    if (empty($userID) && $this->_contactId) {
      return $userChecksum;
    }
    return FALSE;
  }

}
