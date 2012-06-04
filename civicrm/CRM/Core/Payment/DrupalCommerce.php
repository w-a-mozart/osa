<?php 

/**
 * @file
 * CiviCRM Payment Processor to redirect purchases of Memberships, Event Registrations and Donations to Drupal Commerce
 *
 */
require_once 'CRM/Core/Payment.php';

class CRM_Core_Payment_DrupalCommerce extends CRM_Core_Payment { 

    /**
     * Use the singleton pattern
     *
     * @var object
     * @static
     */
    static private $_singleton = null;

    /**
     * mode of operation: live or test
     * - not used as "real" transactions are handled by Drupal Commerce
     *   retained only to be consistent with other processors
     *
     * @var object
     * @static
     */
    static protected $_mode = null;

    /** 
     * Constructor 
     *
     * @param string $mode the mode of operation: live or test
     * 
     * @return void 
     */ 
    function __construct( $mode, &$paymentProcessor ) {
        $this->_mode             = $mode;
        $this->_paymentProcessor = $paymentProcessor;
        $this->_processorName    = ts('Drupal Commerce Cart');
    }

    /** 
     * singleton function used to manage this object 
     * 
     * @param string $mode the mode of operation: live or test
     *
     * @return object 
     * @static 
     * 
     */ 
    static function &singleton( $mode, &$paymentProcessor ) {
        $processorName = $paymentProcessor['name'];
        if (self::$_singleton[$processorName] === null ) {
            self::$_singleton[$processorName] = new CRM_Core_Payment_DrupalCommerce( $mode, $paymentProcessor );
        }
        return self::$_singleton[$processorName];
    }


    /**  
     * Transfers the item to the Drupal Commerce Cart for later checkout
     *  
     * @param array $params  name value pair of contribution data
     *  
     * @return void  
     * @access public 
     *  
     */  
    function doTransferCheckout( &$params, $component ) {
        $component = strtolower( $component );
        
        $dbgtxt = "doTransferCheckout";
        $dbgtxt .= "\nparams=" . var_export($params,1);
        $dbgtxt .= "\ncomponent=" . var_export($component,1);
        watchdog('osa',"<pre>" . var_export($dbgtxt,1) . "</pre>");
    

        // Allow manipulation of the arguments via custom hooks
        $cookedParams = $params; // no translation in this processor
        CRM_Utils_Hook::alterPaymentProcessorParams( $this, $params, $cookedParams );

        // Create a new shopping cart line item
        module_load_include('php', 'civicrm_commerce', 'civicrm_line_item');
        $result = civicrm_commerce_line_item_add_new( $params, $component );
        if ( $result ) {
            drupal_set_message("Added to Cart", 'status', FALSE);
        }
    }

    /** 
     * Called by framework to see if we have the right config values
     * - no config needed
     * 
     * @return string the error message if any 
     * @public 
     */ 
    function checkConfig() {
        return null;
    }

    /**
     * Abstract functon of the parent class called by the framework for direct payment processing
     * - not implemented for this payment processor type (BILLING_MODE_NOTIFY)
     *
     * @param  array $params 
     * @public
     */
    function doDirectPayment( &$params ) {
        CRM_Core_Error::fatal( ts( 'This function is not implemented' ) );
    }
}
