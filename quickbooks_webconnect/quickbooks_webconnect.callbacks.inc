<?php
/*
 * QuickBooks RPC callbacks
 *
 * See http://developer.intuit.com/qbsdk-current/doc/pdf/qbwc_proguide.pdf
 */
require_once 'quickbooks_webconnect.inc';

/*
 * Return the server version to the client. Runs pre-authentication.
 */
function quickbooks_webconnect_serverVersion($request) {
  _quickbooks_webconnect_debug('[serverVersion] called with : @request', array('@request' => var_export($request, TRUE)));
  $info = system_get_info('module', 'quickbooks_webconnect');
  $response = new stdClass();
  $response->serverVersionResult = $info['version'];
  return $response;
}

/*
 * Return the client version to the server. Runs pre-authentication.
 */
function quickbooks_webconnect_clientVersion($request) {
  _quickbooks_webconnect_debug('[clientVersion] called with : @request', array('@request' => var_export($request, TRUE)));
  $response = new stdClass();
  $response->clientVersionResult = '';
  return $response;
}

/*
 * Authenticate client. Returns a drupal session id as a unique token.
 */
function quickbooks_webconnect_authenticate($request) {
  _quickbooks_webconnect_debug('[authenticate] called with : @request', array('@request' => var_export($request, TRUE))); 

  module_load_include('inc', 'services', 'resources/user_resource');
  try {
    drupal_session_start();
    $login = _user_resource_login($request->strUserName, $request->strPassword);
  }
  catch (Exception $e) {
    watchdog('quickbooks_webconnect', $e->getMessage(), array(), WATCHDOG_ERROR);
  }

  if (isset($login->user)) {
    // check if this user has access to this service
    if (user_access('access quickbooks webconnect server')) {
    
      // check if there is anything to do
      $queue = DrupalQueue::get('quickbooks_webconnect');
      $return = ($queue->numberOfItems() > 0) ? variable_get('quickbooks_webconnect_company_file', '') : 'none';
      $result = array($login->sessid, $return);
      
      // initialize session variables
      $_SESSION['quickbooks_webconnect_item'] = NULL;
      $_SESSION['quickbooks_webconnect_counter'] = 0;
      $_SESSION['quickbooks_webconnect_error'] = NULL;
      
      drupal_save_session(TRUE);
      drupal_session_commit();
    }
    else {
      watchdog('access denied', 'quickbooks_webconnect', NULL, WATCHDOG_WARNING);
      _user_resource_logout();
      $result = array(session_id(), 'nvu');
    }
  }
  else {
    $result = array(session_id(), 'nvu');
  }

  $response = new stdClass();
  $response->authenticateResult = $result;
  return $response;
}

/*
 * Sends data to the QuickBooks Webconnect client using QBXML. Requires authentication.
 *
 * See https://member.developer.intuit.com/qbSDK-current/Common/newOSR/index.html (use
 * IE to view this page).
 */
function quickbooks_webconnect_sendRequestXML($request) {
  _quickbooks_webconnect_debug('[sendRequestXML] called with : @request', array('@request' => var_export($request, TRUE))); 
  if (!_quickbooks_webconnect_check_session($request->ticket)) {
    return false;
  }
  // default to blank string. QB doc says this is fine, but it will generate a call to getLastError
  $result = '';

  // get the next item to send to QuickBooks
  $queue = DrupalQueue::get('quickbooks_webconnect');
  $item = $queue->claimItem();

  if ($item) {
    _quickbooks_webconnect_debug('[sendRequestXML] claimItem : @item', array('@item' => var_export($item, TRUE)));

    // create a QBXML object
    $qb_obj = _quickbooks_webconnect_transform($item->data['element'], $item->data['type'], $item->data['op']);
    _quickbooks_webconnect_debug('[sendRequestXML] QB_OBJ : @qb_obj', array('@qb_obj' => var_export($qb_obj, TRUE)));

    // generate the QBXML
    $request = $item->data['type'] . $item->data['op'];
    $qbxml = $qb_obj->asQBXML($request, variable_get('quickbooks_webconnect_qbxml_version'));

    // allow modules to alter the QBXML
    drupal_alter('quickbooks_webconnect_qbxml', $qbxml, $item->data['element'], $item->data['type'] );
    $result = _quickbooks_webconnect_wrap_qbxml($qbxml);
    _quickbooks_webconnect_debug('[sendRequestXML] qbXML : @qbxml', array('@qbxml' => var_export($result, TRUE)));
  }
  else {
    // create a 'NULL' item
    $item = array();
  }
  
  // save the item in the session for when the response is returned.
  $_SESSION['quickbooks_webconnect_item'] = serialize($item);
  drupal_save_session(TRUE);
  drupal_session_commit();
      
  $response = new stdClass();
  $response->sendRequestXMLResult = $result;
  return $response;
}

