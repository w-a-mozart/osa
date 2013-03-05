{*
 +--------------------------------------------------------------------+
 | CiviCRM version 4.2                                                |
 +--------------------------------------------------------------------+
 | Copyright CiviCRM LLC (c) 2004-2012                                |
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
*}
{crmRegion name="billing-block"}
{* Move pay later from Main.tpl *}

{if $paymentProcessor.payment_processor_type neq 'drupalcommerce'} 
  <fieldset class="billing_mode-group {if $paymentProcessor.payment_type & 2}direct_debit_info-group{else}credit_card_info-group{/if}">
    <legend>{ts}Payment Information{/ts}</legend>
  </fieldset>

	{if $form.is_pay_later}
	    <div class="crm-section {$form.is_pay_later.name}-section">
      {* Change Pay Later to Radio Buttons to look like Commerce Payment *}
			  <div class="content">
          <input type="radio" name="is_pay_later" value="0" onclick="return showHideByValue('is_pay_later','','payment_information','block','radio',false);" checked /> {if $paymentProcessor.payment_type & 2}{ts}Pay by Debit{/ts}{else}{ts}Pay by Credit Card{/ts}{/if}<br />
          <input type="radio" name="is_pay_later" value="1" onclick="return showHideByValue('is_pay_later','','payment_information','block','radio',false);" /> {ts}Pay by Cheque{/ts}
        </div>
	    </div>
	{/if} 


{if $form.credit_card_number or $form.bank_account_number}
    <div id="payment_information">

{* Move legend above payment radio buttons *}

            {if $paymentProcessor.billing_mode & 2 and !$hidePayPalExpress }
            <div class="crm-section no-label paypal_button_info-section">
			    <div class="content description">
			        {ts}If you have a PayPal account, you can click the PayPal button to continue. Otherwise, fill in the credit card and billing information on this form and click <strong>Continue</strong> at the bottom of the page.{/ts}
				</div>
			</div>
			 <div class="crm-section no-label {$form.$expressButtonName.name}-section">
			    <div class="content description">
			        {$form.$expressButtonName.html}
			        <div class="description">Save time. Checkout securely. Pay without sharing your financial information. </div>
				</div>
			</div>
            {/if}

            {if $paymentProcessor.billing_mode & 1}
                <div class="crm-section billing_mode-section {if $paymentProcessor.payment_type & 2}direct_debit_info-section{else}credit_card_info-section{/if}">
                   {if $paymentProcessor.payment_type & 2}
                        <div class="crm-section {$form.account_holder.name}-section">
							<div class="label">{$form.account_holder.label}</div>
                            <div class="content">{$form.account_holder.html}</div>
                            <div class="clear"></div>
                        </div>
                        <div class="crm-section {$form.bank_account_number.name}-section">
							<div class="label">{$form.bank_account_number.label}</div>
                            <div class="content">{$form.bank_account_number.html}</div>
                            <div class="clear"></div>
                        </div>
                        <div class="crm-section {$form.bank_identification_number.name}-section">
							<div class="label">{$form.bank_identification_number.label}</div>
                            <div class="content">{$form.bank_identification_number.html}</div>
                            <div class="clear"></div>
                        </div>
                        <div class="crm-section {$form.bank_name.name}-section">
							<div class="label">{$form.bank_name.label}</div>
                            <div class="content">{$form.bank_name.html}</div>
                            <div class="clear"></div>
                        </div>
                   {else}
                		<div class="crm-section {$form.credit_card_type.name}-section">
							<div class="label">{$form.credit_card_type.label}</div>
                			<div class="content">{$form.credit_card_type.html}</div>
                			<div class="clear"></div>
                		</div>
                		<div class="crm-section {$form.credit_card_number.name}-section">
							<div class="label">{$form.credit_card_number.label}</div>
                			<div class="content">{$form.credit_card_number.html}
                				<div class="description">{ts}Enter numbers only, no spaces or dashes.{/ts}</div>
                			</div>
                			<div class="clear"></div>
                		</div>
{* Remove CVV2 field *}
                		<div class="crm-section {$form.credit_card_exp_date.name}-section">
							<div class="label">{$form.credit_card_exp_date.label}</div>
                			<div class="content">{$form.credit_card_exp_date.html}</div>
                			<div class="clear"></div>
                		</div>
                    {/if}
                </div>
                </fieldset>

{* Remove Billing Address field *}

            {else}
                </fieldset>
            {/if}
    </div>

{if $profileAddressFields}
<script type="text/javascript">
{literal}
cj( function( ) {
  cj('#billingcheckbox').click( function( ) {
    sameAddress( this.checked ); // need to only action when check not when toggled, can't assume desired behaviour
  });
});

function sameAddress( setValue ) {
  {/literal}
  var addressFields = {$profileAddressFields|@json_encode};
  {literal}
  var locationTypeInProfile = 'Primary';
  var orgID = field = fieldName = null;
  if ( setValue ) {
    cj('.billing_name_address-section input').each( function( i ){
      orgID = cj(this).attr('id');
      field = orgID.split('-');
      fieldName = field[0].replace('billing_', '');
      if ( field[1] ) { // ie. there is something after the '-' like billing_street_address-5
        // this means it is an address field
        if ( addressFields[fieldName] ) {
          fieldName =  fieldName + '-' + addressFields[fieldName];
        }
      }
      cj(this).val( cj('#' + fieldName ).val() );
    });
    
    var stateId;
    cj('.billing_name_address-section select').each( function( i ){
      orgID = cj(this).attr('id');
      field = orgID.split('-');
      fieldName = field[0].replace('billing_', '');
      fieldNameBase = fieldName.replace('_id', '');
      if ( field[1] ) { 
        // this means it is an address field
        if ( addressFields[fieldNameBase] ) {
          fieldName =  fieldNameBase + '-' + addressFields[fieldNameBase];
        }
      }

      // don't set value for state-province, since
      // if need reload state depending on country
      if ( fieldNameBase == 'state_province' ) {
        stateId = cj('#' + fieldName ).val();
      }
      else {
        cj(this).val( cj('#' + fieldName ).val() ).change( );   
      }
    });

    // now set the state province
    // after ajax call loads all the states
    if ( stateId ) {
      cj('select[id^="billing_state_province_id"]').ajaxStop(function() {
        cj( 'select[id^="billing_state_province_id"]').val( stateId );
      });
    }  
  }
}
{/literal}
</script>
{/if}
{/if}
{/if} {* $paymentProcessor.payment_processor_type neq 'drupalcommerce' *}
{/crmRegion}
