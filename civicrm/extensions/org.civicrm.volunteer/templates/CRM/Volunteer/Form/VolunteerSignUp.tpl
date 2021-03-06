<div class="crm-block crm-volunteer-signup-form-block">

  <div class="help">
      {ts domain='org.civicrm.volunteer'}Thank you for being a volunteer! You are registering for the following volunteer commitments:{/ts}
  </div>

  <p class="description">
    {ts domain='org.civicrm.volunteer'}For additional detail, click the corresponding detail icon in the table below.{/ts}
    <span class="icon ui-icon-comment"></span>
  </p>

  <div class="crm-volunteer-signup-summary">
    <table>
      <tr>
        <th class="crm-vol-opp-time">{ts domain='org.civicrm.volunteer'}Date and Time{/ts}</th>
        <th class="crm-vol-opp-project">Event</th>
        <th class="crm-vol-opp-role">{ts domain='org.civicrm.volunteer'}Role{/ts}</th>
        <th class="crm-vol-opp-description">Notes</th>
        <th class="ccrm-vol-opp-duration">Credits</th>
      </tr>
      {foreach from=$volunteerNeeds key=key item=volunteerNeed}
        <tr>
          <td class="crm-vol-opp-time">{$volunteerNeed.display_time}</td>
          <td class="crm-vol-opp-project">
            {$volunteerNeed.project.title}
            {if $volunteerNeed.project.description}
              <span class="icon ui-icon-comment crm-vol-description">
                <div class="vol-project-description-wrapper">{$volunteerNeed.project.description}</div>
              </span>
            {/if}
          </td>
          <td class="crm-vol-opp-role">
            {$volunteerNeed.role_label}
            {if $volunteerNeed.role_description}
              <span class="icon ui-icon-comment crm-vol-description">
                <div class="vol-role-description-wrapper">{$volunteerNeed.role_description}</div>
              </span>
            {/if}
          </td>
          <td class="crm-vol-opp-description">{$volunteerNeed.description}</td>
          <td class="crm-vol-opp-credits">{$volunteerNeed.duration}</td>
        </tr>
      {/foreach}
    </table>
  </div>

  <div class="help">
    Please confirm the following information and click Submit.
  </div>

  <div class="crm-volunteer-signup-profiles">
    {foreach from=$customProfiles key=ufID item=ufFields }
      {include file="CRM/UF/Form/Block.tpl" fields=$ufFields}
    {/foreach}
  </div>

  {if $allowAdditionalVolunteers}

    <fieldset class="crm-volunteer-additional-volunteers-section">
      <legend>{ts domain='org.civicrm.volunteer'}Additional Volunteers{/ts}</legend>
      <div class="crm-section">
        <div class="label">{$form.additionalVolunteerQuantity.label}</div>
        <div class="content">{$form.additionalVolunteerQuantity.html}</div>
        <div class="clear"></div>
      </div>

      <div class="crm-volunteer-additional-volunteers" id="additionalVolunteers">
        {if $additionalVolunteerProfiles}
          {foreach from=$additionalVolunteerProfiles item=additionalVolunteer }
            <div class='additional-volunteer-profile'>
              {foreach from=$additionalVolunteer.profiles key=ufID item=ufFields }
                {include file="CRM/UF/Form/Block.tpl" fields=$ufFields prefix=$additionalVolunteer.prefix}
              {/foreach}
              <div class="clear"></div>
            </div>
          {/foreach}
        {/if}
      </div>
    </fieldset>
  {/if}

  <div>
    {include file="CRM/common/formButtons.tpl" location="bottom"}
  </div>
</div>

{if $allowAdditionalVolunteers}
</form>
<form>
  <div class="crm-volunteer-additional-volunteers-template">
    <div class='additional-volunteer-profile'>
      {foreach from=$additionalVolunteersTemplate key=ufID item=ufFields }
        {include file="CRM/UF/Form/Block.tpl" fields=$ufFields prefix='additionalVolunteersTemplate'}
      {/foreach}
      <div class="clear"></div>
    </div>
  </div>
{/if}

{include file="CRM/common/notifications.tpl" location="bottom"}
