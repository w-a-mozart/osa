<?php
/*
+--------------------------------------------------------------------+
| CiviCRM version 4.7                                                |
+--------------------------------------------------------------------+
| Copyright CiviCRM LLC (c) 2004-2017                                |
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
 * @package CRM
 * @copyright CiviCRM LLC (c) 2004-2017
 *
 * Generated from xml/schema/CRM/Volunteer/Need.xml
 * DO NOT EDIT.  Generated by CRM_Core_CodeGen
 * (GenCodeChecksum:7a18bad085346374296c8eda6506f7b8)
 */
require_once 'CRM/Core/DAO.php';
require_once 'CRM/Utils/Type.php';
/**
 * CRM_Volunteer_DAO_Need constructor.
 */
class CRM_Volunteer_DAO_Need extends CRM_Core_DAO {
  /**
   * Static instance to hold the table name.
   *
   * @var string
   */
  static $_tableName = 'civicrm_volunteer_need';
  /**
   * Should CiviCRM log any modifications to this table in the civicrm_log table.
   *
   * @var boolean
   */
  static $_log = true;
  /**
   * Need Id
   *
   * @var int unsigned
   */
  public $id;
  /**
   * FK to civicrm_volunteer_project table which contains entity_table + entity for each volunteer project (initially civicrm_event + eventID).
   *
   * @var int unsigned
   */
  public $project_id;
  /**
   *
   * @var datetime
   */
  public $start_time;
  /**
   * Used for specifying fuzzy dates, e.g., I have a need for 3 hours of volunteer work to be completed between 12/01/2015 and 12/31/2015.
   *
   * @var datetime
   */
  public $end_time;
  /**
   * Length in minutes of this volunteer time slot.
   *
   * @var int
   */
  public $duration;
  /**
   * Boolean indicating whether or not the time and role are flexible. Activities linked to a flexible need indicate that the volunteer is generally available.
   *
   * @var boolean
   */
  public $is_flexible;
  /**
   * The number of volunteers needed for this need.
   *
   * @var int
   */
  public $quantity;
  /**
   *  Indicates whether this need is offered on public volunteer signup forms. Implicit FK to option_value row in visibility option_group.
   *
   * @var int unsigned
   */
  public $visibility_id;
  /**
   * The role associated with this need. Implicit FK to option_value row in volunteer_role option_group.
   *
   * @var int unsigned
   */
  public $role_id;
  /**
   * Is this need enabled?
   *
   * @var boolean
   */
  public $is_active;
  /**
   *
   * @var timestamp
   */
  public $created;
  /**
   *
   * @var timestamp
   */
  public $last_updated;
  /**
   * Full description of the Volunteer Need. Text and HTML allowed. Displayed on sign-up screens.
   *
   * @var text
   */
  public $description;
  /**
   * Class constructor.
   */
  function __construct() {
    $this->__table = 'civicrm_volunteer_need';
    parent::__construct();
  }
  /**
   * Returns foreign keys and entity references.
   *
   * @return array
   *   [CRM_Core_Reference_Interface]
   */
  static function getReferenceColumns() {
    if (!isset(Civi::$statics[__CLASS__]['links'])) {
      Civi::$statics[__CLASS__]['links'] = static ::createReferenceColumns(__CLASS__);
      Civi::$statics[__CLASS__]['links'][] = new CRM_Core_Reference_Basic(self::getTableName() , 'project_id', 'civicrm_volunteer_project', 'id');
      CRM_Core_DAO_AllCoreTables::invoke(__CLASS__, 'links_callback', Civi::$statics[__CLASS__]['links']);
    }
    return Civi::$statics[__CLASS__]['links'];
  }
  /**
   * Returns all the column names of this table
   *
   * @return array
   */
  static function &fields() {
    if (!isset(Civi::$statics[__CLASS__]['fields'])) {
      Civi::$statics[__CLASS__]['fields'] = array(
        'id' => array(
          'name' => 'id',
          'type' => CRM_Utils_Type::T_INT,
          'title' => ts('CiviVolunteer Need ID', array('domain' => 'org.civicrm.volunteer')) ,
          'description' => 'Need Id',
          'required' => true,
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
        ) ,
        'project_id' => array(
          'name' => 'project_id',
          'type' => CRM_Utils_Type::T_INT,
          'description' => 'FK to civicrm_volunteer_project table which contains entity_table + entity for each volunteer project (initially civicrm_event + eventID).',
          'required' => false,
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
          'FKClassName' => 'CRM_Volunteer_DAO_Project',
        ) ,
        'start_time' => array(
          'name' => 'start_time',
          'type' => CRM_Utils_Type::T_DATE + CRM_Utils_Type::T_TIME,
          'title' => ts('Start Date and Time', array('domain' => 'org.civicrm.volunteer')) ,
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
        ) ,
        'end_time' => array(
          'name' => 'end_time',
          'type' => CRM_Utils_Type::T_DATE + CRM_Utils_Type::T_TIME,
          'title' => ts('End Date and Time', array('domain' => 'org.civicrm.volunteer')) ,
          'description' => 'Used for specifying fuzzy dates, e.g., I have a need for 3 hours of volunteer work to be completed between 12/01/2015 and 12/31/2015.',
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
        ) ,
        'duration' => array(
          'name' => 'duration',
          'type' => CRM_Utils_Type::T_INT,
          'title' => ts('Duration', array('domain' => 'org.civicrm.volunteer')) ,
          'description' => 'Length in minutes of this volunteer time slot.',
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
        ) ,
        'is_flexible' => array(
          'name' => 'is_flexible',
          'type' => CRM_Utils_Type::T_BOOLEAN,
          'title' => ts('Flexible', array('domain' => 'org.civicrm.volunteer')) ,
          'description' => 'Boolean indicating whether or not the time and role are flexible. Activities linked to a flexible need indicate that the volunteer is generally available.',
          'required' => true,
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
        ) ,
        'quantity' => array(
          'name' => 'quantity',
          'type' => CRM_Utils_Type::T_INT,
          'title' => ts('Quantity', array('domain' => 'org.civicrm.volunteer')) ,
          'description' => 'The number of volunteers needed for this need.',
          'default' => 'NULL',
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
        ) ,
        'visibility_id' => array(
          'name' => 'visibility_id',
          'type' => CRM_Utils_Type::T_INT,
          'title' => ts('Visibility', array('domain' => 'org.civicrm.volunteer')) ,
          'description' => ' Indicates whether this need is offered on public volunteer signup forms. Implicit FK to option_value row in visibility option_group.',
          'default' => 'NULL',
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
          'pseudoconstant' => array(
            'optionGroupName' => 'visibility',
            'optionEditPath' => 'civicrm/admin/options/visibility',
          )
        ) ,
        'role_id' => array(
          'name' => 'role_id',
          'type' => CRM_Utils_Type::T_INT,
          'title' => ts('Role', array('domain' => 'org.civicrm.volunteer')) ,
          'description' => 'The role associated with this need. Implicit FK to option_value row in volunteer_role option_group.',
          'default' => 'NULL',
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
          'pseudoconstant' => array(
            'optionGroupName' => 'volunteer_role',
            'optionEditPath' => 'civicrm/admin/options/volunteer_role',
          )
        ) ,
        'is_active' => array(
          'name' => 'is_active',
          'type' => CRM_Utils_Type::T_BOOLEAN,
          'title' => ts('Enabled', array('domain' => 'org.civicrm.volunteer')) ,
          'description' => 'Is this need enabled?',
          'required' => true,
          'default' => '1',
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
        ) ,
        'created' => array(
          'name' => 'created',
          'type' => CRM_Utils_Type::T_TIMESTAMP,
          'title' => ts('Date of Creation', array('domain' => 'org.civicrm.volunteer')) ,
          'default' => 'NULL',
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
        ) ,
        'last_updated' => array(
          'name' => 'last_updated',
          'type' => CRM_Utils_Type::T_TIMESTAMP,
          'title' => ts('Date of Last Update', array('domain' => 'org.civicrm.volunteer')) ,
          'default' => 'CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP',
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
        ) ,
        'description' => array(
          'name' => 'description',
          'type' => CRM_Utils_Type::T_TEXT,
          'title' => ts('Description', array('domain' => 'org.civicrm.volunteer')) ,
          'description' => 'Full description of the Volunteer Need. Text and HTML allowed. Displayed on sign-up screens.',
          'required' => false,
          'rows' => 8,
          'cols' => 60,
          'table_name' => 'civicrm_volunteer_need',
          'entity' => 'Need',
          'bao' => 'CRM_Volunteer_DAO_Need',
          'html' => array(
            'type' => 'RichTextEditor',
          ) ,
        ) ,
      );
      CRM_Core_DAO_AllCoreTables::invoke(__CLASS__, 'fields_callback', Civi::$statics[__CLASS__]['fields']);
    }
    return Civi::$statics[__CLASS__]['fields'];
  }
  /**
   * Return a mapping from field-name to the corresponding key (as used in fields()).
   *
   * @return array
   *   Array(string $name => string $uniqueName).
   */
  static function &fieldKeys() {
    if (!isset(Civi::$statics[__CLASS__]['fieldKeys'])) {
      Civi::$statics[__CLASS__]['fieldKeys'] = array_flip(CRM_Utils_Array::collect('name', self::fields()));
    }
    return Civi::$statics[__CLASS__]['fieldKeys'];
  }
  /**
   * Returns the names of this table
   *
   * @return string
   */
  static function getTableName() {
    return self::$_tableName;
  }
  /**
   * Returns if this table needs to be logged
   *
   * @return boolean
   */
  function getLog() {
    return self::$_log;
  }
  /**
   * Returns the list of fields that can be imported
   *
   * @param bool $prefix
   *
   * @return array
   */
  static function &import($prefix = false) {
    $r = CRM_Core_DAO_AllCoreTables::getImports(__CLASS__, 'volunteer_need', $prefix, array());
    return $r;
  }
  /**
   * Returns the list of fields that can be exported
   *
   * @param bool $prefix
   *
   * @return array
   */
  static function &export($prefix = false) {
    $r = CRM_Core_DAO_AllCoreTables::getExports(__CLASS__, 'volunteer_need', $prefix, array());
    return $r;
  }
}