/*
 * Process the response from the QuickBooks Webconnect client for the last sendRequestXML. Requires authentication.
 */
function quickbooks_webconnect_receiveResponseXML($request) {
  _quickbooks_webconnect_debug('[receiveResponseXML] called with : @request', array('@request' => var_export($request, TRUE))); 
  if (!_quickbooks_webconnect_check_session($request->ticket)) {
    return false;
  }

  _quickbooks_webconnect_debug('[receiveResponseXML] [_SESSION] : @session', array('@session' => var_export($_SESSION, TRUE))); 

  // get the item this response is for, from the session.
  $item = unserialize($_SESSION['quickbooks_webconnect_item']);
  $_SESSION['quickbooks_webconnect_item'] = NULL;
  $retry = FALSE;
  
  try {
    if (!$item) {
      throw new Exception('[receiveResponseXML] message received out of order');
    }

    // check for errors from QuickBooks
    if ($request->hresult) {
      watchdog('quickbooks_webconnect', '[receiveResponseXML] received error @hresult : @message, for item @item', array('@hresult' => $request->hresult, '@message' => $request->message, '@item' => var_export($item, TRUE)), WATCHDOG_ERROR);
    }

    // check for errors in the response
    if ($request->response) {
      $doc = DOMDocument::loadXML($request->response);
      if (!$doc) {
        throw new Exception('[receiveResponseXML] invalid XML:' . $request->response);
      }
      $xpath = new DOMXPath($doc);
      $rs_node = $xpath->query('/QBXML/QBXMLMsgsRs/*')->item(0);
      $statusCode = $rs_node->getAttribute('statusCode');
      $statusMessage = $rs_node->getAttribute('statusMessage');

      // 3180 is an internal locking issue with QuickBooks, so we will retry
      if ($statusCode == '3180') {
        $retry = TRUE;
      }
      elseif (($item->data['op'] == 'Add') && ($statusCode == '3100')) {
        // ignore "item already exists" error
      }
      elseif ($statusCode !== '0') {
        watchdog('quickbooks_webconnect', '[receiveResponseXML] received QBXML error @statusCode : @statusMessage, for item @item', array('@statusCode' => $statusCode, '@statusMessage' => $statusMessage, '@item' => var_export($item, TRUE)), WATCHDOG_ERROR);
      }
      else {
        // create the QuickBooks_QBXML_Object from the QBXML
        require_once 'quickbooks_webconnect.devkit.inc';
        $ret_node = $xpath->query('/QBXML/QBXMLMsgsRs/*/*')->item(0);
        $qbxml  = $doc->saveXML($ret_node);
        $qb_obj = QuickBooks_QBXML_Object::fromQBXML($qbxml);
        
        // let other modules process the response
        module_invoke_all('quickbooks_webconnect_process_response', $qbxml, $qb_obj, $item->data['element'], $item->data['type'], $item->data['op']);
      }
    }

    $queue = DrupalQueue::get('quickbooks_webconnect');
    if ($retry) {
      // release the lock on the item
      $queue->releaseItem($item);
    }
    else {
      // remove the item from further processing - we'll have to re-queue to send again 
      $queue->deleteItem($item);
      $count = isset($_SESSION['quickbooks_webconnect_counter']) ? $_SESSION['quickbooks_webconnect_counter'] + 1: 1;
      $_SESSION['quickbooks_webconnect_counter'] = $count;
    }

    // set the return parameter
    $num_items = $queue->numberOfItems();
    if ($queue->numberOfItems() > 0) {
      $return = max(1, (int)(($count * 100) / ($count + $num_items)));
    }
    else {
      $return = 100;
    }
  }
  catch (Exception $exception) {
      $_SESSION['quickbooks_webconnect_error'] = 'Error returned by ' . variable_get('quickbooks_webconnect_app_name', 'SOAP service') . '. See server error log.';
      watchdog('quickbooks_webconnect', $exception->getMessage(), array(), WATCHDOG_ERROR);
      $return = -1;
  }

  // save the session data
  drupal_save_session(TRUE);
  drupal_session_commit();
  
  $response = new stdClass();
  $response->receiveResponseXMLResult = $return;
  return $response;
}

