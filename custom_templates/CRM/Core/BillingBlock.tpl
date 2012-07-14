{*
 +--------------------------------------------------------------------+
 | Custom OSA Credit Card Billing Block                               |
 +--------------------------------------------------------------------+
 | Override Standard CiviCRM Template BillingBlock.tpl                |
 | - remove Security Code and Billing Address fields                  |
 +--------------------------------------------------------------------+
 | Copyright Oakville Suzuki Association 2012                         |
 | Copyright CiviCRM LLC (c) 2004-2011                                |
 +--------------------------------------------------------------------+
*}
{if $form.credit_card_number or $form.bank_account_number}
    <div id="payment_information">

{* Move legend to Main.tpl *}

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
{/if}