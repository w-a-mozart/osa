{*
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
*}
{crmRegion name="billing-block"}
{* Move pay later from Main.tpl *}

{if $paymentProcessor.payment_processor_type neq 'drupalcommerce'}
    <fieldset class="billing_mode-group {$paymentTypeName}_info-group">
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
  {if $paymentFields|@count}
    <div id="payment_information">
{* Move legend above payment radio buttons *}
      {crmRegion name="billing-block-pre"}
      {/crmRegion}
      <div class="crm-section billing_mode-section {$paymentTypeName}_info-section">
        {foreach from=$paymentFields item=paymentField}
          {assign var='name' value=$form.$paymentField.name}
          <div class="crm-section {$form.$paymentField.name}-section">
            <div class="label">{$form.$paymentField.label}
              {if $requiredPaymentFields.$name}<span class="crm-marker" title="{ts}This field is required.{/ts}">*</span>{/if}
            </div>
            <div class="content">{$form.$paymentField.html}
              {if $paymentField == 'cvv2'}{* @todo move to form assignment*}
                <span class="cvv2-icon" title="{ts}Usually the last 3-4 digits in the signature area on the back of the card.{/ts}"> </span>
              {/if}
              {if $paymentField == 'credit_card_type'}
                <div class="crm-credit_card_type-icons"></div>
              {/if}
            </div>
            <div class="clear"></div>
          </div>
        {/foreach}
      </div>
    </div>
  {/if}

{* Remove Billing Address field *}

  <script type="text/javascript">
    {literal}
    CRM.$(function ($) {
      // remove spaces, dashes from credit card number
      $('#credit_card_number').change(function () {
        var cc = $('#credit_card_number').val()
                .replace(/ /g, '')
                .replace(/-/g, '');
        $('#credit_card_number').val(cc);
      });
    });
  </script>
  {/literal}
{/if} {* $paymentProcessor.payment_processor_type neq 'drupalcommerce' *}
{if $suppressSubmitButton}
{literal}
  <script type="text/javascript">
    CRM.$(function($) {
      $('.crm-submit-buttons', $('#billing-payment-block').closest('form')).hide();
    });
  </script>
{/literal}
{/if}
{/crmRegion}
{crmRegion name="billing-block-post"}
  {* Payment processors sometimes need to append something to the end of the billing block. We create a region for
     clarity  - the plan is to move to assigning this through the payment processor to this region *}
{/crmRegion}

