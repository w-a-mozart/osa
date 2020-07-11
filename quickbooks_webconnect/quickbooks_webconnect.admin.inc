<?php

/**
 * Administrative settings
 */
function quickbooks_webconnect_admin_form() {
  $form = array();

  $form['quickbooks_webconnect'] = array(
    '#type' => 'fieldset',
    '#title' => t('QuickBooks Web Connector Service Configuration'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['quickbooks_webconnect']['quickbooks_webconnect_devkit_path'] = array(
    '#type' => 'textfield',
    '#title' => t('QuickBooks PHP DevKit path'),
    '#description' => t('Path to the QuickBooks PHP DevKit from ConsoliBYTE (https://github.com/consolibyte/quickbooks-php)'),
    '#default_value' => variable_get('quickbooks_webconnect_devkit_path', DRUPAL_ROOT . '/sites/all/libraries/quickbooks-php'),
  );

  $form['quickbooks_webconnect']['quickbooks_webconnect_qbxml_version'] = array(
    '#type' => 'select',
    '#title' => t('qbXML version'),
    '#options' => array(
      '5.0' => '5.0',
      '6.0' => '6.0',
      '7.0' => '7.0',
      '8.0' => '8.0',
    ),
    '#default_value' => variable_get('quickbooks_webconnect_qbxml_version', '8.0'),
  );

  $form['quickbooks_webconnect']['quickbooks_webconnect_debug'] = array(
    '#title' => t('Debug logging'),
    '#description' => t('Write debugging information to the watchdog log.'),
    '#type' => 'checkbox',
    '#default_value' => variable_get('quickbooks_webconnect_debug', FALSE),
  );

  $form['quickbooks_webconnect']['quickbooks_webconnect_allowed_ip'] = array(
    '#type' => 'textfield',
    '#title' => t('IP Restriction'),
    '#description' => t('If you want to limit connections to the Web Connector service to a limited set of hosts, enter one or more IP addresses here, separated by commas.'),
    '#default_value' => variable_get('quickbooks_webconnect_allowed_ip'),
  );

  $form['quickbooks_webconnect']['qwc'] = array(
    '#type' => 'fieldset',
    '#title' => t('QuickBooks Web Connector Configuration File Settings'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $name = variable_get('quickbooks_webconnect_app_name', variable_get('site_name', 'Drupal'));
  $form['quickbooks_webconnect']['qwc']['quickbooks_webconnect_app_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Application name'),
    '#description' => t('The name of the application visible to the user. This name is displayed in the QuickBooks Web Connector client.'),
    '#default_value' => $name,
    '#required' => TRUE,
  );

  $form['quickbooks_webconnect']['qwc']['quickbooks_webconnect_app_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Application ID'),
    '#description' => t('The AppID of the application. This can be empty'),
    '#default_value' => variable_get('quickbooks_webconnect_app_id', ''),
  );

  $form['quickbooks_webconnect']['qwc']['quickbooks_webconnect_app_desc'] = array(
    '#type' => 'textfield',
    '#title' => t('Application description'),
    '#description' => t('A brief description of the application to be displayed in the QuickBooks Web Connector client.'),
    '#default_value' => variable_get('quickbooks_webconnect_app_desc', t('QuickBooks Web Connector Service provided by @1', array('@1' => $name))),
    '#required' => TRUE,
  );

  $qb_url = variable_get('quickbooks_webconnect_url', str_replace('http:', 'https:', url('qbwc', array('absolute' => TRUE))));
  $form['quickbooks_webconnect']['qwc']['quickbooks_webconnect_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Web Connector Service URL'),
    '#description' => t('The URL the Web Connector will use to communicate with your Drupal site.  Please note that it MUST be SSL if the hostname is not "localhost"'),
    '#default_value' => $qb_url,
    '#required' => TRUE,
  );

  $form['quickbooks_webconnect']['qwc']['quickbooks_webconnect_app_support'] = array(
    '#type' => 'textfield',
    '#title' => t('Application support URL'),
    '#description' => t('The URL where users can get support for your service.'),
    '#default_value' => variable_get('quickbooks_webconnect_app_support', $qb_url . '/support'),
    '#required' => TRUE,
  );

  $form['quickbooks_webconnect']['qwc']['quickbooks_webconnect_user'] = array(
    '#type' => 'textfield',
    '#title' => t('QuickBooks user'),
    '#description' => t('This is the Drupal user that will be used to authenticate. You can use an existing user, but you probably want to create a user for this purpose.'),
    '#autocomplete_path' => 'user/autocomplete',
    '#default_value' => variable_get('quickbooks_webconnect_user'),
  );

  $form['quickbooks_webconnect']['qwc']['quickbooks_webconnect_company_file'] = array(
    '#type' => 'textfield',
    '#title' => t('Company file'),
    '#description' => t('The full path to your QuickBooks company file, e.g. C:\Data\MyCompany\MyCompany.qdb.  If you leave this blank, applications will use the file that is currently open on your workstation.'),
    '#default_value' => variable_get('quickbooks_webconnect_company_file', ''),
  );

  $options = array(
    0 => t('Manually (No scheduling)'),
    5 => t('Every 5 minutes'),
    15 => t('Every 15 minutes'),
    60 => t('Every hour'),
    240 => t('Every 4 hours'),
    480 => t('Every 8 hours'),
    480 => t('Every 8 hours'),
    720 => t('Every 12 hours'),
    1440 => t('Once per day'),
  );
  $form['quickbooks_webconnect']['qwc']['quickbooks_webconnect_scheduler'] = array(
    '#type' => 'select',
    '#title' => 'Run the Web Connector...',
    '#description' => t('Select the time interval you wish to poll this site for updates.'),
    '#default_value' => variable_get('quickbooks_webconnect_scheduler', 0),
    '#options' => $options,
  );

  $form['quickbooks_webconnect']['qwc']['quickbooks_webconnect_qwc_file'] = array(
    '#type' => 'markup',
    '#markup' => l(t('Download the .QWC file to load into the Web Connector Client'), 'admin/config/quickbooks_webconnect/qwc'),
  );

  return system_settings_form($form);
}

/*
 * Use the server settings provided by the Services module
 */
function _quickbooks_webconnect_settings(&$form, $endpoint, $settings) {
  $form = $form + quickbooks_webconnect_admin_form();
  unset($form['actions']);
}

function _quickbooks_webconnect_settings_submit($endpoint, &$values) {

  foreach ($values['quickbooks_webconnect'] as $key => $value) {
    _quickbooks_webconnect_variable_set($key, $value);
  }
  
  return NULL;
}

function _quickbooks_webconnect_variable_set($name, $value) {
  if (is_array($value)) {
    if (strncmp($name, 'quickbooks_webconnect_', 22) === 0) {
      $value = array_keys(array_filter($value));
      variable_set($name, $value);
    }
    else {
      foreach ($value as $k => $v) {
        _quickbooks_webconnect_variable_set($k, $v);
      }
    }
  }
  else {
    variable_set($name, $value);
  }
}

/*
 * Generate and download the .QWC file
 */
function quickbooks_webconnect_qwc() {
  $app_name = variable_get('quickbooks_webconnect_app_name');
  if (!isset($app_name)) {
    drupal_set_message(t('You must save the configuration settings prior to generating the QWC file.'), 'error');
    drupal_goto('admin/config/quickbooks_webconnect');
    return;
  }
  
  require_once 'quickbooks_webconnect.devkit.inc';
  $qwc = new QuickBooks_WebConnector_QWC(
    variable_get('quickbooks_webconnect_app_name'),
		variable_get('quickbooks_webconnect_app_desc'),
		variable_get('quickbooks_webconnect_url'),
		variable_get('quickbooks_webconnect_app_support'),
		variable_get('quickbooks_webconnect_user'),
		QuickBooks_WebConnector_QWC::fileID(),
		QuickBooks_WebConnector_QWC::ownerID(),
		QUICKBOOKS_TYPE_QBFS,
		FALSE,
		variable_get('quickbooks_webconnect_scheduler') * 60, //seconds
		QUICKBOOKS_PERSONALDATA_DEFAULT,
		QUICKBOOKS_UNATTENDEDMODE_DEFAULT,
		QUICKBOOKS_SUPPORTED_DEFAULT,
		FALSE,
		NULL,
		NULL,
		variable_get('quickbooks_webconnect_app_id', '')
  );

  ob_end_clean();
  $qwc->http();
  exit;
}
