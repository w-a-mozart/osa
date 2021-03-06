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
  <div class="crm-block crm-content-block">
{if $event_rows}
    <div id="events">
  {strip}
      <table id="event" class="display">
        <thead>
          <tr>
            <th>{ts}Event{/ts}</th>
            <th>{ts}Name{/ts}</th>
            <th>{ts}Event Date(s){/ts}</th>
            <th>{ts}Status{/ts}</th>
            <th></th>
          </tr>
        </thead>
  {foreach from=$event_rows item=event}
        <tr class="{cycle values="odd-row,even-row"} crm-event-participant-id_{$event.participant_id}{if $event.status eq Cancelled} disabled{/if}">
          <td class="bold"><a href="{crmURL p='civicrm/event/info' q="reset=1&id=`$event.event_id`&context=dashboard"}">{$event.event_title}</a></td>
          <td>{$event.sort_name}</td>
          <td class="crm-participant-event_start_date">{$event.event_start_date|crmDate}
    {if $event.event_end_date}
            &nbsp;-
      {if $event.event_end_date|date_format:"%Y%m%d" == $event.event_start_date|date_format:"%Y%m%d"}
            {$event.event_end_date|crmDate:0:1}
      {else}
            {$event.event_end_date|crmDate}
      {/if}
    {/if}
          </td>
          <td class="crm-participant-participant_status"><a class='box-load' href="/osa/osa/participant/view?id={$event.id}&cid={$event.contact_id}&snippet=1&context=boxload&width=50%">{$event.participant_status} </a></td>
          <td class="crm-participant-showConfirmUrl">
    {if $event.showConfirmUrl}
            <a href="{crmURL p='civicrm/event/confirm' q="reset=1&participantId=`$event.participant_id`"}">{ts}Confirm{/ts}</a>
    {/if}
          </td>
        </tr>
  {/foreach}
      </table>
  {/strip}
    </div>
{/if}

{if NOT ($event_rows)}
    <div class="status">
      <div class="icon inform-icon"></div>&nbsp;{ts}There are no current or upcoming events on record.{/ts}            
    </div>
{/if}
{if $contact.contact_type eq 'Household'}
    <div class="action-link">
      <a title='Register for Special Events' class='add button' href="{$config->userFrameworkBaseURL}events"><span><div class='icon add-icon'></div> Register for an Event </span></a>
    </div>
{/if}
  </div>
</div>