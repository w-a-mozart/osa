{*
 +--------------------------------------------------------------------+
 | CiviCRM version 4.5                                                |
 +--------------------------------------------------------------------+
 | Copyright CiviCRM LLC (c) 2004-2014                                |
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
{* Add 'required' marker to billing fields in this template for front-end / online contribution and event registration forms only. *}
{if $context EQ 'front-end'}
  {assign var=reqMark value=' <span class="crm-marker" title="This field is required.">*</span>'}
{else}
  {assign var=reqMark value=''}
{/if}

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
                             <div class="label">{$form.credit_card_type.label} {$reqMark}</div>
                             <div class="content">
                                 {$form.credit_card_type.html}
                                 <div class="crm-credit_card_type-icons"></div>
                             </div>
                             <div class="clear"></div>
                        </div>
                        <div class="crm-section {$form.credit_card_number.name}-section">
                             <div class="label">{$form.credit_card_number.label} {$reqMark}</div>
                             <div class="content">{$form.credit_card_number.html|crmAddClass:creditcard}</div>
                             <div class="clear"></div>
                        </div>
{* Remove CVV2 field *}
                        <div class="crm-section {$form.credit_card_exp_date.name}-section">
                            <div class="label">{$form.credit_card_exp_date.label} {$reqMark}</div>
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

CRM.$(function($) {
  // build list of ids to track changes on
  var address_fields = {/literal}{$profileAddressFields|@json_encode}{literal};
  var input_ids = {};
  var select_ids = {};
  var orig_id, field, field_name;

  // build input ids
  $('.billing_name_address-section input').each(function(i){
    orig_id = $(this).attr('id');
    field = orig_id.split('-');
    field_name = field[0].replace('billing_', '');
    if(field[1]) {
      if(address_fields[field_name]) {
        input_ids['#'+field_name+'-'+address_fields[field_name]] = '#'+orig_id;
      }
    }
  });
  if($('#first_name').length)
    input_ids['#first_name'] = '#billing_first_name';
  if($('#middle_name').length)
    input_ids['#middle_name'] = '#billing_middle_name';
  if($('#last_name').length)
    input_ids['#last_name'] = '#billing_last_name';

  // build select ids
  $('.billing_name_address-section select').each(function(i){
    orig_id = $(this).attr('id');
    field = orig_id.split('-');
    field_name = field[0].replace('billing_', '').replace('_id', '');
    if(field[1]) {
      if(address_fields[field_name]) {
        select_ids['#'+field_name+'-'+address_fields[field_name]] = '#'+orig_id;
      }
    }
  });

  // detect if billing checkbox should default to checked
  var checked = true;
  for(var id in input_ids) {
    orig_id = input_ids[id];
    if($(id).val() != $(orig_id).val()) {
      checked = false;
      break;
    }
  }
  for(var id in select_ids) {
    orig_id = select_ids[id];
    if($(id).val() != $(orig_id).val()) {
      checked = false;
      break;
    }
  }
  if(checked) {
    $('#billingcheckbox').prop('checked', true);
    if (!CRM.billing || CRM.billing.billingProfileIsHideable) {
      $('.billing_name_address-group').hide();
    }
  }

  // onchange handlers for non-billing fields
  for(var id in input_ids) {
    orig_id = input_ids[id];
    $(id).change(function(){
      var id = '#'+$(this).attr('id');
      var orig_id = input_ids[id];

      // if billing checkbox is active, copy other field into billing field
      if($('#billingcheckbox').prop('checked')) {
        $(orig_id).val( $(id).val() );
      }
    });
  }
  for(var id in select_ids) {
    orig_id = select_ids[id];
    $(id).change(function(){
      var id = '#'+$(this).attr('id');
      var orig_id = select_ids[id];

      // if billing checkbox is active, copy other field into billing field
      if($('#billingcheckbox').prop('checked')) {
        $(orig_id+' option').prop('selected', false);
        $(orig_id+' option[value="'+$(id).val()+'"]').prop('selected', true);
      }

      if(orig_id == '#billing_country_id-5') {
        $(orig_id).change();
      }
    });
  }


  // toggle show/hide
  $('#billingcheckbox').click(function(){
    if(this.checked) {
      if (!CRM.billing || CRM.billing.billingProfileIsHideable) {
        $('.billing_name_address-group').hide(200);
      }

      // copy all values
      for(var id in input_ids) {
        orig_id = input_ids[id];
        $(orig_id).val( $(id).val() );
      }
      for(var id in select_ids) {
        orig_id = select_ids[id];
        $(orig_id+' option').prop('selected', false);
        $(orig_id+' option[value="'+$(id).val()+'"]').prop('selected', true);
      }
    } else {
      $('.billing_name_address-group').show(200);
    }
  });

  // remove spaces, dashes from credit card number
  $('#credit_card_number').change(function(){
    var cc = $('#credit_card_number').val()
      .replace(/ /g, '')
      .replace(/-/g, '');
    $('#credit_card_number').val(cc);
  });
});
{/literal}
</script>
{/if}
{/if}
{/if} {* $paymentProcessor.payment_processor_type neq 'drupalcommerce' *}
{/crmRegion}