/*
 * Close the connection to the QuickBooks Webconnect client and terminate the session.
 */
function quickbooks_webconnect_closeConnection($request) {
  global $user;
  _quickbooks_webconnect_debug('[closeConnection] called with : @request', array('@request' => var_export($request, TRUE))); 
  if (!_quickbooks_webconnect_check_session($request->ticket)) {
    return false;
  }

  module_load_include('inc', 'services', 'resources/user_resource');
  try {
    drupal_save_session(TRUE);
    drupal_session_destroy_uid($user->uid);
    _user_resource_logout();
  }
  catch (Exception $e) {
    watchdog('quickbooks_webconnect', $e->getMessage(), array(), WATCHDOG_ERROR);
  }

  // allow connection to close
  $response = new stdClass();
  $response->closeConnectionResult = 'OK';
  return $response;
}

/*
 * Log errors returned by the QBWC client. Requires authentication.
 */
function quickbooks_webconnect_connectionError($request) {
  _quickbooks_webconnect_debug('[connectionError] called with : @request', array('@request' => var_export($request, TRUE)));
  if (!_quickbooks_webconnect_check_session($request->ticket)) {
    return false;
  }

  watchdog('quickbooks_webconnect', '[connectionError] called with : @request', array('@request' => var_export($request, TRUE)), WATCHDOG_ERROR);

  // allow connection to close
  $response = new stdClass();
  $response->connectionErrorResult = 'DONE';
  return $response;
}

/*
 * Called if sendRequestXML or receiveResponseXML returns an error code. Requires authentication.
 */
function quickbooks_webconnect_getLastError($request) {
  _quickbooks_webconnect_debug('[getLastError] called with : @request', array('@request' => var_export($request, TRUE))); 
  if (!_quickbooks_webconnect_check_session($request->ticket)) {
    return false;
  }

  $response = new stdClass();
  $last_error = $_SESSION['quickbooks_webconnect_error'];
  if ($last_error) {
    $response->getLastErrorResult = $last_error;
    $_SESSION['quickbooks_webconnect_error'] = NULL;
  }
  else {
    $response->getLastErrorResult = NULL;
  }
  
  return $response;
}

/* Helper functions */

/*
 * Authentication and authorization of the session token
 */
function _quickbooks_webconnect_check_session($sid) {
  global $user;

  // create a dummy cookie and load the session
  $_COOKIE[session_name()] = $sid;
  $_COOKIE[substr(session_name(), 1)] = $sid;
  
  session_id($sid);
  drupal_session_start();
  _drupal_session_read($sid);
    
  if (!user_access('access quickbooks webconnect server')) {
    drupal_add_http_header('Status', '403 Forbidden');
    watchdog('quickbooks_webconnect', 'Invalid user session sid = %sid', array('%sid' => $sid), WATCHDOG_ERROR);
    return FALSE;
  }

  return TRUE;
}
