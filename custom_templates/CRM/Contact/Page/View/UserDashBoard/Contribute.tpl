{*
 +--------------------------------------------------------------------+
 | Custom OSA User Dashboard (Family Profile)                         |
 +--------------------------------------------------------------------+
 | Override Standard CiviCRM Template for Contributions               |
 | - changes to text                                                  |
 +--------------------------------------------------------------------+
 | Copyright Oakville Suzuki Association 2012                         |
 | Copyright CiviCRM LLC (c) 2004-2011                                |
 +--------------------------------------------------------------------+
*}
<div class="view-content">
  <div class="crm-block crm-content-block">
{if $contribute_rows}
    <div id="contributions">
  {strip}
      <table id="contributions" class="display">
        <thead>
          <tr>
            <th>{ts}Contribution{/ts}</th>
            <th>{ts}Name{/ts}</th>
            <th>{ts}Total Amount{/ts}</th>
            <th>{ts}Received date{/ts}</th>
            <th>{ts}Receipt Sent{/ts}</th>
            <th>{ts}Status{/ts}</th>
          </tr>
        </thead>
  {foreach from=$contribute_rows item=row}
        <tr id='rowid{$row.contribution_id}' class="{cycle values="odd-row,even-row"}{if $row.cancel_date} disabled{/if}">
          <td class="bold">{$row.contribution_type}</td>
          <td>{$row.sort_name}</td>
          <td>{$row.total_amount|crmMoney:$row.currency} {if $row.amount_level } - {$row.amount_level} {/if}
    {if $row.contribution_recur_id}
            <br /> {ts}(Recurring Contribution){/ts}
    {/if}
          </td>
          <td>{$row.receive_date|truncate:10:''|crmDate}</td>
          <td>{$row.receipt_date|truncate:10:''|crmDate}</td>
          <td>{$row.contribution_status}</td>
        </tr>
  {/foreach}
      </table>
  {/strip}
    </div>
{else}
    <div class="messages status">
      <div class="icon inform-icon"></div>{ts}There are no recent payments on record for you.{/ts}
    </div>
{/if}
  </div>
</div>
