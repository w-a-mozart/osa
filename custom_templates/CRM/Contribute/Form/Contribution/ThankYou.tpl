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
{include file="CRM/common/TrackingFields.tpl"}

<div class="crm-block crm-contribution-thankyou-form-block">
{if $thankyou_text}
    <div id="thankyou_text" class="crm-section thankyou_text-section">
        {$thankyou_text}
    </div>
{/if}
    
{* Show link to Tell a Friend (CRM-2153) *}
{if $friendText}
    <div id="tell-a-friend" class="crm-section friend_link-section">
        <a href="{$friendURL}" title="{$friendText}" class="button"><span>&raquo; {$friendText}</span></a>
    </div>{if !$linkText}<br /><br />{/if}
{/if}

    <div class="action-link">
        <a title='Family Profile' class='button' href='{crmURL p='civicrm/user' q='reset=1'}'><span><div class='icon dashboard-icon'></div> Return to Family Profile </span></a>
        <a title='Check Out' class='button'  href='{$config->userFrameworkBaseURL}checkout/'><span><div class='icon check-icon'></div> Check Out </span></a>
    </div>

    <div class="spacer"></div>
        
    <div id="thankyou_footer" class="contribution_thankyou_footer-section">
        <p>
            {$thankyou_footer}
        </p>
    </div>
</div>
