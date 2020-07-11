{*
 +--------------------------------------------------------------------+
 | CiviCRM version 5                                                  |
 +--------------------------------------------------------------------+
 | Copyright CiviCRM LLC (c) 2004-2019                                |
 +--------------------------------------------------------------------+
 | This file is a part of CiviCRM.                                    |
 |                                                                    |
 | CiviCRM is free software; you can copy, modify, and distribute it  |
 | under the terms of the GNU Affero General Public License           |
 | Version 3, 19 November 2007 and the CiviCRM Licensing Exception.   |
 |                                                                    |
 | CiviCRM is distributed in the hope that it will be useful, but     |
 | WITHOUT ANY WARRANTY; without even the implied warranty of         |
 | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.               |
 | See the GNU Affero General Public License for more details.        |
 |                                                                    |
 | You should have received a copy of the GNU Affero General Public   |
 | License and the CiviCRM Licensing Exception along                  |
 | with this program; if not, contact CiviCRM LLC                     |
 | at info[AT]civicrm[DOT]org. If you have questions about the        |
 | GNU Affero General Public License or the licensing of CiviCRM,     |
 | see the CiviCRM license FAQ at http://civicrm.org/licensing        |
 +--------------------------------------------------------------------+
*}
<div id="osa-fp-announcement" class="grid_3">
    <p>2019-2020 Registration is now open. Click here to register.</p>
    <button class="osa-fp-button" title="Register Now" onclick="window.location='/osa/form/2019-2020-registration';"><div class="icon register-icon"></div> Register&nbsp;&nbsp;</button>
</div>
<!-- Family Tree -->
<ul class="osa-fp-ul">
  <!-- Start of Family Info -->
  <li class="osa-fp-center">
    <ul class="osa-fp-ul">
      <li class="osa-fp-inline osa-fp-tick-below">
        <fieldset class="grid_4">
          <legend>{$family.display_name}</legend>
          <div class="osa-fp-img">
            <img class="osa-fp-center" src="/images/fp/Household.png" title="Family" />
            <div class="osa-fp-center">
              <button class="osa-fp-button" title="Edit your primary contact information" onclick="{literal}
                CRM.loadForm('/osa/civicrm/profile/edit?reset=1&gid=10&id={/literal}{$family.id}{literal}')
                  .on('crmFormSuccess', function(e,d) {
                    history.go(0);
                  });
                {/literal}"><div class="icon edit-icon"></div>
              </button>
            </div>
          </div>
          <div class="osa-fp-details">
            <p style="padding: 0;">
              {$family.email}<br/>
              {$family.phone}
            </p>
            <hr style="width: 90%; margin: 0.5em 0;"/>
            <p>
              {$family.street_address}<br/>
              {$family.city}, {$family.state_province}<br/>
              {$family.postal_code}
            </p>
            {if ($family.credits_committed >= 0) || ($family.credits_required > 0)}
            <hr style="width: 90%; margin: 0.5em 0;"/>
            <p class="osa-fp-vol-commit" style="width: 85%;" onclick="window.location='/osa/view/volunteer-commitments/{$family.id}';">
              {$family.credits_committed} / {$family.credits_required} Volunteer Commitments
            </p>
            {/if}
          </div>
        </fieldset>
      </li>
    </ul>
  </li>
  <!-- Start of Parents -->
  <li class="osa-fp-center osa-fp-line-above">
    <ul class="osa-fp-ul">
    {foreach from=$parents item=parent}
      <li class="osa-fp-inline osa-fp-tick-below">
        <fieldset class="grid_4">
          <legend>{$parent.contact.display_name}</legend>
          <div class="osa-fp-img">
            <img class="osa-fp-center" src="/images/fp/Parent.png" title="Parent" />
            <div class="osa-fp-center">
              <button class="osa-fp-button" title="Edit this parent's information" onclick="{literal}
                CRM.loadForm('/osa/civicrm/profile/edit?reset=1&gid=13&id={/literal}{$parent.contact_id}{literal}')
                  .on('crmFormSuccess', function(e,d) {
                    history.go(0);
                  });
                {/literal}"><div class="icon edit-icon"></div>
              </button>
              <button class="osa-fp-button" title="Remove this parent from your profile" onclick="{literal}
                CRM.confirm({message: 'Are you sure you want to delete this Parent?', options: {yes: 'Delete', no: 'Cancel'}})
                  .on('crmConfirm:yes', function() {
                    CRM.$('html').addClass('wait');
                    CRM.api('relationship', 'delete', {'id': {/literal}{$parent.relationship.id}{literal}}, {
                      success: function(){
                        history.go(0);
                      }
                    });
                  });
                {/literal}"><div class="icon delete-icon"></div>
              </button>
            </div>
          </div>
          <div class="osa-fp-details">
            <p style="padding: 0;">
              {$parent.contact.email}<br/>
              {$parent.contact.phone}
            </p>
          </div>
        </fieldset>
      </li>
    {/foreach}
    </ul>
  </li>
  <!-- Start of Children -->
  <li class="osa-fp-center osa-fp-line-above">
    <ul class="osa-fp-ul">
    {foreach from=$students item=student}
      <li class="osa-fp-inline">
        <fieldset class="grid_5">
          <legend>{$student.contact.display_name}</legend>
          <div class="osa-fp-img">
          {if $student.contact.instrument_1 }
            <img class="osa-fp-center" src="/images/fp/{$student.contact.instrument_1}.png" title="Primary Instrument" />
          {else}
            <img class="osa-fp-center" src="/images/fp/Student.png" title="Student" />
          {/if}
            <div class="osa-fp-center">
              <button class="osa-fp-button" title="Edit this student's information" onclick="{literal}
                CRM.loadForm('/osa/civicrm/profile/edit?reset=1&gid=14&id={/literal}{$student.contact_id}{literal}')
                  .on('crmFormSuccess', function(e,d) {
                    history.go(0);
                  });
                {/literal}"><div class="icon edit-icon"></div>
              </button>
              <button class="osa-fp-button" title="Remove this student from your profile" onclick="{literal}
                CRM.confirm({message: 'Are you sure you want to delete this Student?', options: {yes: 'Delete', no: 'Cancel'}})
                  .on('crmConfirm:yes', function() {
                    CRM.$('html').addClass('wait');
                    CRM.api('relationship', 'delete', {'id': {/literal}{$student.relationship.id}{literal}}, {
                      success: function(){
                        history.go(0);
                      }
                    });
                  });
                {/literal}"><div class="icon delete-icon"></div>
              </button>
            </div>
          </div>
          <div class="osa-fp-details">
            <table>
              <tr>
                <td>Age:</td>
                <td>{$student.contact.age}</td>
              </tr>
              <tr>
                <td>Membership Status:</td>
                <td>{$student.membership.status}</td>
              </tr>
              <tr>
                <td>Group Class:</td>
                <td>
                {foreach from=$student.group_class item=group_class name=group_class_loop}
                  {$group_class.event_title}{if !$smarty.foreach.group_class_loop.last}<br/>{/if}
                {foreachelse}
                  Not Enrolled
                {/foreach}
                </td>
              </tr>
              <tr>
                <td>Other Events:</td>
                <td>
                {foreach from=$student.participant item=participant name=participant_loop}
                  {$participant.event_title}{if !$smarty.foreach.participant_loop.last}<br/>{/if}
                {foreachelse}
                  Not Enrolled
                {/foreach}
                </td>
              </tr>
            </table>
          </div>
        </fieldset>
      </li>
    {/foreach}
    </ul>
  </li>
