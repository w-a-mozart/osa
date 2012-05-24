{*
 +--------------------------------------------------------------------+
 | Custom OSA User Dashboard (Family Profile)                         |
 +--------------------------------------------------------------------+
 | Override Standard CiviCRM Template for Memberships                 |
 | -                                                                  |
 +--------------------------------------------------------------------+
 | Copyright Oakville Suzuki Association 2012                         |
 | Copyright CiviCRM LLC (c) 2004-2011                                |
 +--------------------------------------------------------------------+
*}
<div class="view-content">
  <div class="crm-block crm-content-block">
{if $members}
    <div id="memberships">
  {strip}
      <table id="membership" class="display">
        <thead>
          <tr>
            <th>{ts}Membership{/ts}</th>
            <th></th>
            <th>{ts}Start Date{/ts}</th>
            <th>{ts}End Date{/ts}</th>
            <th>{ts}Status{/ts}</th>
            <th></th>
          </tr>
        </thead>
  {foreach from=$members item=member}
        <tr class="{cycle values="odd-row,even-row"}">
          <td class="bold">{$member.display_name}</td>
          <td>{$member.membership_name}</td>
          <td>{$member.start_date|crmDate}</td>
          <td>{$member.end_date|crmDate}</td>
          <td>{$member.status}</td>
          <td>{if $member.renewPageId}<a href="{crmURL p='civicrm/contribute/transact' q="id=`$member.renewPageId`&cid=`$member.contact_id`&reset=1"}">[ {ts}Renew Now{/ts} ]</a>{/if}</td>
        </tr>
  {/foreach}
      </table>
  {/strip}
    </div>
{/if}

{if NOT ($members)}
    <div class="messages status">
      <div class="icon inform-icon"></div> {ts}There are no memberships on record for this profile.{/ts}
    </div>
{/if}
  </div>
</div>