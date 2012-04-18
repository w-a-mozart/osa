{*
 +--------------------------------------------------------------------+
 | Custom OSA Membership Block                                        |
 +--------------------------------------------------------------------+
 | Override Standard CiviCRM Template MembershipBlock.tpl             |
 | - minor edits to not display table "membership-listings" when it   |
 |   is empty.                                                        |
 +--------------------------------------------------------------------+
 | Copyright Oakville Suzuki Association 2012                         |
 | Copyright CiviCRM LLC (c) 2004-2011                                |
 +--------------------------------------------------------------------+
*}
{if !empty($useForMember)}
<div id="membership" class="crm-group membership-group">
{if $context EQ "makeContribution"}
    <div id="priceset">
        <fieldset>
        {if $renewal_mode}
            {if $membershipBlock.renewal_title}
                <legend>{$membershipBlock.renewal_title}</legend>
            {/if}
            {if $membershipBlock.renewal_text}
                <div id="membership-intro" class="crm-section membership_renewal_intro-section">
                    {$membershipBlock.renewal_text}
                </div> 
            {/if}
        {else}  
          {if $membershipBlock.new_title}
              <legend>{$membershipBlock.new_title}</legend>
          {/if}
          {if $membershipBlock.new_text}
              <div id="membership-intro" class="crm-section membership_new_intro-section">
                 {$membershipBlock.new_text}
              </div> 
          {/if}
        {/if}
        {if !empty($membershipTypes)}
            {foreach from=$membershipTypes item=row}
                {if array_key_exists( 'current_membership', $row )}
                    <div id='help'>
                    {* Lifetime memberships have no end-date so current_membership array key exists but is NULL *}
                    {if $row.current_membership}
                        {if $row.current_membership|date_format:"%Y%m%d" LT $smarty.now|date_format:"%Y%m%d"}
                            {ts 1=$row.current_membership|crmDate 2=$row.name}Your <strong>%2</strong> membership expired on %1.{/ts}<br />
                        {else}
                            {ts 1=$row.current_membership|crmDate 2=$row.name}Your <strong>%2</strong> membership expires on %1.{/ts}<br />
                        {/if}
                    {else}
                        {ts 1=$row.name}Your <strong>%1</strong> membership does not expire (you do not need to renew that membership).{/ts}<br />
                    {/if}
                    </div>
                {/if}
            {/foreach}
        {/if}

        {include file="CRM/Price/Form/PriceSet.tpl" extends="Membership"}
      	<div id="allow_auto_renew">    
            <div class='crm-section auto-renew'>
                <div class='label'></div>
                <div class ='content'>
                    {if isset($form.auto_renew) }
                        {$form.auto_renew.html}&nbsp;{$form.auto_renew.label}
                        <span class="description crm-auto-renew-cancel-info">({ts}Your initial membership fee will be processed once you complete the confirmation step. You will be able to cancel automatic renewals at any time by logging in to your account or contacting us.{/ts})</span>
                    {/if}
                </div>
            </div>
         </div>
        </fieldset>
    </div>
{elseif $lineItem and $priceSetID}
  {assign var="totalAmount" value=$amount}
  <div class="header-dark">
  {ts}Membership Fee{/ts}
  </div>
  <div class="display-block">
    {include file="CRM/Price/Page/LineItem.tpl" context="Membership"}
  </div>
{/if}
</div>
{literal}
<script type="text/javascript">
cj(function(){
    //if price set is set we use below below code to show for showing auto renew
    var autoRenewOption =  {/literal}'{$autoRenewOption}'{literal};
    cj('#allow_auto_renew').hide();
    if ( autoRenewOption == 1 ) {
        cj('#allow_auto_renew').show();
    } else if ( autoRenewOption == 2 ) {
        var autoRenew = cj("#auto_renew");
        autoRenew.attr( 'checked',  true );
        autoRenew.attr( 'readonly', true );
        cj('#allow_auto_renew').show();
    }
});
</script>
{/literal}
{elseif $membershipBlock}
<div id="membership" class="crm-group membership-group">
  {if $context EQ "makeContribution"}
  <fieldset>    
      {if $renewal_mode }
        {if $membershipBlock.renewal_title}
            <legend>{$membershipBlock.renewal_title}</legend>
        {/if}
        {if $membershipBlock.renewal_text}
            <div id="membership-intro" class="crm-section membership_renewal_intro-section">
                <p>{$membershipBlock.renewal_text}</p>
            </div> 
        {/if}

      {else}        
        {if $membershipBlock.new_title}
            <legend>{$membershipBlock.new_title}</legend>
        {/if}
        {if $membershipBlock.new_text}
            <div id="membership-intro" class="crm-section membership_new_intro-section">
                <p>{$membershipBlock.new_text}</p>
            </div> 
        {/if}
      {/if}
  {/if}
  {if  $context neq "makeContribution" }
        <div class="header-dark">
            {if $renewal_mode }
                    {if $membershipBlock.renewal_title}
                        {$membershipBlock.renewal_title}
                    {else}
                        {ts}Select a Membership Renewal Level{/ts}
                    {/if}

            {else}
                    {if $membershipBlock.new_title}
                        {$membershipBlock.new_title}
                    {else}
                        {ts}Select a Membership Level{/ts}
                    {/if}
            {/if}
        </div>
    {/if}

    {* Only show this table if there is something to show *}
    {assign var="firstRow" value=$membershipTypes[0]}
    {if $showRadio OR array_key_exists('current_membership', $firstRow) OR isset($form.auto_renew) }
    {strip}
        <table id="membership-listings">
        {foreach from=$membershipTypes item=row }
        <tr {if $context EQ "makeContribution" OR $context EQ "thankContribution" }class="odd-row" {/if}valign="top">
            {if $showRadio }
                {assign var="pid" value=$row.id}
                <td style="width: 1em;">{$form.selectMembership.$pid.html}</td>
            {else}
                <td>&nbsp;</td>
            {/if}
           <td style="width: auto;">
                <span class="bold">{if $showRadio }{$row.name} {/if}&nbsp;
                {if ($membershipBlock.display_min_fee AND $context EQ "makeContribution") AND $row.minimum_fee GT 0 }
                    {if $is_separate_payment OR ! $form.amount.label}
                        - {$row.minimum_fee|crmMoney}
                    {else}
                        {ts 1=$row.minimum_fee|crmMoney}(contribute at least %1 to be eligible for this membership){/ts}
                    {/if}
                {/if}
                </span>
                {if $row.description }<br />{$row.description} &nbsp;{/if}
           </td>
           <td style="width: auto;">
              {* Check if there is an existing membership of this type (current_membership NOT empty) and if the end-date is prior to today. *}
              {if array_key_exists( 'current_membership', $row ) AND $context EQ "makeContribution" }
                  {if $row.current_membership}
                        {if $row.current_membership|date_format:"%Y%m%d" LT $smarty.now|date_format:"%Y%m%d"}
                            <em>{ts 1=$row.current_membership|crmDate 2=$row.name}Your <strong>%2</strong> membership expired on %1.{/ts}</em>
                        {else}
                            <em>{ts 1=$row.current_membership|crmDate 2=$row.name}Your <strong>%2</strong> membership expires on %1.{/ts}</em>
                        {/if}
                  {else}
                    {ts 1=$row.name}Your <strong>%1</strong> membership does not expire (you do not need to renew that membership).{/ts}<br />
                  {/if}
              {else}
                &nbsp;
              {/if}
           </td> 
        </tr>
        {/foreach}
	    {if isset($form.auto_renew) }
	        <tr id="allow_auto_renew">    
	        <td style="width: auto;">{$form.auto_renew.html}</td>
	        <td style="width: auto;">
	            {$form.auto_renew.label}
                <div class="description crm-auto-renew-cancel-info">({ts}Your initial membership fee will be processed once you complete the confirmation step. You will be able to cancel automatic renewals at any time by logging in to your account or contacting us.{/ts})</div>
	        </td>
    	    </tr>
        {/if}
        {if $showRadio}
            {if $showRadioNoThanks } {* Provide no-thanks option when Membership signup is not required - per membership block configuration. *}
            <tr class="odd-row">
              <td>{$form.selectMembership.no_thanks.html}</td>
              <td colspan="2"><strong>{ts}No thank you{/ts}</strong></td>      
            </tr> 
            {/if}
        {/if}          
        </table>
    {/strip}
    {/if}
    {if $context EQ "makeContribution"}
        </fieldset>
    {/if}
