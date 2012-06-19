{*
 +--------------------------------------------------------------------+
 | Custom OSA Thank You page                                          |
 +--------------------------------------------------------------------+
 | Override Standard CiviCRM Template ThankYou.tpl                    |
 | - using Drupal Commerce Cart so don't show everything just         |
 |   thanks and checkout button.                                      |
 +--------------------------------------------------------------------+
 | Copyright Oakville Suzuki Association 2012                         |
 | Copyright CiviCRM LLC (c) 2004-2011                                |
 +--------------------------------------------------------------------+
*}
{if $action & 1024}
    {include file="CRM/Event/Form/Registration/PreviewHeader.tpl"}
{/if}

{include file="CRM/common/TrackingFields.tpl"}

<div class="crm-block crm-event-thankyou-form-block">
    {* Don't use "normal" thank-you message for Waitlist and Approval Required registrations - since it will probably not make sense for those situations. dgg *}
    {if $event.thankyou_text AND (not $isOnWaitlist AND not $isRequireApproval)} 
        <div id="intro_text" class="crm-section event_thankyou_text-section">
            <p>
            {$event.thankyou_text}
            </p>
        </div>
    {/if}
    
    {* Show link to Tell a Friend (CRM-2153) *}
    {if $friendText}
        <div id="tell-a-friend" class="crm-section tell_friend_link-section">
            <a href="{$friendURL}" title="{$friendText}" class="button"><span>&raquo; {$friendText}</span></a>
       </div><br /><br />
    {/if}

    {* Add button for donor to create their own Personal Campaign page *}
    {if $pcpLink}
      <div class="crm-section create_pcp_link-section">
            <a href="{$pcpLink}" title="{$pcpLinkText}" class="button"><span>&raquo; {$pcpLinkText}</span></a>
        </div><br /><br />
    {/if}
    
        {if $isOnWaitlist}
          <div id="help">
            <p>
                <span class="bold">{ts}You have been added to the WAIT LIST for this event.{/ts}</span>
                {ts}If space becomes available you will receive an email with a link to a web page where you can complete your registration.{/ts}
             </p> 
          </div>
        {elseif $isRequireApproval}
          <div id="help">
            <p>
                <span class="bold">{ts}Your registration has been submitted.{/ts}
                {ts}Once your registration has been reviewed, you will receive an email with a link to a web page where you can complete the registration process.{/ts}</span>
            </p>
          </div>
        {elseif $is_pay_later and $paidEvent and !$isAmountzero}
          <div id="help">
            <div class="bold">{$pay_later_receipt}</div>
            {if $is_email_confirm}
                <p>{ts 1=$email}An email with event details has been sent to %1.{/ts}</p>
            {/if}
          </div>
        {* PayPal_Standard sets contribution_mode to 'notify'. We don't know if transaction is successful until we receive the IPN (payment notification) *}
        {elseif $contributeMode EQ 'notify' and $paidEvent}
            {* Remove message *}
            {if $is_email_confirm}
          <div id="help">
                <p>{ts 1=$email}A registration confirmation email will be sent to %1 once the transaction is processed successfully.{/ts}</p>
          </div>
            {/if}
        {else}
          <div id="help">
            <p>{ts}Your registration has been processed successfully. Please print this page for your records.{/ts}</p>
            {if $is_email_confirm}
                <p>{ts 1=$email}A registration confirmation email has also been sent to %1{/ts}</p>
            {/if}
          </div>
        {/if}
    <div class="spacer"></div>

    <div class="crm-group event_info-group">
        <div class="header-dark">
            {ts}Event Information{/ts}
        </div>
        <div class="display-block">
            {include file="CRM/Event/Form/Registration/EventInfoBlock.tpl" context="ThankYou"}
        </div>
    </div>
    
    {if $paidEvent}
        <div class="crm-group event_fees-group">
            <div class="header-dark">
                {$event.fee_label}
            </div>
            {if $lineItem}
                {include file="CRM/Price/Page/LineItem.tpl" context="Event"}
            {elseif $amount || $amount == 0}
	            <div class="crm-section no-label amount-item-section">
                    {foreach from= $finalAmount item=amount key=level}  
            			<div class="content">
            			    {$amount.amount|crmMoney}&nbsp;&nbsp;{$amount.label}
            			</div>
            			<div class="clear"></div>
                    {/foreach}
                </div>
                {if $totalAmount}
        			<div class="crm-section no-label total-amount-section">
                		<div class="content bold">{ts}Event Total{/ts}:&nbsp;&nbsp;{$totalAmount|crmMoney}</div>
                		<div class="clear"></div>
                	</div>
                    {if $hookDiscount.message}
                        <div class="crm-section hookDiscount-section">
                            <em>({$hookDiscount.message})</em>
                        </div>
                    {/if}
                {/if}	
            {/if}
            {if $receive_date}
                <div class="crm-section no-label receive_date-section">
                    <div class="content bold">{ts}Transaction Date{/ts}: {$receive_date|crmDate}</div>
                	<div class="clear"></div>
                </div>
            {/if}
            {if $contributeMode ne 'notify' AND $trxn_id}
                <div class="crm-section no-label trxn_id-section">
                    <div class="content bold">{ts}Transaction #{/ts}: {$trxn_id}</div>
            		<div class="clear"></div>
            	</div>
            {/if}
        </div>
    
    {elseif $participantInfo}
        <div class="crm-group participantInfo-group">
            <div class="header-dark">
                {ts}Additional Participant Email(s){/ts}
            </div>
            <div class="crm-section no-label participant_info-section">
                <div class="content">
                    {foreach from=$participantInfo  item=mail key=no}  
                        <strong>{$mail}</strong><br />	
                    {/foreach}
                </div>
        		<div class="clear"></div>
        	</div>
        </div>
    {/if}

    {* Remove Registered Email *}

    {if $event.participant_role neq 'Attendee' and $defaultRole}
        <div class="crm-group participant_role-group">
            <div class="header-dark">
                {ts}Participant Role{/ts}
            </div>
            <div class="crm-section no-label participant_role-section">
                <div class="content">
                    {$event.participant_role}
                </div>
        		<div class="clear"></div>
        	</div>
        </div>
    {/if}

    {if $customPre}
            <fieldset class="label-left">
                {include file="CRM/UF/Form/Block.tpl" fields=$customPre}
            </fieldset>
    {/if}

    {if $customPost}
            <fieldset class="label-left">  
                {include file="CRM/UF/Form/Block.tpl" fields=$customPost}
            </fieldset>
    {/if}

    {*display Additional Participant Profile Information*}
    {if $addParticipantProfile}
        {foreach from=$addParticipantProfile item=participant key=participantNo}
            <div class="crm-group participant_info-group">
                <div class="header-dark">
                    {ts 1=$participantNo+1}Participant Information - Participant %1{/ts}	
                </div>
                {if $participant.additionalCustomPre}
		    <fieldset class="label-left"><div class="header-dark">{$participant.additionalCustomPreGroupTitle}</div>	
                        {foreach from=$participant.additionalCustomPre item=value key=field}
                            <div class="crm-section {$field}-section">
                                <div class="label">{$field}</div>
                                <div class="content">{$value}</div>
                                <div class="clear"></div>
                            </div>
                        {/foreach}
                    </fieldset>
                {/if}

                {if $participant.additionalCustomPost}
		{foreach from=$participant.additionalCustomPost item=value key=field}
		<fieldset class="label-left"><div class="header-dark">{$participant.additionalCustomPostGroupTitle.$field.groupTitle}</div>
                        {foreach from=$participant.additionalCustomPost.$field item=value key=field}
                            <div class="crm-section {$field}-section">
                                <div class="label">{$field}</div>
                                <div class="content">{$value}</div>
                                <div class="clear"></div>
                            </div>
                        {/foreach}		 
		{/foreach}		

                    </fieldset>
                {/if}
            </div>
        <div class="spacer"></div>
        {/foreach}
    {/if}

    {if $contributeMode ne 'notify' and $paidEvent and ! $is_pay_later and ! $isAmountzero and !$isOnWaitlist and !$isRequireApproval}   
        <div class="crm-group billing_name_address-group">
            <div class="header-dark">
                {ts}Billing Name and Address{/ts}
            </div>
        	<div class="crm-section no-label billing_name-section">
        		<div class="content">{$billingName}</div>
        		<div class="clear"></div>
        	</div>
        	<div class="crm-section no-label billing_address-section">
        		<div class="content">{$address|nl2br}</div>
        		<div class="clear"></div>
        	</div>
        </div>
    {/if}

    {if $contributeMode eq 'direct' and $paidEvent and ! $is_pay_later and !$isAmountzero and !$isOnWaitlist and !$isRequireApproval}
        <div class="crm-group credit_card-group">
            <div class="header-dark">
                {ts}Credit Card Information{/ts}
            </div>
            <div class="crm-section no-label credit_card_details-section">
                <div class="content">{$credit_card_type}</div>
        		<div class="content">{$credit_card_number}</div>
        		<div class="content">{ts}Expires{/ts}: {$credit_card_exp_date|truncate:7:''|crmDate}</div>
        		<div class="clear"></div>
        	</div>
        </div>
    {/if}

    {if $event.thankyou_footer_text}
        <div id="footer_text" class="crm-section event_thankyou_footer-section">
            <p>{$event.thankyou_footer_text}</p>
        </div>
    {/if}
    
    <div class="action-link section event_info_link-section">
        {* Change links to Registration Page *}
        <a title='Family Profile' class='button' href='{crmURL p='civicrm/user' q='reset=1'}'><span><div class='icon dashboard-icon'></div> Return to Family Profile </span></a>
        <a title='Register' class='button' href="{crmURL p='civicrm/event/register' q="reset=1&id=`$event.id`"}"><span><div class='icon back-icon'></div> Register Another Participant </span></a>
        <a title='Check Out' class='button'  href='{$config->userFrameworkBaseURL}checkout/'><span><div class='icon check-icon'></div> Check Out </span></a>
    </div>

    {if $event.is_public }
        {include file="CRM/Event/Page/iCalLinks.tpl"}
    {/if}
    {if $event.is_share}
    {capture assign=eventUrl}{crmURL p='civicrm/event/info' q="id=`$event.id`&amp;reset=1" a=true fe=1 h=1}{/capture}
    {include file="CRM/common/SocialNetwork.tpl" url=$eventUrl title=$event.title pageURL=$eventUrl}
    {/if}
</div>