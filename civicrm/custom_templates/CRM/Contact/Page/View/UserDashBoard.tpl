{*
 +--------------------------------------------------------------------+
 | CiviCRM version 4.7                                                |
 +--------------------------------------------------------------------+
 | Copyright CiviCRM LLC (c) 2004-2016                                |
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
{foreach from=$profileElements item=element}
<fieldset><legend>{$element.title}</legend>
{if $element.edit_gid}
<div class='osa-profile-edit'><a title='{$element.edit_title}' class='edit button box-load'  href='{crmURL p='civicrm/profile/edit' q="reset=1&snippet=1&context=boxload&gid=`$element.edit_gid`&id=`$element.edit_cid`"}&width=50%'><span><div class='icon edit-icon'></div> Edit </span></a></div>
{/if}
{$element.html}
</fieldset>
{/foreach}

{include file="CRM/common/jsortable.tpl" useAjax=0}   

{foreach from=$dashboardElements item=element}
<div class="crm-accordion-wrapper crm-accordion_title-accordion {$element.sectionState}">
    <div class="crm-accordion-header">
        {$element.sectionTitle}
    </div>
    <div class="crm-accordion-body">
        {include file=$element.templatePath}
    </div>
</div>
<br>
{/foreach}

{literal}
<script>
  jQuery('a.box-load').colorbox({width:"50%", opacity:0.6, overlayClose:false});
</script>
{/literal}