</div>

{literal}
<script type="text/javascript">
cj(function(){
    showHideAutoRenew( null );
});
function showHideAutoRenew( memTypeId ) 
{
  var considerUserInput = {/literal}'{$takeUserSubmittedAutoRenew}'{literal};	    
  if ( memTypeId ) considerUserInput = false;
  if ( !memTypeId ) memTypeId = cj('input:radio[name="selectMembership"]:checked').val();
  
  //does this page has only one membership type.
  var singleMembership = {/literal}'{$singleMembership}'{literal};
  if ( !memTypeId && singleMembership ) memTypeId = cj("#selectMembership").val( ); 
  
  var renewOptions  = {/literal}{$autoRenewMembershipTypeOptions}{literal};	 
  var currentOption = eval( "renewOptions." + 'autoRenewMembershipType_' + memTypeId );
  
  funName = 'hide();';
  var readOnly = false;
  var isChecked  = false; 
  if ( currentOption == 1 ) {
     funName = 'show();';
     
     //uncomment me, if we'd like 
     //to load auto_renew checked.
     //isChecked = true;
  
  } else if ( currentOption == 2 ) {
     funName = 'show();';
     isChecked = readOnly = true;
  }
  
  var autoRenew = cj("#auto_renew");	
  if ( considerUserInput ) isChecked = autoRenew.attr( 'checked' ); 

  //its a normal recur contribution.
  if ( cj( "is_recur" ) && 
      ( cj( 'input:radio[name="is_recur"]:checked').val() == 1 ) ) {
     isChecked = false;
     funName   = 'hide();';
  }
 
  //when we do show auto_renew read only 
  //which implies it should be checked.	 
  if ( readOnly && funName == 'show();' ) isChecked = true; 

  autoRenew.attr( 'readonly', readOnly );
  autoRenew.attr( 'checked',  isChecked );
  eval( "cj('#allow_auto_renew')." + funName );
}

{/literal}{if $allowAutoRenewMembership}{literal}
  cj( function( ) {
     //keep read only always checked.
     cj( "#auto_renew" ).click(function( ) {
        if ( cj(this).attr( 'readonly' ) ) { 
            cj(this).attr( 'checked', true );
        }
     });
  }); 
{/literal}{/if}{literal}
</script>
{/literal}

{/if}{* membership block end here *}