</ul>

<div class="osa-fp-action osa-fp-block osa-fp-center">
  <button class="osa-fp-button" title="Register for Membership and Group Classes" onclick="window.location='/osa/form/2019-2020-registration';">
    <div class="icon register-icon"></div> Register for 2019-20&nbsp;&nbsp;
  </button>
  <button class="osa-fp-button" title="Sign up for an Event" onclick="window.location='/osa/events';">
    <div class="icon event-icon"></div> Special Events&nbsp;&nbsp;
  </button>
  <button class="osa-fp-button" title="Add a student to your profile" onclick="{literal}
    CRM.loadForm('/osa/civicrm/profile/create?reset=1&gid=14&hid={/literal}{$family.id}{literal}')
      .on('crmFormSuccess', function(e,d) {
        history.go(0);
      });
    {/literal}"><div class="icon add-icon"></div>
    Add a Student&nbsp;&nbsp;
  </button>
  <button class="osa-fp-button" title="Add another parent to your profile" onclick="{literal}
    CRM.loadForm('/osa/civicrm/profile/create?reset=1&gid=13&hid={/literal}{$family.id}{literal}')
      .on('crmFormSuccess', function(e,d) {
        history.go(0);
      });
    {/literal}"><div class="icon add-icon"></div>
    Add a Parent&nbsp;&nbsp;
  </button>
  <button class="osa-fp-button" title="View your payment history" {literal}onclick="OSA.loadView('family_contributions', 'block', {title: 'Family Contributions'});"{/literal}>
    <div class="icon log-icon"></div> Payment History&nbsp;&nbsp;
  </button>
  <button class="osa-fp-button" title="Search for Volunteer Opportunities" onclick="window.location='/osa/ang/search-volunteer-opportunities';">
    <div class="icon search-icon"></div> Volunteer Opportunities&nbsp;&nbsp;
  </button>
</div>
