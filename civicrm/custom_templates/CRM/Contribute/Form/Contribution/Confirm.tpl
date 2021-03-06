{*
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
*}
{if $action & 1024}
  {include file="CRM/Contribute/Form/Contribution/PreviewHeader.tpl"}
{/if}

{include file="CRM/common/TrackingFields.tpl"}

<div class="crm-contribution-page-id-{$contributionPageID} crm-block crm-contribution-confirm-form-block">
  <div class="help">
    <p>{ts}Please verify the information below carefully. Click <strong>Go Back</strong> if you need to make changes.{/ts}
      {$continueText}
    </p>
  </div>
  <div id="crm-submit-buttons" class="crm-submit-buttons">
    {include file="CRM/common/formButtons.tpl" location="top"}
  </div>
{* Move pay later instructions *}

  {include file="CRM/Contribute/Form/Contribution/MembershipBlock.tpl" context="confirmContribution"}

{* Show who the contribution is for. *}
{if $onbehalfFamily}
  <div class="crm-section contact-id-section">
    <fieldset class="label-left">
      <div class="header-dark">Family Member </div>
      <div class="label">{$contact.contact_type}</div>
      <div class="content">
        {$contact.display_name}
      </div>
    </fieldset>
  </div>
  <div class="clear"></div> 
{/if}
    
