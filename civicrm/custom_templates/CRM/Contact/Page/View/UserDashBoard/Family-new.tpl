{*
 +--------------------------------------------------------------------+
 | Custom OSA User Dashboard (Family Profile)                         |
 +--------------------------------------------------------------------+
 | Override Standard CiviCRM Template for Relationships               |
 | - use profiles to display and edit contact info                    |
 | - use colorbox library to display dialogues                        |
 | - split parents and students                                       |
 +--------------------------------------------------------------------+
 | Copyright Oakville Suzuki Association 2012                         |
 | Copyright CiviCRM LLC (c) 2004-2011                                |
 +--------------------------------------------------------------------+
*}
<div class="view-content">
  <div class="crm-block crm-content-block">
{if $parents}
    <div id="parent-relationships">
  {strip}
      <table id="parent_relationship" class="display">
        <thead>
          <tr>
            <th>{ts}Parents/Guardians{/ts}</th>
            <th>{ts}Email{/ts}</th>
            <th>{ts}Phone{/ts}</th>
            <th></th>
            <th></th>
          </tr>
        </thead>
    {foreach from=$parents item=parent}
        <tr id="parent_{$parent.contact.id}" class="{cycle values="odd-row,even-row"} row-relationship">
          <td class="bold">{$parent.contact.display_name}</td>
          <td>{$parent.contact.email}</td>
          <td>{$parent.contact.phone}</td>
      {if $parent.relationship.relation eq 'Head of Household is'}
          <td><img src="/images/contact_house.png"> Primary Contact</td>
      {else}
          <td>&nbsp;</td>
      {/if}
          <td class="nowrap">
            {$parent.links}
          </td>
        </tr>
    {/foreach}
      </table>
  {/strip}
    </div><br/>
{else}
    <div class="status">
      <div class="icon inform-icon"></div> {ts}There are no Parents/Guardians related to this profile.{/ts}
    </div>
{/if}
{if $students}
    <div id="student-relationships">
  {strip}
    {foreach from=$students item=student}
      <div id="student_{$student.contact.id}" style="display: inline-block; margin: 10px;">
        <table border="10px" cellspacing="0" cellpadding="5px">
          <tbody>
            <tr><td style="text-align: center;" colspan="2"><img src="/images/junior_grad.png" /></td></tr>
            <tr><td style="text-align: center; background-color: blue;" colspan="2"><span style="color: #ffffff; ;font-size: 80%;"><strong>STUDENT</strong></span></td></tr>
            <tr><td style="text-align: center;" colspan="2"><span style="font-size: 150%;">{$student.contact.display_name}</span></td></tr>
            <tr><td style="text-align: center; background-color: blue;" colspan="2"><span style="color: #ffffff; ;font-size: 80%;"><strong>INSTRUMENTS</strong></span></td></tr>
            <tr><td style="text-align: center;" colspan="2">{$student.contact.instrument_1}{if $student.contact.instrument_2}, {$student.contact.instrument_2}{/if}{if $student.contact.instrument_3}, {$student.contact.instrument_3}{/if}</td></tr>
            <tr><td style="text-align: center; background-color: blue;" colspan="2"><span style="color: #ffffff; ;font-size: 80%;"><strong>TEACHERS</strong></span></td></tr>
            <tr><td style="text-align: center;" colspan="2">Hana Gertalova, Jesse Dietrick</td></tr>
            <tr><td style="text-align: center; background-color: blue;" colspan="2"><span style="color: #ffffff; ;font-size: 80%;"><strong>REGISTRATIONS</strong></span></td></tr>
            <tr>
              <td style="text-align: center;">2016 / 2017 Membership</td>
      {if $student.contact.is_current_member}
              <td style="text-align: center;"><img src="/images/Membership.png" /> {$student.membership.status}</td>
      {else}
              <td style="text-align: center;"><a title="Renew Membership" href="/osa/civicrm/contribute/transact?reset=1&id=1&cid={$student.contact.id}">{$student.membership.status}</a></td>
      {/if}
            </tr>
            <tr>
              <td class="nowrap" colspan="2"><strong><span class="btn-slide crm-hover-button">Click to Register for:</span></strong>
                <ul class="panel">
                  <li><a class="action-item crm-hover-button" title="Register/Renew Membership" href="/osa/civicrm/contribute/transact?reset=1&id=1&cid={$student.contact.id}">Membership</a></li>
                  <li><a class="action-item crm-hover-button" title="Register for Group Class" href="https://oakvillesuzuki.org/osa/group_class_selection/{$student.contact.id}">Group Class</a></li>
                  <li><a class="action-item crm-hover-button" title="Register for an Event" href="https://oakvillesuzuki.org/osa/events/{$student.contact.id}">Events</a></li>
                </ul>
                <strong><span class="btn-slide crm-hover-button">More</span></strong>
                <ul class="panel">
                  <li><a class="action-item crm-hover-button box-load cboxElement" title="Edit Student" href="/osa/civicrm/profile/edit?reset=1&snippet=1&context=boxload&gid=14&id={$student.contact.id}">Edit</a></li>
                  <li><a class="action-item crm-hover-button" title="Delete Student" href="#">Delete</a></li>
                </ul>
              </td>
            </tr>
          </tbody>
        </table>
      </div>
    {/foreach}
  {/strip}
    </div>
{else}
    <div class="status">
      <div class="icon inform-icon"></div> {ts}There are no Students related to this profile.{/ts}
    </div>
{/if}
{if $contact.contact_type eq 'Household'}
    <div class="action-link">
      <a title='New Parent' class='add button box-load'  href='{crmURL p='civicrm/profile/create' q="reset=1&snippet=1&context=boxload&gid=13&hid=`$contact.contact_id`"}&width=50%'><span><div class='icon add-icon'></div> Add a Parent/Guardian </span></a>
      <a title='New Student' class='add button box-load'  href='{crmURL p='civicrm/profile/create' q="reset=1&snippet=1&context=boxload&gid=14&hid=`$contact.contact_id`"}&width=50%'><span><div class='icon add-icon'></div> Add a Student </span></a>
    </div>
{/if}
  </div>
</div>
