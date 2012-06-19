{*
 +--------------------------------------------------------------------+
 | Custom OSA Custom Event Data Info Page                             |
 +--------------------------------------------------------------------+
 | Override Standard CiviCRM Template CustomDataView.tpl              |
 | - place in table like the other event info                         |
 +--------------------------------------------------------------------+
 | Copyright Oakville Suzuki Association 2012                         |
 | Copyright CiviCRM LLC (c) 2004-2012                                |
 +--------------------------------------------------------------------+
*}

{* hate putting logic here but what can you do *}
{* first figure out if we should show discounts *}
{assign var="showDiscounts" value=0}
{foreach from=$viewCustomData item=customValues key=customGroupId}
  {foreach from=$customValues item=cd_edit key=cvID}
    {if $cd_edit.name == 'Discounts'}
      {foreach from=$cd_edit.fields item=element key=field_id}
        {if $element.field_data_type == 'Money'}
          {if $element.field_type == 'Text'}
            {if $element.field_value != '0.00'}
              {assign var="showDiscounts" value=1}
            {/if}
          {else}
              {assign var="showDiscounts" value=1}
          {/if}
        {else}
            {assign var="showDiscounts" value=1}
        {/if}
      {/foreach}
    {/if}
  {/foreach}
{/foreach}

{foreach from=$viewCustomData item=customValues key=customGroupId}
  {foreach from=$customValues item=cd_edit key=cvID}

    {if $cd_edit.name != 'Discounts' || $showDiscounts == 1}

<div class="crm-section {$cd_edit.name}-section">
  <div class="label"><label>{$cd_edit.title}</label></div>
  <div class="content">
    <table class="form-layout-compressed {$cd_edit.name}-table">
    {foreach from=$cd_edit.fields item=element key=field_id}
      <tr>
        <td class="crm-event-label">{$element.field_title}</td>
      {if $element.options_per_line != 0}
        <td>
        {foreach from=$element.field_value item=val}
          {$val}<br/>
        {/foreach}
        </td>
      {else}
        {if $element.field_type == 'File'}
          {if $element.field_value.displayURL}
        <td><a href="javascript:imagePopUp('{$element.field_value.imageURL}')" ><img src="{$element.field_value.displayURL}" height = "100" width="100"></a></td>
          {else}
        <td><a href="{$element.field_value.fileURL}">{$element.field_value.fileName}</a></td>
          {/if}
        {else}
          {if $element.field_data_type == 'Money'}
            {if $element.field_type == 'Text'}
        <td class="right">{$element.field_value|crmMoney}</td>
            {else}
        <td>{$element.field_value}</td>
            {/if}
          {else}
        <td>
            {if $element.contact_ref_id}
          <a href='/civicrm/contact/view?reset=1&cid={$element.contact_ref_id}'>
            {/if}
            {$element.field_value}
            {if $element.contact_ref_id}
          </a>
            {/if}
        </td>
          {/if}
        {/if}
      {/if}
      </tr>
    {/foreach}
    </table>
  </div>
</div>

    {/if}

  {/foreach}
{/foreach}