{* Move custom fields *}
    {if $customPre}
            <fieldset class="label-left">
                {include file="CRM/UF/Form/Block.tpl" fields=$customPre}
{*            </fieldset> *}
    {/if}

  {if $amount GTE 0 OR $minimum_fee GTE 0 OR ( $isDisplayLineItems and $lineItem ) }
    <div class="crm-group amount_display-group">
      {if !$useForMember}
        <div class="header-dark">
          {if !$membershipBlock AND $amount OR ( $isDisplayLineItems and $lineItem ) }{ts}Contribution Amount{/ts}{else}{ts}Membership Fee{/ts} {/if}
        </div>
      {/if}
      <div class="display-block">
        {if !$useForMember}
          {if $lineItem and $isDisplayLineItems}
            {if !$amount}{assign var="amount" value=0}{/if}
            {assign var="totalAmount" value=$amount}
            {include file="CRM/Price/Page/LineItem.tpl" context="Contribution"}
          {elseif $is_separate_payment }
            {if $amount AND $minimum_fee}
              {$membership_name} {ts}Membership{/ts}:
              <strong>{$minimum_fee|crmMoney}</strong>
              <br/>
              {ts}Additional Contribution{/ts}:
              <strong>{$amount|crmMoney}</strong>
              <br/>
              <strong> -------------------------------------------</strong>
              <br/>
              {ts}Total{/ts}:
              <strong>{$amount+$minimum_fee|crmMoney}</strong>
              <br/>
            {elseif $amount }
              {ts}Amount{/ts}:
              <strong>{$amount|crmMoney} {if $amount_level }<span class='crm-price-amount-label'>
                  &ndash; {$amount_level}</span>{/if}</strong>
            {else}
              {$membership_name} {ts}Membership{/ts}:
              <strong>{$minimum_fee|crmMoney}</strong>
            {/if}
          {else}
            {if $totalTaxAmount }
              {ts 1=$taxTerm}Total %1 Amount{/ts}:
              <strong>{$totalTaxAmount|crmMoney} </strong>
              <br/>
            {/if}
            {if $amount}
              {if $installments}{ts}Installment Amount{/ts}{else}{ts}Total Amount{/ts}{/if}:
              <strong>{$amount|crmMoney}{if $amount_level }<span class='crm-price-amount-label'>
                  &ndash; {$amount_level}</span>{/if}</strong>
            {else}
              {$membership_name} {ts}Membership{/ts}:
              <strong>{$minimum_fee|crmMoney}</strong>
            {/if}
          {/if}
        {/if}

        {if $is_recur}
          {if !empty($auto_renew)} {* Auto-renew membership confirmation *}
            {crmRegion name="contribution-confirm-recur-membership"}
              <br/>
              <strong>
                {if $autoRenewOption == 1}
                  {ts 1=$frequency_interval 2=$frequency_unit}I want this membership to be renewed automatically every %1 %2(s).{/ts}
                {elseif $autoRenewOption == 2}
                  {ts 1=$frequency_interval 2=$frequency_unit}This membership will be renewed automatically every %1 %2(s).{/ts}
                {/if}
              </strong>
              </p>
              <div class="description crm-auto-renew-cancel-info">
                ({ts}Your initial membership fee will be processed once you complete the confirmation step. You will be able to cancel the auto-renewal option by visiting the web page link that will be included in your receipt.{/ts}
                )
              </div>
            {/crmRegion}
          {else}
            {crmRegion name="contribution-confirm-recur"}
            {if $installments}
              {if $frequency_interval > 1}
                <p>
                  <strong>{ts 1=$frequency_interval 2=$frequency_unit 3=$installments}I want to contribute this amount every %1 %2s for %3 installments.{/ts}</strong>
                </p>
              {else}
                <p>
                  <strong>{ts 1=$frequency_unit 2=$installments}I want to contribute this amount every %1 for %2 installments.{/ts}</strong>
                </p>
              {/if}
            {else}
              {if $frequency_interval > 1}
                <p>
                  <strong>{ts 1=$frequency_interval 2=$frequency_unit}I want to contribute this amount every %1 %2s.{/ts}</strong>
                </p>
              {else}
                <p><strong>{ts 1=$frequency_unit }I want to contribute this amount every %1.{/ts}</strong></p>
              {/if}
            {/if}
              <p>{ts}Your initial contribution will be processed once you complete the confirmation step. You will be able to cancel the recurring contribution by visiting the web page link that will be included in your receipt.{/ts}</p>
            {/crmRegion}
          {/if}
        {/if}

        {if $is_pledge }
          {if $pledge_frequency_interval GT 1}
            <p>
              <strong>{ts 1=$pledge_frequency_interval 2=$pledge_frequency_unit 3=$pledge_installments}I pledge to contribute this amount every %1 %2s for %3 installments.{/ts}</strong>
            </p>
          {else}
            <p>
              <strong>{ts 1=$pledge_frequency_interval 2=$pledge_frequency_unit 3=$pledge_installments}I pledge to contribute this amount every %2 for %3 installments.{/ts}</strong>
            </p>
          {/if}
          {if $is_pay_later}
            <p>{ts 1=$receiptFromEmail 2=$button}Click &quot;%2&quot; below to register your pledge. You will be able to modify or cancel future pledge payments at any time by logging in to your account or contacting us at %1.{/ts}</p>
          {else}
            <p>{ts 1=$receiptFromEmail 2=$button}Your initial pledge payment will be processed when you click &quot;%2&quot; below. You will be able to modify or cancel future pledge payments at any time by logging in to your account or contacting us at %1.{/ts}</p>
          {/if}
        {/if}
      </div>
    </div>
  {/if}

  {if $onbehalfProfile && $onbehalfProfile|@count}
    <div class="crm-group onBehalf_display-group label-left crm-profile-view">
      {include file="CRM/UF/Form/Block.tpl" fields=$onbehalfProfile prefix='onbehalf'}
    </div>
  {/if}

  {if $honoreeProfileFields && $honoreeProfileFields|@count}
    <div class="crm-group honor_block-group">
      <div class="header-dark">
        {$soft_credit_type}
      </div>
      <div class="display-block">
        <div class="label-left crm-section honoree_profile-section">
          <strong>{$honorName}</strong></br>
          {include file="CRM/UF/Form/Block.tpl" fields=$honoreeProfileFields mode=8 prefix='honor'}
        </div>
      </div>
    </div>
  {/if}

