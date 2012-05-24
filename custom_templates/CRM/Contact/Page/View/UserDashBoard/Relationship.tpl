{*
 +--------------------------------------------------------------------+
 | Custom OSA User Dashboard (Family Profile)                         |
 +--------------------------------------------------------------------+
 | Override Standard CiviCRM Template for Relationships               |
 | - use profiles to display and edit contact info                    |
 | - use colorbox library to display dialogues                        |
 | - only shows misc relationships (e.g Emergency Contacts)           |
 +--------------------------------------------------------------------+
 | Copyright Oakville Suzuki Association 2012                         |
 | Copyright CiviCRM LLC (c) 2004-2011                                |
 +--------------------------------------------------------------------+
*}
<div class="view-content">
    <div class="crm-block crm-content-block">
{if $others}
        <div id="other-relationships">
    {strip}
            <table id="other_relationship" class="display">
                <thead>
                    <tr>
                        <th>{ts}Relationship{/ts}</th>
                        <th></th>
                        <th>{ts}Email{/ts}</th>
                        <th>{ts}Phone{/ts}</th>
                        <th></th>
                    </tr>
                </thead>
    {foreach from=$others item=other}
                <tr id="other_{$other.contact.id}" class="{cycle values="odd-row,even-row"} row-relationship">
                    <td>{$other.relationship.relation}</td>
                    <td class="bold">{$other.contact.display_name}</td>
                    <td>{$other.contact.email}</td>
                    <td>{$other.contact.phone}</td>
                    <td class="nowrap">
                        {$other.links}
                    </td>
                </tr>
    {/foreach}
            </table>
    {/strip}
        </div>
{else}
        <div class="messages status">
            <div class="icon inform-icon"></div> {ts}There are no other people related to this profile.{/ts}
        </div>
{/if}
{if $contact.contact_type eq 'Household'}
        <div class="action-link">
            <a title='New Person' class='add button box-load'  href='{crmURL p='civicrm/profile/create' q="reset=1&snippet=1&context=boxload&gid=2&osa_hid=`$contact.contact_id`"}&width=50%'><span><div class='icon add-icon'></div> Add an Individual </span></a>
        </div>
{/if}
    </div>
</div>
