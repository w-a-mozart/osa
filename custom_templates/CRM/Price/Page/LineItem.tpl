{*
 +--------------------------------------------------------------------+
 | Custom OSA Price Line Item Page                                    |
 +--------------------------------------------------------------------+
 | Override Standard CiviCRM Template LineItem.tpl                    |
 | - don't show quantity and amount for zero amount items             |
 | - Teacher pay customizations                                       |
 +--------------------------------------------------------------------+
 | Copyright Oakville Suzuki Association 2012                         |
 | Copyright CiviCRM LLC (c) 2004-2011                                |
 +--------------------------------------------------------------------+
*}

{* Displays contribution/event fees when price set is used. *}
{foreach from=$lineItem item=value key=priceset}
    {if $value neq 'skip'}
    {if $lineItem|@count GT 1} {* Header for multi participant registration cases. *}
        {if $priceset GT 0}<br />{/if}
        <strong>{ts}Participant {$priceset+1}{/ts}</strong> {$part.$priceset.info}
    {/if}				 
    <table>
            <tr class="columnheader">
		    <th>{ts}Item{/ts}</th>
	    	{if $context EQ "Membership"}	
		    <th class="right">{ts}Fee{/ts}</th>
                {else}
		    <th class="right">{if $title eq "Teacher Registration"}{ts}Installments{/ts}{else}{ts}Qty{/ts}{/if}</th>
                    <th class="right">{ts}Unit Price{/ts}</th>
		    <th class="right">{ts}Total Price{/ts}</th>
		{/if}
                
	 	{if $pricesetFieldsCount}
		    <th class="right">{ts}Total Participants{/ts}</th>{/if} 
            </tr>
            {foreach from=$value item=line}
            <tr>
              <td>{if $line.html_type eq 'Text'}{$line.label}{else}{$line.field_title} - {$line.label}{/if} {if $line.description}<div class="description">{$line.description}</div>{/if}</td>
{* Dont show qty and price for no-cost items *}
		{if $context NEQ "Membership"}
		    <td class="right">{if $line.qty neq 1 or $line.unit_price neq 0}{$line.qty}{/if}</td>
                    <td class="right">{if $line.qty neq 1 or $line.unit_price neq 0}{$line.unit_price|crmMoney}{/if}</td>
		{/if}
                <td class="right">{if $line.line_total neq 0}{$line.line_total|crmMoney}{/if}</td>
         	{if $pricesetFieldsCount}<td class="right">{$line.participant_count}</td> {/if}
            </tr>
            {/foreach}
    </table>
    {/if}
{/foreach}

<div class="crm-section no-label total_amount-section">
    <div class="content bold">
        {if $context EQ "Contribution"}
            {ts}Contribution Total{/ts}:
        {elseif $context EQ "Event"}
            {ts}Event Total{/ts}: 
 	{elseif $context EQ "Membership"}
            {ts}Membership Fee Total{/ts}: 
        {else}
            {ts}Total Amount{/ts}: 
        {/if}
    {$totalAmount|crmMoney}
    </div>
    <div class="content bold">
      {if $pricesetFieldsCount}
      {ts}Total Participants{/ts}:
      {foreach from=$lineItem item=pcount}
        {if $pcount neq 'skip'}
        {assign var="lineItemCount" value=0}
	
        {foreach from=$pcount item=p_count}
          {assign var="lineItemCount" value=$lineItemCount+$p_count.participant_count}
        {/foreach}
        {if $lineItemCount < 1 }
      	  {assign var="lineItemCount" value=1}
        {/if}
        {assign var="totalcount" value=$totalcount+$lineItemCount}
        {/if} 
      {/foreach}
      {$totalcount}
      {/if}
     </div>    
</div>

{if $hookDiscount.message}
    <div class="crm-section hookDiscount-section">
        <em>({$hookDiscount.message})</em>
    </div>
{/if}
