{*
 +--------------------------------------------------------------------+
 | Custom OSA User Dashboard (Family Profile)                         |
 +--------------------------------------------------------------------+
 | Override Standard CiviCRM Template and add Teachers                |
 | -                                                                  |
 +--------------------------------------------------------------------+
 | Copyright Oakville Suzuki Association 2012                         |
 | Copyright CiviCRM LLC (c) 2004-2011                                |
 +--------------------------------------------------------------------+
*}
<div class="view-content">
  <div class="crm-block crm-content-block">
{if $teachers}
    <div id="teachers">
  {strip}
      <table id="teachers" class="display">
        <thead>
          <tr>
            <th>{ts}Student{/ts}</th>
            <th>{ts}Teacher{/ts}</th>
            <th>{ts}Start Date{/ts}</th>
            <th>{ts}End Date{/ts}</th>
            <th>{ts}Status{/ts}</th>
          </tr>
        </thead>
  {foreach from=$teachers item=teacher}
        <tr class="{cycle values="odd-row,even-row"}">
          <td class="bold">{$teacher.student.display_name}</td>
          <td>{$teacher.display_name}</td>
          <td>{$teacher.start_date|crmDate}</td>
          <td>{$teacher.end_date|crmDate}</td>
          <td>{$teacher.status}</td>
        </tr>
  {/foreach}
      </table>
  {/strip}
    </div>
{/if}

{if NOT ($teachers)}
    <div class="messages status">
      <div class="icon inform-icon"></div> {ts}There are no teacher registrations on record for this profile.{/ts}
    </div>
{/if}
  </div>
</div>