{* Move custom fields *}

  {if $pcpBlock}
    <div class="crm-group pcp_display-group">
      <div class="header-dark">
        {ts}Contribution Honor Roll{/ts}
      </div>
      <div class="display-block">
        {if $pcp_display_in_roll}
          {ts}List my contribution{/ts}
          {if $pcp_is_anonymous}
            <strong>{ts}anonymously{/ts}.</strong>
          {else}
            {ts}under the name{/ts}:
            <strong>{$pcp_roll_nickname}</strong>
            <br/>
            {if $pcp_personal_note}
              {ts}With the personal note{/ts}:
              <strong>{$pcp_personal_note}</strong>
            {else}
              <strong>{ts}With no personal note{/ts}</strong>
            {/if}
          {/if}
        {else}
          {ts}Don't list my contribution in the honor roll.{/ts}
        {/if}
        <br/>
      </div>
    </div>
  {/if}

  {if ( $contributeMode ne 'notify' and (!$is_pay_later or $isBillingAddressRequiredForPayLater) and $is_monetary and ( $amount GT 0 OR $minimum_fee GT 0 ) ) or $email }
    {if $contributeMode ne 'notify' and (!$is_pay_later or $isBillingAddressRequiredForPayLater) and $is_monetary and ( $amount GT 0 OR $minimum_fee GT 0 ) }
    {* Remove Billing Address *}


  {* Show credit or debit card section for 'direct' mode, except for PayPal Express (detected because credit card number is empty) *}
  {if $contributeMode eq 'direct' and ! $is_pay_later and $is_monetary and ( $amount GT 0 OR $minimum_fee GT 0 )}
    {crmRegion name="contribution-confirm-billing-block"}
    {if ($credit_card_number or $bank_account_number)}
      <div class="crm-group credit_card-group">
        {if $paymentFieldsetLabel}
          <div class="header-dark">
            {$paymentFieldsetLabel}
          </div>
        {/if}
        {if $paymentProcessor.payment_type == 2}
          <div class="display-block">
            {ts}Account Holder{/ts}: {$account_holder}<br/>
            {ts}Bank Account Number{/ts}: {$bank_account_number}<br/>
            {ts}Bank Identification Number{/ts}: {$bank_identification_number}<br/>
            {ts}Bank Name{/ts}: {$bank_name}<br/>
          </div>
          {if $contributeMode eq 'direct'}
            <div class="crm-group debit_agreement-group">
              <div class="header-dark">
                {ts}Agreement{/ts}
              </div>
              <div class="display-block">
                {ts}Your account data will be used to charge your bank account via direct debit. While submitting this form you agree to the charging of your bank account via direct debit.{/ts}
              </div>
            </div>
          {/if}
        {else}
          <div class="crm-section no-label credit_card_details-section">
            <div class="content">{$credit_card_type}</div>
            <div class="content">{$credit_card_number}</div>
            <div class="content">{if $credit_card_exp_date}{ts}Expires{/ts}: {$credit_card_exp_date|truncate:7:''|crmDate}{/if}</div>
            <div class="clear"></div>
          </div>
        {/if}
      </div>
    {/if}
    {/crmRegion}
  {/if}

  {include file="CRM/Contribute/Form/Contribution/PremiumBlock.tpl" context="confirmContribution"}

  {if $customPost}
    <fieldset class="label-left crm-profile-view">
      {include file="CRM/UF/Form/Block.tpl" fields=$customPost}
    </fieldset>
  {/if}

{* Move pay later instructions *}

        {if $is_pay_later OR $amount LE 0.0}
          <div class="bold pay_later_receipt-section">{$pay_later_receipt}</div>
        {/if}

{* Remove second set of instructions to click make payment *}

  <div id="crm-submit-buttons" class="crm-submit-buttons">
    {include file="CRM/common/formButtons.tpl" location="bottom"}
  </div>
</div>
