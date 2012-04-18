{*
 +--------------------------------------------------------------------+
 | Custom OSA User Dashboard (Family Profile)                         |
 +--------------------------------------------------------------------+
 | Override Standard CiviCRM Template for User Dashboard              |
 | - use profiles to display and edit contact info                    |
 | - use colorbox library to display dialogues                        |
 | - use crm-accordion instead of tables                              |
 +--------------------------------------------------------------------+
 | Copyright Oakville Suzuki Association 2012                         |
 | Copyright CiviCRM LLC (c) 2004-2011                                |
 +--------------------------------------------------------------------+
*}
{foreach from=$profileElements item=element}
<fieldset><legend>{$element.title}</legend>
{$element.html}
<div style='position:relative;top:-45px;right:-90%'><a title='{$element.title}' class='edit button box-load'  href='{crmURL p='civicrm/profile/edit' q="reset=1&snippet=1&context=boxload&gid=`$element.gid`&id=`$element.cid`"}&width=50%'><span><div class='icon edit-icon'></div> Edit </span></a></div>
</fieldset>
{/foreach}

{include file="CRM/common/jsortable.tpl" useAjax=0}   

{foreach from=$dashboardElements item=element}
<div class="crm-accordion-wrapper crm-accordion_title-accordion crm-accordion-{$element.sectionState}">
    <div class="crm-accordion-header">
        <div class="icon crm-accordion-pointer"></div>
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
    cj(function() {
        cj().crmaccordions();
    });
</script>
{/literal}