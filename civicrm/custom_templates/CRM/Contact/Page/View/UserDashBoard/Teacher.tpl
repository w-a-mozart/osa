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
            <th>{ts}School Year{/ts}</th>
            <th>{ts}Instrument{/ts}</th>
            <th>{ts}Location{/ts}</th>
            <th>{ts}Lesson Length{/ts}</th>
            <th>{ts}Status{/ts}</th>
          </tr>
        </thead>
  {foreach from=$teachers item=teacher}
        <tr class="{cycle values="odd-row,even-row"}">
          <td class="bold">{$teacher.student.display_name}</td>
          <td>{$teacher.display_name}</td>
          <td>{$teacher.school_year}</td>
          <td>{$teacher.lesson_instrument}</td>
          <td>{$teacher.lesson_location}</td>
          <td>{$teacher.lesson_length}</td>
          <td>
            {if $teacher.status neq 'Expired'}
              {$teacher.status}
            {else}
              <a href="{crmURL p='civicrm/contribute/transact' q='reset=1&id=2'}&cid={$teacher.student.contact_id}">{$teacher.status}</a>
            {/if}
          </td>
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
{if $contact.contact_type eq 'Household'}
    <div class="action-link">
      <a title='New Teacher Registration' class='add button'  href="{crmURL p='civicrm/contribute/transact' q='reset=1&id=2'}"><span><div class='icon add-icon'></div> Register for Private Lessons </span></a>
    </div>
{/if}
  </div>
</div>