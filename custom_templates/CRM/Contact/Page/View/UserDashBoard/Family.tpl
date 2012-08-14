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
                    <td><img src="/images/contact_house.png"> Primary Contact</span></td>
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
        </div>
{else}
        <div class="messages status">
            <div class="icon inform-icon"></div> {ts}There are no Parents/Guardians related to this profile.{/ts}
        </div>
{/if}
{if $students}
        <div id="student-relationships">
    {strip}
            <table id="student_relationship" class="display">
                <thead>
                    <tr>
                        <th>{ts}Students{/ts}</th>
                        <th>{ts}Gender{/ts}</th>
                        <th>{ts}Birth Day{/ts}</th>
                        <th>{ts}Member{/ts}</th>
                        <th>{ts}Instrument{/ts}</th>
                        <th>{ts}Click to Register for...{/ts}</th>
                    </tr>
                </thead>
    {foreach from=$students item=student}
                <tr id="student_{$student.contact.id}" class="{cycle values="odd-row,even-row"} row-relationship">
                    <td class="bold">{$student.contact.display_name}</td>
                    <td>{$student.contact.gender}</td>
                    <td>{$student.contact.birth_date}</td>
        {if $student.contact.is_current_member}
                    <td align='center'><img src="/images/Membership.png"></td>
        {else}
                    <td>{$student.membership.status}</td>
        {/if}
                    <td>{$student.contact.instrument_1}{if $student.contact.instrument_2}, {$student.contact.instrument_2}{/if}{if $student.contact.instrument_3}, {$student.contact.instrument_3}{/if}</td>
                    <td class="nowrap">
                        {$student.links}
                    </td>
                </tr>
    {/foreach}
            </table>
    {/strip}
        </div>
{else}
        <div class="messages status">
            <div class="icon inform-icon"></div> {ts}There are no Students related to this profile.{/ts}
        </div>
{/if}
{if $contact.contact_type eq 'Household'}
        <div class="action-link">
            <a title='New Parent' class='add button box-load'  href='{crmURL p='civicrm/profile/create' q="reset=1&snippet=1&context=boxload&gid=13&osa_hid=`$contact.contact_id`"}&width=50%'><span><div class='icon add-icon'></div> Add a Parent/Guardian </span></a>
            <a title='New Student' class='add button box-load'  href='{crmURL p='civicrm/profile/create' q="reset=1&snippet=1&context=boxload&gid=14&osa_hid=`$contact.contact_id`"}&width=50%'><span><div class='icon add-icon'></div> Add a Student </span></a>
        </div>
{/if}
    </div>
</div>
