<?php
/*
 * Code to include the QuickBooks PHP DevKit from ConsoliBYTE.
 * See http://consolibyte.com/quickbooks-open-source/
 * Download from http://consolibyte.com/downloads/quickbooks-php-devkit/
 */

// set config options
define('QUICKBOOKS_FRAMEWORKS', 0);
define('QUICKBOOKS_LOADER_REQUIREONCE', TRUE);
define('QUICKBOOKS_LOADER_AUTOLOADER', TRUE);

// include the library
$devkit_path = variable_get('quickbooks_webconnect_devkit_path', DRUPAL_ROOT . '/sites/all/libraries/quickbooks-php');
require_once $devkit_path . '/QuickBooks.php';
