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

<div class="crm-contribution-page-id-{$contributionPageID} crm-block crm-contribution-thankyou-form-block">
  {if $thankyou_text}
    <div id="thankyou_text" class="crm-section thankyou_text-section">
      {$thankyou_text}
    </div>
  {/if}

  {* Show link to Tell a Friend (CRM-2153) *}
  {if $friendText}
    <div id="tell-a-friend" class="crm-section friend_link-section">
      <a href="{$friendURL}" title="{$friendText|escape:'html'}" class="button"><span>&raquo; {$friendText}</span></a>
    </div>{if !$linkText}<br /><br />{/if}
  {/if}
  {* Add button for donor to create their own Personal Campaign page *}
  {if $linkText}
    <div class="crm-section create_pcp_link-section">
      <a href="{$linkTextUrl}" title="{$linkText|escape:'html'}" class="button"><span>&raquo; {$linkText}</span></a>
    </div><br /><br />
  {/if}

  {* move help div's *}
    {* PayPal_Standard sets contribution_mode to 'notify'. We don't know if transaction is successful until we receive the IPN (payment notification) *}
    {if $is_pay_later}
    <div class="help">
      <div class="bold">{$pay_later_receipt}</div>
      {if $is_email_receipt}
        <div>
          {if $onBehalfEmail AND ($onBehalfEmail neq $email)}
            {ts 1=$email 2=$onBehalfEmail}An email confirmation with these payment instructions has been sent to %1 and to %2.{/ts}
          {else}
            {ts 1=$email}An email confirmation with these payment instructions has been sent to %1.{/ts}
          {/if}
        </div>
      {/if}
    </div>
    {elseif $isPendingOutcome}
      {* Remove message *}
      {if $is_email_receipt}
    <div class="help">
      <div>
        {if $onBehalfEmail AND ($onBehalfEmail neq $email)}
          {ts 1=$email 2=$onBehalfEmail}An email receipt will be sent to %1 and to %2 once the transaction is processed successfully.{/ts}
        {else}
          {ts 1=$email}An email receipt will be sent to %1 once the transaction is processed successfully.{/ts}
        {/if}
      </div>
    </div>
    {/if}
  {else}
    <div class="help">
    <div>{ts}Your transaction has been processed successfully.{/ts}</div>
      {if $is_email_receipt}
        <div>
          {if $onBehalfEmail AND ($onBehalfEmail neq $email)}
            {ts 1=$email 2=$onBehalfEmail}An email receipt has also been sent to %1 and to %2{/ts}
          {elseif $email}
            {ts 1=$email}An email receipt has also been sent to %1{/ts}
          {/if}
        </div>
      {/if}
  </div>
    {/if}
  <div class="spacer"></div>

  {include file="CRM/Contribute/Form/Contribution/MembershipBlock.tpl" context="thankContribution"}

  {if $amount GTE 0 OR $minimum_fee GTE 0 OR ( $priceSetID and $lineItem ) }
    <div class="crm-group amount_display-group">
      {if !$useForMember}
        {if $amount > 0}
        <div class="header-dark">
          {if !$membershipBlock AND $amount OR ( $priceSetID and $lineItem )}{ts}Contribution Information{/ts}{else}{ts}Membership Fee{/ts}{/if}
        </div>
      {/if}
      {/if}
      <div class="display-block">
        {if !$useForMember}
          {if $lineItem and $priceSetID}
            {if !$amount}{assign var="amount" value=0}{/if}
            {assign var="totalAmount" value=$amount}
            {if $amount > 0}
            {include file="CRM/Price/Page/LineItem.tpl" context="Contribution"}
            {/if}
          {elseif $membership_amount}
            {$membership_name} {ts}Membership{/ts}: <strong>{$membership_amount|crmMoney}</strong><br />
            {if $amount}
              {if !$is_separate_payment}
                {ts}Contribution Amount{/ts}: <strong>{$amount|crmMoney}</strong><br />
              {else}
                {ts}Additional Contribution{/ts}: <strong>{$amount|crmMoney}</strong><br />
              {/if}
            {/if}
            <strong> -------------------------------------------</strong><br />
            {ts}Total{/ts}: <strong>{$amount+$membership_amount|crmMoney}</strong><br />
          {else}
            {if $totalTaxAmount}
              {ts}Tax Amount{/ts}: <strong>{$totalTaxAmount|crmMoney}</strong><br />
            {/if}
            {if $installments}{ts}Installment Amount{/ts}{else}{ts}Amount{/ts}{/if}: <strong>{$amount|crmMoney}{if $amount_level } &ndash; {$amount_level}{/if}</strong>
          {/if}
        {/if}
        {if $receive_date}
          {if $amount > 0}
          {ts}Date{/ts}: <strong>{$receive_date|crmDate}</strong><br />
        {/if}
        {/if}
        {if $contributeMode ne 'notify' and $is_monetary and ! $is_pay_later and $trxn_id}
          {ts}Transaction #{/ts}: {$trxn_id}<br />
        {/if}
        {if $membership_trx_id}
          {ts}Membership Transaction #{/ts}: {$membership_trx_id}
        {/if}

        {* Recurring contribution / pledge information *}
        {if $is_recur}
          {if !empty($auto_renew)} {* Auto-renew membership confirmation *}
            {crmRegion name="contribution-thankyou-recur-membership"}
              <br />
              {if $frequency_interval > 1}
                <strong>{ts 1=$frequency_interval 2=$frequency_unit}This membership will be renewed automatically every %1 %2(s).{/ts}</strong>
              {else}
                <strong>{ts 1=$frequency_unit}This membership will be renewed automatically every %1.{/ts}</strong>
              {/if}
              <div class="description crm-auto-renew-cancel-info">({ts}You will receive an email receipt which includes information about how to cancel the auto-renewal option.{/ts})</div>
            {/crmRegion}
          {else}
            {crmRegion name="contribution-thankyou-recur"}
              {if $installments > 1}
                {if $frequency_interval > 1}
                  <p><strong>{ts 1=$frequency_interval 2=$frequency_unit 3=$installments}This recurring contribution will be automatically processed every %1 %2s for a total %3 installments (including this initial contribution).{/ts}</strong></p>
                {else}
                    <p><strong>{ts 1=$frequency_unit 2=$installments}This recurring contribution will be automatically processed every %1 for a total %2 installments (including this initial contribution).{/ts}</strong></p>
                  {/if}
                {else}
                  {if $frequency_interval > 1}
                    <p><strong>{ts 1=$frequency_interval 2=$frequency_unit}This recurring contribution will be automatically processed every %1 %2s.{/ts}</strong></p>
                  {else}
                    <p><strong>{ts 1=$frequency_unit}This recurring contribution will be automatically processed every %1.{/ts}</strong></p>
                  {/if}
                {/if}
                  <p>
                  {if $is_email_receipt}
                    {ts}You will receive an email receipt which includes information about how to update or cancel this recurring contribution.{/ts}
                  {/if}
                </p>
            {/crmRegion}
          {/if}
        {/if}

        {if $is_pledge}
          {if $pledge_frequency_interval GT 1}
            <p><strong>{ts 1=$pledge_frequency_interval 2=$pledge_frequency_unit 3=$pledge_installments}I pledge to contribute this amount every %1 %2s for %3 installments.{/ts}</strong></p>
          {else}
            <p><strong>{ts 1=$pledge_frequency_interval 2=$pledge_frequency_unit 3=$pledge_installments}I pledge to contribute this amount every %2 for %3 installments.{/ts}</strong></p>
          {/if}
          <p>
            {if $is_pay_later}
              {ts 1=$receiptFromEmail}We will record your initial pledge payment when we receive it from you. You will be able to modify or cancel future pledge payments at any time by logging in to your account or contacting us at %1.{/ts}
            {else}
              {ts 1=$receiptFromEmail}Your initial pledge payment has been processed. You will be able to modify or cancel future pledge payments at any time by contacting us at %1.{/ts}
            {/if}
            {if $max_reminders}
              {ts 1=$initial_reminder_day}We will send you a payment reminder %1 days prior to each scheduled payment date. The reminder will include a link to a page where you can make your payment online.{/ts}
            {/if}
          </p>
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
          {include file="CRM/UF/Form/Block.tpl" fields=$honoreeProfileFields prefix='honor'}
        </div>
      </div>
   </div>
  {/if}

    {* Remove Profile block *}

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
            {ts}under the name{/ts}: <strong>{$pcp_roll_nickname}</strong><br/>
            {if $pcp_personal_note}
              {ts}With the personal note{/ts}: <strong>{$pcp_personal_note}</strong>
            {else}
              <strong>{ts}With no personal note{/ts}</strong>
            {/if}
          {/if}
        {else}
          {ts}Don't list my contribution in the honor roll.{/ts}
        {/if}
        <br />
      </div>
    </div>
  {/if}

  {* Remove Billing Address *}

  {* Remove Card Details *}
    
  {* Remove Premium Block *}

  {* Remove Custom Fields *}

  {* Add custom buttons *}
  <div class="action-link">
    {if $membershipBlock}
      <a title='You can now Register for Group Class' class='button'  href='{$config->userFrameworkBaseURL}group_class_selection/{$cid}'><span><div class='icon next-icon'></div> Register for Group Class </span></a>
    {/if}
    {if $session->get('ufID') > 0}
      <a title='Family Profile' class='button' href='{crmURL p='civicrm/user' q='reset=1'}'><span><div class='icon dashboard-icon'></div> Return to Family Profile </span></a>
    {/if}
    {if $paymentProcessor.payment_processor_type eq 'drupalcommerce'} 
      <a title='Check Out' class='button'  href='{$config->userFrameworkBaseURL}checkout/'><span><div class='icon cart-icon'></div> Check Out </span></a>
    {/if}
  </div>

  <div class="spacer"></div>

  <div id="thankyou_footer" class="contribution_thankyou_footer-section">
    <p>
      {$thankyou_footer}
    </p>
  </div>
  {if $isShare}
    {capture assign=contributionUrl}{crmURL p='civicrm/contribute/transact' q="$qParams" a=1 fe=1 h=1}{/capture}
    {include file="CRM/common/SocialNetwork.tpl" url=$contributionUrl title=$title pageURL=$contributionUrl}
  {/if}
</div>
