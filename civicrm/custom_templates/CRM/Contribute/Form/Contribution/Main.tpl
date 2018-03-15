{*
 +--------------------------------------------------------------------+
 | CiviCRM version 4.7                                                |
 +--------------------------------------------------------------------+
 | Copyright CiviCRM LLC (c) 2004-2017                                |
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
{* Callback snippet: On-behalf profile *}
{if $snippet and !empty($isOnBehalfCallback) and !$ccid}
  <div class="crm-public-form-item crm-section">
    {include file="CRM/Contribute/Form/Contribution/OnBehalfOf.tpl" context="front-end"}
  </div>
{else}
  {literal}
  <script type="text/javascript">

  // Putting these functions directly in template so they are available for standalone forms
  function useAmountOther() {
    var priceset = {/literal}{if $contriPriceset}'{$contriPriceset}'{else}0{/if}{literal};

    for( i=0; i < document.Main.elements.length; i++ ) {
      element = document.Main.elements[i];
      if ( element.type == 'radio' && element.name == priceset ) {
        if (element.value == '0' ) {
          element.click();
        }
        else {
          element.checked = false;
        }
      }
    }
  }

  function clearAmountOther() {
    var priceset = {/literal}{if $priceset}'#{$priceset}'{else}0{/if}{literal}
    if( priceset ){
      cj(priceset).val('');
      cj(priceset).blur();
    }
    if (document.Main.amount_other == null) return; // other_amt field not present; do nothing
    document.Main.amount_other.value = "";
  }

  </script>
  {/literal}

  {if $action & 1024}
  {include file="CRM/Contribute/Form/Contribution/PreviewHeader.tpl"}
  {/if}

  {include file="CRM/common/TrackingFields.tpl"}

  <div class="crm-contribution-page-id-{$contributionPageID} crm-block crm-contribution-main-form-block">

  {* Remove 'Not You' message *}

  <div id="intro_text" class="crm-public-form-item crm-section intro_text-section">
    {$intro_text}
  </div>
  {* Remove include file="CRM/common/cidzero.tpl" *}
  {if $islifetime or $ispricelifetime }
  <div class="help">{ts}You have a current Lifetime Membership which does not need to be renewed.{/ts}</div>
  {/if}

{* Family selection block *}
{if $onbehalfFamily }
  <div class="crm-section contact-id-section">
    <fieldset>
      <legend>{ts}Individual{/ts}</legend>
      <div class="label">{$form.contact_id.label}</div>
      <div class="content">
        {$form.contact_id.html}
        {if $members_only}<br /><em style="font-size:.8em">Restricted to OSA Members only.</em>{/if}
    </fieldset>
  </div>
  <div class="clear"></div> 
{/if}
{* End of Family selection block *}

{* Move Custom fields *}
    <div class="crm-group custom_pre_profile-group">
      {include file="CRM/UF/Form/Block.tpl" fields=$customPre}
    </div>
{* End of Move Custom fields *}

  {if !empty($useForMember) && !$ccid}
    <div class="crm-public-form-item crm-section">
      {include file="CRM/Contribute/Form/Contribution/MembershipBlock.tpl" context="makeContribution"}
    </div>
  {elseif !empty($ccid)}
    {if $lineItem && $priceSetID && !$is_quick_config}
      <div class="header-dark">
        {ts}Contribution Information{/ts}
      </div>
      {assign var="totalAmount" value=$pendingAmount}
      {include file="CRM/Price/Page/LineItem.tpl" context="Contribution"}
    {else}
      <div class="display-block">
        <td class="label">{$form.total_amount.label}</td>
        <td><span>{$form.total_amount.html|crmMoney}&nbsp;&nbsp;{if $taxAmount}(includes {$taxTerm} of {$taxAmount|crmMoney}){/if}</span></td>
      </div>
    {/if}
  {else}
    <div id="priceset-div">
      <fieldset>
        <legend>{ts}Options{/ts}</legend>
    {include file="CRM/Price/Form/PriceSet.tpl" extends="Contribution"}
      {if $paymentProcessor.payment_processor_type eq 'drupalcommerce'} 
        <div class="discount-info">Applicable discounts will be applied in the shopping cart.</div>
      {/if}
      </fieldset>
    </div>
  {/if}

  {if !$ccid}
    {crmRegion name='contribution-main-pledge-block'}
    {if $pledgeBlock}
      {if $is_pledge_payment}
      <div class="crm-public-form-item crm-section {$form.pledge_amount.name}-section">
        <div class="label">{$form.pledge_amount.label}&nbsp;<span class="crm-marker">*</span></div>
        <div class="content">{$form.pledge_amount.html}</div>
        <div class="clear"></div>
      </div>
      {else}
        <div class="crm-public-form-item crm-section {$form.is_pledge.name}-section">
          <div class="label">&nbsp;</div>
          <div class="content">
            {$form.is_pledge.html}&nbsp;
            {if $is_pledge_interval}
              {$form.pledge_frequency_interval.html}&nbsp;
            {/if}
            {$form.pledge_frequency_unit.html}<span id="pledge_installments_num">&nbsp;{ts}for{/ts}&nbsp;{$form.pledge_installments.html}&nbsp;{ts}installments.{/ts}</span>
          </div>
          <div class="clear"></div>
          {if $start_date_editable}
            {if $is_date}
              <div class="label">{$form.start_date.label}</div><div class="content">{include file="CRM/common/jcalendar.tpl" elementName=start_date}</div>
            {else}
              <div class="label">{$form.start_date.label}</div><div class="content">{$form.start_date.html}</div>
            {/if}
          {else}
            <div class="label">{$form.start_date.label}</div>
            <div class="content">{$start_date_display|date_format}</div>
          {/if}
        <div class="clear"></div>
        </div>
      {/if}
    {/if}
    {/crmRegion}

    {if $form.is_recur}
    <div class="crm-public-form-item crm-section {$form.is_recur.name}-section">
      <div class="label">&nbsp;</div>
      <div class="content">
        {$form.is_recur.html} {$form.is_recur.label} {ts}every{/ts}
        {if $is_recur_interval}
          {$form.frequency_interval.html}
        {/if}
        {if $one_frequency_unit}
          {$frequency_unit}
          {else}
          {$form.frequency_unit.html}
        {/if}
        {if $is_recur_installments}
          <span id="recur_installments_num">
          {ts}for{/ts} {$form.installments.html} {$form.installments.label}
          </span>
        {/if}
        <div id="recurHelp" class="description">
          {$recurringHelpText}
        </div>
      </div>
      <div class="clear"></div>
    </div>
    {/if}
    {if $pcpSupporterText}
    <div class="crm-public-form-item crm-section pcpSupporterText-section">
      <div class="label">&nbsp;</div>
      <div class="content">{$pcpSupporterText}</div>
      <div class="clear"></div>
    </div>
    {/if}
    {if $showMainEmail}
      {assign var=n value=email-$bltID}
      <div class="crm-public-form-item crm-section {$form.$n.name}-section">
        <div class="label">{$form.$n.label}</div>
        <div class="content">
          {$form.$n.html}
        </div>
        <div class="clear"></div>
      </div>
    {/if}

    <div id='onBehalfOfOrg' class="crm-public-form-item crm-section">
      {include file="CRM/Contribute/Form/Contribution/OnBehalfOf.tpl"}
    </div>

    {* User account registration option. Displays if enabled for one of the profiles on this page. *}
    <div class="crm-public-form-item crm-section cms_user-section">
      {include file="CRM/common/CMSUser.tpl"}
    </div>
    <div class="crm-public-form-item crm-section premium_block-section">
      {include file="CRM/Contribute/Form/Contribution/PremiumBlock.tpl" context="makeContribution"}
    </div>

    {if $honoreeProfileFields|@count}
      <fieldset class="crm-public-form-item crm-group honor_block-group">
        {crmRegion name="contribution-soft-credit-block"}
          <legend>{$honor_block_title}</legend>
          <div class="crm-public-form-item crm-section honor_block_text-section">
            {$honor_block_text}
          </div>
          {if $form.soft_credit_type_id.html}
            <div class="crm-public-form-item crm-section {$form.soft_credit_type_id.name}-section">
              <div class="content" >
                {$form.soft_credit_type_id.html}
                <div class="description">{ts}Select an option to reveal honoree information fields.{/ts}</div>
              </div>
            </div>
          {/if}
        {/crmRegion}
        <div id="honorType" class="honoree-name-email-section">
          {include file="CRM/UF/Form/Block.tpl" fields=$honoreeProfileFields mode=8 prefix='honor'}
        </div>
      </fieldset>
    {/if}

{* Move Custom fields *}

    {if $isHonor}
    <fieldset class="crm-public-form-item crm-group pcp-group">
      <div class="crm-public-form-item crm-section pcp-section">
        <div class="crm-public-form-item crm-section display_in_roll-section">
          <div class="content">
            {$form.pcp_display_in_roll.html} &nbsp;
            {$form.pcp_display_in_roll.label}
          </div>
          <div class="clear"></div>
        </div>
        <div id="nameID" class="crm-public-form-item crm-section is_anonymous-section">
          <div class="content">
            {$form.pcp_is_anonymous.html}
          </div>
          <div class="clear"></div>
        </div>
        <div id="nickID" class="crm-public-form-item crm-section pcp_roll_nickname-section">
          <div class="label">{$form.pcp_roll_nickname.label}</div>
          <div class="content">{$form.pcp_roll_nickname.html}
            <div class="description">{ts}Enter the name you want listed with this contribution. You can use a nick name like 'The Jones Family' or 'Sarah and Sam'.{/ts}</div>
          </div>
          <div class="clear"></div>
        </div>
        <div id="personalNoteID" class="crm-public-form-item crm-section pcp_personal_note-section">
          <div class="label">{$form.pcp_personal_note.label}</div>
          <div class="content">
            {$form.pcp_personal_note.html}
            <div class="description">{ts}Enter a message to accompany this contribution.{/ts}</div>
          </div>
          <div class="clear"></div>
        </div>
      </div>
    </fieldset>
    {/if}

  {* end of ccid loop *}
  {/if}

  {if $form.payment_processor_id.label}
    <div id="payment_options-div">
  {* PP selection only works with JS enabled, so we hide it initially *}
  <fieldset class="crm-public-form-item crm-group payment_options-group" style="display:none;">
    <legend>{ts}Payment Options{/ts}</legend>
    <div class="crm-public-form-item crm-section payment_processor-section">
      <div class="label">{$form.payment_processor_id.label}</div>
      <div class="content">{$form.payment_processor_id.html|replace:'&nbsp;':'<br/>'}</div>
      <div class="clear"></div>
    </div>
  </fieldset>
    </div>
  {/if}

  {if $is_pay_later}
  <fieldset class="crm-public-form-item crm-group pay_later-group">
    <legend>{ts}Payment Options{/ts}</legend>
    <div class="crm-public-form-item crm-section pay_later_receipt-section">
      <div class="label">&nbsp;</div>
      <div class="content">
        [x] {$pay_later_text}
      </div>
      <div class="clear"></div>
    </div>
  </fieldset>
  {/if}

  <div id="billing-payment-block">
    {include file="CRM/Financial/Form/Payment.tpl" snippet=4}
  </div>
  {include file="CRM/common/paymentBlock.tpl"}

  <div class="crm-public-form-item crm-group custom_post_profile-group">
  {include file="CRM/UF/Form/Block.tpl" fields=$customPost}
  </div>

  {if $is_monetary and $form.bank_account_number}
  <div id="payment_notice">
    <fieldset class="crm-public-form-item crm-group payment_notice-group">
      <legend>{ts}Agreement{/ts}</legend>
      {ts}Your account data will be used to charge your bank account via direct debit. While submitting this form you agree to the charging of your bank account via direct debit.{/ts}
    </fieldset>
  </div>
  {/if}

  {if $isCaptcha}
    {include file='CRM/common/ReCAPTCHA.tpl'}
  {/if}
  <div id="crm-submit-buttons" class="crm-submit-buttons">
  {include file="CRM/common/formButtons.tpl" location="bottom"}
  </div>
  {if $footer_text}
  <div id="footer_text" class="crm-public-form-item crm-section contribution_footer_text-section">
    <p>{$footer_text}</p>
  </div>
  {/if}
</div>
<script type="text/javascript">
  {if $isHonor}
  pcpAnonymous();
  {/if}

  {literal}

  cj('input[name="soft_credit_type_id"]').on('change', function() {
    enableHonorType();
  });

  function enableHonorType( ) {
    var selectedValue = cj('input[name="soft_credit_type_id"]:checked');
    if ( selectedValue.val() > 0) {
      cj('#honorType').show();
    }
    else {
      cj('#honorType').hide();
    }
  }

  cj('input[id="is_recur"]').on('change', function() {
    toggleRecur();
  });

  function toggleRecur( ) {
    var isRecur = cj('input[id="is_recur"]:checked');
    var allowAutoRenew = {/literal}'{$allowAutoRenewMembership}'{literal};
    var quickConfig = {/literal}{$quickConfig}{literal};
    if ( allowAutoRenew && cj("#auto_renew") && quickConfig) {
      showHideAutoRenew( null );
    }
    if (isRecur.val() > 0) {
      cj('#recurHelp').show();
      cj('#amount_sum_label').text('{/literal}{ts escape='js'}Regular amount{/ts}{literal}');
    }
    else {
      cj('#recurHelp').hide();
      cj('#amount_sum_label').text('{/literal}{ts escape='js'}Total Amount{/ts}{literal}');
    }
  }

  function pcpAnonymous( ) {
    // clear nickname field if anonymous is true
    if (document.getElementsByName("pcp_is_anonymous")[1].checked) {
      document.getElementById('pcp_roll_nickname').value = '';
    }
    if (!document.getElementsByName("pcp_display_in_roll")[0].checked) {
      cj('#nickID').hide();
      cj('#nameID').hide();
      cj('#personalNoteID').hide();
    }
    else {
      if (document.getElementsByName("pcp_is_anonymous")[0].checked) {
        cj('#nameID').show();
        cj('#nickID').show();
        cj('#personalNoteID').show();
      }
      else {
        cj('#nameID').show();
        cj('#nickID').hide();
        cj('#personalNoteID').hide();
      }
    }
  }

  CRM.$(function($) {
    enableHonorType();
    toggleRecur();
    skipPaymentMethod();
  });

  CRM.$(function($) {
    // highlight price sets
    function updatePriceSetHighlight() {
      $('#priceset .price-set-row span').removeClass('highlight');
      $('#priceset .price-set-row input:checked').parent().addClass('highlight');
    }
    $('#priceset input[type="radio"]').change(updatePriceSetHighlight);
    updatePriceSetHighlight();

    // Update pledge contribution amount when pledge checkboxes change
    $("input[name^='pledge_amount']").on('change', function() {
      var total = 0;
      $("input[name^='pledge_amount']:checked").each(function() {
        total += Number($(this).attr('amount'));
      });
      $("input[name^='price_']").val(total.toFixed(2));
    });
  });
  {/literal}
</script>


{* jQuery validate *}
{* disabled because more work needs to be done to conditionally require credit card fields *}
{*include file="CRM/Form/validate.tpl"*}

{* Teacher Registration customizations *}
{if $contributionPageID eq 2}
{literal}
<script type="text/javascript">
function setLevelOpt() {
  var pricelvl = {/literal}{$pricelvl}{literal};
  var tidx = cj('#custom_24').val();
  if (typeof tidx === "undefined") tidx = 0;

  var el = cj("#custom_32");
  cj("#custom_32").empty(); // remove old options

  if (cj(pricelvl[tidx]).length > 0) {
    cj.each(pricelvl[tidx], function(value, label) {
      cj("#custom_32").append(cj("<option></option>")
        .attr("value", value).text(label));
    });
  }
}

function setLessonAmt() {
  var pricelvl = {/literal}{$pricelvl}{literal};
  var price30 = {/literal}{$price30}{literal};
  var price45 = {/literal}{$price45}{literal};
  var price60 = {/literal}{$price60}{literal};

  showHideByValue('custom_18','QE Park','priceset-div','block','radio',false);
  showHideByValue('custom_18','QE Park','payment_options-div','block','radio',false);
  showHideByValue('custom_18','QE Park','billing-payment-block','block','radio',false);

  var tidx = cj('#custom_24').val();
  var lidx = cj('#custom_32').val();
  var plvl = pricelvl[tidx];

  if ((typeof tidx === "undefined") || (typeof lidx === "undefined") || (typeof plvl === "undefined")) {
    tidx = 0;
    lidx = 0;
  }

  if (cj('#CIVICRM_QFID_QE_Park_2:checked').val() != 'QE Park') {
    tidx = 0;
    lidx = 0;
  }

  len = cj("select#custom_32 option").length;
  if (typeof len === "undefined") len = 0;

  if (tidx == 0 || len <= 1) {
    cj("#custom_32").val(pricelvl[tidx]);
    cj("#editrow-custom_32").css("visibility", "hidden");
  }
  else {
    cj("#editrow-custom_32").css("visibility", "visible");
  }

  var price_attr = '[\"price_6\",\"' + price30[tidx][lidx] + '||\"]';
  cj('#CIVICRM_QFID_11_8').attr('price', price_attr);
  document.getElementById('30min').innerHTML = " - " + symbol + " " + formatMoney( price30[tidx][lidx], 2, seperator, thousandMarker) + ' / year';

  price_attr = '[\"price_6\",\"' + price45[tidx][lidx] + '||\"]';
  cj('#CIVICRM_QFID_12_10').attr('price', price_attr);
  document.getElementById('45min').innerHTML = " - " + symbol + " " + formatMoney( price45[tidx][lidx], 2, seperator, thousandMarker) + ' / year';

  price_attr = '[\"price_6\",\"' + price60[tidx][lidx] + '||\"]';
  cj('#CIVICRM_QFID_13_12').attr('price', price_attr);
  document.getElementById('60min').innerHTML = " - " + symbol + " " + formatMoney( price60[tidx][lidx], 2, seperator, thousandMarker) + ' / year';
  
  calcTotal();
}

cj(document).ready(function() {
  calcTotal();
  setLessonAmt();
  setLevelOpt();
  
  cj("#custom_24").change(function(){
    setLevelOpt();
    setLessonAmt();
  });
  cj("#custom_32").change(function(){
    setLessonAmt();
  });
  cj("#CIVICRM_QFID_QE_Park_2").change(function(){
    setLevelOpt();
    setLessonAmt();
  });
  cj("#CIVICRM_QFID_Other_4").change(function(){
    setLevelOpt();
    setLessonAmt();
  });
});
</script>
<style type="text/css">
.yearly-fee {
  position: relative;
  top: -1.35em;
  left: 100px;
  margin-bottom: -1.2em;
}
</style>
{/literal}
{/if} {* End of Teacher Registration customizations *}
{if ($contributionPageID eq 10) or ($contributionPageID eq 11)} {* Camp Food Order (or Camp Teacher Food Order) *}
{literal}
<script type="text/javascript">
  var p_week1 = '#price_152';
  var p_week2 = '#price_153';
{/literal}
{if ($contributionPageID eq 11)} {* Camp Teacher (no Wed meals)*}
{literal}
  p_week1 = '#price_160';
  p_week2 = '#price_161';
{/literal}
{/if} {* End Camp Teacher *}
{literal}

function resetWeekMeals(week, plans) {
  if (week == 1) {
    cj('#custom_137').val(plans);
    cj('#custom_138').val(0);
    cj('#custom_139').val(0);
    cj('#custom_140').val(plans);
    cj('#custom_141').val(0);
    cj('#custom_142').val(0);
    cj('#custom_143').val(plans);
    cj('#custom_144').val(0);
    cj('#custom_145').val(0);
    cj('#custom_146').val(plans);
    cj('#custom_147').val(0);
    cj('#custom_148').val(0);
    cj('#custom_149').val(plans);
    cj('#custom_150').val(0);
    cj('#custom_151').val(0);
    if (plans > 0) {
      cj("[id^=editrow-custom]:lt(10)").show();
      cj("[id^=helprow-custom]:lt(5)").show();
    }
    else {
      cj("[id^=editrow-custom]:lt(10)").hide();
      cj("[id^=helprow-custom]:lt(5)").hide();
    }
  }
  else {
    cj('#custom_152').val(plans);
    cj('#custom_153').val(0);
    cj('#custom_154').val(0);
    cj('#custom_155').val(plans);
    cj('#custom_156').val(0);
    cj('#custom_157').val(0);
    cj('#custom_158').val(plans);
    cj('#custom_159').val(0);
    cj('#custom_160').val(0);
    cj('#custom_161').val(plans);
    cj('#custom_162').val(0);
    cj('#custom_163').val(0);
    cj('#custom_164').val(plans);
    cj('#custom_165').val(0);
    cj('#custom_166').val(0);
    if (plans > 0) {
      cj("[id^=editrow-custom]:gt(9)").show();
      cj("[id^=helprow-custom]:gt(4)").show();
    }
    else {
      cj("[id^=editrow-custom]:gt(9)").hide();
      cj("[id^=helprow-custom]:gt(4)").hide();
    }
  }

  if ((cj(p_week1).val() > 0) && (cj(p_week2).val() > 0)) {
    cj("#hr").show();
  }
  else {
    cj("#hr").hide();
  }

  {/literal}
{if ($contributionPageID eq 11)} {* Camp Teacher (no Wed meals)*}
{literal}
  cj('#custom_143').val(0);
  cj('#editrow-custom_143').hide();
  cj('#editrow-custom_144').hide();
  cj('#editrow-custom_145').hide();
  cj('#helprow-custom_143 .content.description').html("Wednesday - Special Teacher's Lunch");
  cj('#custom_158').val(0);
  cj('#editrow-custom_158').hide();
  cj('#editrow-custom_159').hide();
  cj('#editrow-custom_160').hide();
  cj('#helprow-custom_158 .content.description').html("Wednesday - Special Teacher's Lunch");
{/literal}
{/if} {* End Camp Teacher *}
{literal}
}

function setDayMeals(changed, other, fixed, plans) {

  if (parseInt(changed.val()) > plans) {
    changed.val(plans);
    other.val(0);
//    fixed.val(0);
  }
  else if ((parseInt(changed.val()) + parseInt(other.val())) > plans) {
    other.val(plans - changed.val());
//    fixed.val(0);
  }
  else {
//    fixed.val(plans - changed.val() - other.val());
    other.val(plans - changed.val());
  }
}

function stopRKey(evt) { 
  var evt = (evt) ? evt : ((event) ? event : null); 
  var node = (evt.target) ? evt.target : ((evt.srcElement) ? evt.srcElement : null); 
  if ((evt.keyCode == 13) && (node.type=="text"))  {return false;} 
} 

cj(document).ready(function() {
  resetWeekMeals(1, cj(p_week1).val());
  resetWeekMeals(2, cj(p_week2).val());

  cj(p_week1).change(function(){
    resetWeekMeals(1, cj(p_week1).val());
  });
  cj(p_week2).change(function(){
    resetWeekMeals(2, cj(p_week2).val());
  });

  cj("[id^=custom]").change(function(){
    var c_id, o_id, f_id, p_id;
    c_id = cj(this).attr('id');
    if (c_id == 'custom_137') {o_id = '#custom_138'; f_id = '#custom_139'; p_id = p_week1;}
    if (c_id == 'custom_138') {o_id = '#custom_137'; f_id = '#custom_139'; p_id = p_week1;}
    if (c_id == 'custom_139') {o_id = '#custom_137'; f_id = '#custom_138'; p_id = p_week1;}
    if (c_id == 'custom_140') {o_id = '#custom_141'; f_id = '#custom_142'; p_id = p_week1;}
    if (c_id == 'custom_141') {o_id = '#custom_140'; f_id = '#custom_142'; p_id = p_week1;}
    if (c_id == 'custom_142') {o_id = '#custom_140'; f_id = '#custom_141'; p_id = p_week1;}
    if (c_id == 'custom_143') {o_id = '#custom_144'; f_id = '#custom_145'; p_id = p_week1;}
    if (c_id == 'custom_144') {o_id = '#custom_143'; f_id = '#custom_145'; p_id = p_week1;}
    if (c_id == 'custom_145') {o_id = '#custom_143'; f_id = '#custom_144'; p_id = p_week1;}
    if (c_id == 'custom_146') {o_id = '#custom_147'; f_id = '#custom_148'; p_id = p_week1;}
    if (c_id == 'custom_147') {o_id = '#custom_146'; f_id = '#custom_148'; p_id = p_week1;}
    if (c_id == 'custom_148') {o_id = '#custom_146'; f_id = '#custom_147'; p_id = p_week1;}
    if (c_id == 'custom_149') {o_id = '#custom_150'; f_id = '#custom_151'; p_id = p_week1;}
    if (c_id == 'custom_150') {o_id = '#custom_149'; f_id = '#custom_151'; p_id = p_week1;}
    if (c_id == 'custom_151') {o_id = '#custom_149'; f_id = '#custom_150'; p_id = p_week1;}

    if (c_id == 'custom_152') {o_id = '#custom_153'; f_id = '#custom_154'; p_id = p_week2;}
    if (c_id == 'custom_153') {o_id = '#custom_152'; f_id = '#custom_154'; p_id = p_week2;}
    if (c_id == 'custom_154') {o_id = '#custom_152'; f_id = '#custom_153'; p_id = p_week2;}
    if (c_id == 'custom_155') {o_id = '#custom_156'; f_id = '#custom_157'; p_id = p_week2;}
    if (c_id == 'custom_156') {o_id = '#custom_155'; f_id = '#custom_157'; p_id = p_week2;}
    if (c_id == 'custom_157') {o_id = '#custom_155'; f_id = '#custom_156'; p_id = p_week2;}
    if (c_id == 'custom_158') {o_id = '#custom_159'; f_id = '#custom_160'; p_id = p_week2;}
    if (c_id == 'custom_159') {o_id = '#custom_158'; f_id = '#custom_160'; p_id = p_week2;}
    if (c_id == 'custom_160') {o_id = '#custom_158'; f_id = '#custom_159'; p_id = p_week2;}
    if (c_id == 'custom_161') {o_id = '#custom_162'; f_id = '#custom_163'; p_id = p_week2;}
    if (c_id == 'custom_162') {o_id = '#custom_161'; f_id = '#custom_163'; p_id = p_week2;}
    if (c_id == 'custom_163') {o_id = '#custom_161'; f_id = '#custom_162'; p_id = p_week2;}
    if (c_id == 'custom_164') {o_id = '#custom_165'; f_id = '#custom_166'; p_id = p_week2;}
    if (c_id == 'custom_165') {o_id = '#custom_164'; f_id = '#custom_166'; p_id = p_week2;}
    if (c_id == 'custom_166') {o_id = '#custom_164'; f_id = '#custom_165'; p_id = p_week2;}

    setDayMeals(cj(this), cj(o_id), cj(f_id), cj(p_id).val());
  });

  cj("div[id^=editrow-custom]").css("width", "70%");
  cj("div[id^=editrow-custom] .label").css({"width": "10em"});
  cj("div[id^=editrow-custom] .content").css({"margin-left": "20%", "width": "25%"});
/*  cj("div[id=editrow-custom_161] .label").css({"margin-left": "-8%", "width": "20%"}); */
  
  document.onkeypress = stopRKey;
});
</script>
<style type="text/css">
div[id^=helprow-custom] {
  margin-left: -28%;
  font-size: 125%;
}
.crm-profile-name-Camp_Food_Order_51 .crm-form-text {
  width: 2em;
  text-align: right;
  margin-left: 2.5em;
}
.content.description {
  font-weight: bold;
  width: 72%;
  margin-top: .5em;
}
#editrow-custom_140,
#editrow-custom_141,
#editrow-custom_142,
#editrow-custom_146,
#editrow-custom_147,
#editrow-custom_148,
#editrow-custom_155,
#editrow-custom_156,
#editrow-custom_157,
#editrow-custom_161,
#editrow-custom_162,
#editrow-custom_163 {
  background-color: #FAFAFA;
}
#editrow-custom_137,
#editrow-custom_140,
#editrow-custom_143,
#editrow-custom_146,
#editrow-custom_149,
#editrow-custom_152,
#editrow-custom_155,
#editrow-custom_158,
#editrow-custom_161,
#editrow-custom_164 {
  margin-left: 2em;
}
#editrow-custom_138,
#editrow-custom_141,
#editrow-custom_144,
#editrow-custom_147,
#editrow-custom_150,
#editrow-custom_153,
#editrow-custom_156,
#editrow-custom_159,
#editrow-custom_162,
#editrow-custom_165 {
  margin-top: -4em;
  margin-left: 20em;
}
#editrow-custom_139,
#editrow-custom_142,
#editrow-custom_145,
#editrow-custom_148,
#editrow-custom_151,
#editrow-custom_154,
#editrow-custom_157,
#editrow-custom_160,
#editrow-custom_163,
#editrow-custom_166 {
  margin-top: -4em;
  margin-left: 28em;
}
</style>
{/literal}
{/if} {* End of Camp Food Order *}
{/if} {* End Main Form *}  
