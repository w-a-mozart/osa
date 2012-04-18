{*
 +--------------------------------------------------------------------+
 | Custom OSA User Dashboard (Family Profile)                         |
 +--------------------------------------------------------------------+
 | Override Standard CiviCRM Template for Events                      |
 | - changes to text                                                  |
 +--------------------------------------------------------------------+
 | Copyright Oakville Suzuki Association 2012                         |
 | Copyright CiviCRM LLC (c) 2004-2011                                |
 +--------------------------------------------------------------------+
*}
<div class="view-content">
{if $event_rows}
  {strip}
  <div class="description">
    {ts}Click on the event name for more information.{/ts}
  </div>
  <table class="selector">
    <tr class="columnheader">
      <th>{ts}Event{/ts}</th>
      <th>{ts}Event Date(s){/ts}</th>
      <th>{ts}Status{/ts}</th>
      <th></th>
    </tr>
  {counter start=0 skip=1 print=false}
  {foreach from=$event_rows item=row}
    <tr id='rowid{$row.participant_id}' class=" crm-event-participant-id_{$row.participant_id} {cycle values="odd-row,even-row"}{if $row.status eq Cancelled} disabled{/if}">
      <td class="crm-participant-event-id_{$row.event_id}"><a href="{crmURL p='civicrm/event/info' q="reset=1&id=`$row.event_id`&context=dashboard"}">{$row.event_title}</a></td>
      <td class="crm-participant-event_start_date">
    {$row.event_start_date|crmDate}
    {if $row.event_end_date}
        &nbsp; - &nbsp;
      {* Only show end time if end date = start date *}
      {if $row.event_end_date|date_format:"%Y%m%d" == $row.event_start_date|date_format:"%Y%m%d"}
        {$row.event_end_date|crmDate:0:1}
      {else}
        {$row.event_end_date|crmDate}
      {/if}
    {/if}
      </td>
      <td class="crm-participant-participant_status">{$row.participant_status}</td>
      <td class="crm-participant-showConfirmUrl">
    {if $row.showConfirmUrl}
        <a href="{crmURL p='civicrm/event/confirm' q="reset=1&participantId=`$row.participant_id`"}">{ts}Confirm Registration{/ts}</a>                            
    {/if}
      </td>
    </tr>
  {/foreach}
  </table>
  {/strip}
{else}
  <div class="messages status">
    <div class="icon inform-icon"></div>&nbsp;
    {ts}There are no current or upcoming events on record for you.{/ts}            
  </div>
{/if}
</div>
