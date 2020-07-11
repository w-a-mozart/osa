{foreach from=$sortedResults key=display_date item=assignments}
  <h3>{$display_date}</h3>
  <table>
    <tr>
      <th>{ts domain='org.civicrm.volunteer'}Volunteer Name{/ts}</th>
      <th>{ts domain='org.civicrm.volunteer'}Role{/ts}</th>
      <th>Need</th>
      <th>Phone</th>
      <th>Email</th>
    </tr>
    {foreach from=$assignments.values item=assignment}
      <tr>
        <td class="roster-name">
          <a href="/osa/civicrm/contact/view?reset=1&cid={$assignment.contact_id}">{$assignment.name}</a>
        </td>
        <td class="roster-role">{$assignment.role_label}</td>
        <td class="roster-description">{$assignment.need_description}</td>
        <td class="roster-phone">{$assignment.assignee_phone}</td>
        <td class="roster-email"><a href="mailto:{$assignment.email}">{$assignment.assignee_email}</a></td>
      </tr>
    {/foreach}
  </table>
{/foreach}
<br/>
<div class='dateBlock'>
  <p>{ts 1=$endDate|crmDate domain='org.civicrm.volunteer'}Assignments that end before %1 are not shown.{/ts}</p>
</div>

<a href='{crmURL p='civicrm/vol/#/volunteer/manage'}'>
  <input type='button' class="crm-vol-modal-closer" value='{ts domain='org.civicrm.volunteer'}Back to Manage Volunteer Projects.{/ts}' onclick="CRM.$('[title=\'Close\']').click();"/>
</a>
