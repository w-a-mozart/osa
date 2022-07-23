/**
 * javaScript included in the student_*_registration_price component in the registration webform
 * requires the global priceTable and schoolDates vars, defined in osa.registration.inc
 */
jQuery(document).ready(function($) {
  var membership = {end_date: ''};
  var _contact_id = $("[name*='_contact_existing]']").val();

  CRM.api3('membership', 'get', {contact_id: _contact_id})
    .done(function(result) {
      if (result.values) {
        membership = result.values[result.id];
      }
  });

  function parseOptionKey(selectedOption) {
    var optionKey = '';
    var minRegEx = /...min/;

    if (selectedOption.includes('RCM Theory')) {
      optionKey = 'RCM';
    } else {
      if (selectedOption.includes('Kodaly')) {
        optionKey = 'Kodaly_';
      }
      var minutes = minRegEx.exec(selectedOption);
      if (Array.isArray(minutes) && (minutes.length > 0)) {
        optionKey += minutes[0];
      }
    }

    return optionKey;
  }

  function generatePrice() {
    var memAmount = ((membership) && (Number(membership.end_date.substring(0,4)) >= Number(schoolDates.end.date.substring(0,4)))) ? 0 : priceTable['Membership'];
    var groupName = '';
    var groupAmount = 0;
    var theoryName = '';
    var theoryAmount = 0;
    var enhTotal = 0;

    var group = $("[name$='_group_class]']").val();
    if (!group) { group = 'none'; }
    var thryPriceKeys = [ group ];
    var grpPriceKeys = [ group ];

    var preferredDay = $("[name*='_preferred_day]']:checked").val();
    if (!preferredDay) { preferredDay = ''; }
    var preferredDayKodaly = $("[name*='_preferred_day_kodaly]']:checked").val();
    if (!preferredDayKodaly) { preferredDayKodaly = ''; }

    /* determine pricing based on selected group and theory options */

    /* special code for ECM */
    if (group == 'ECM') {
      memAmount = 0;
      groupName = 'Early Childhood Music';

      $("[name*='_ecm_term]']:checked").each(function(index,item){
        var checked_value = item.name.replace(/\]/g,'').split(/\[/).pop();
        var price_idx = group.concat('_', checked_value).replace(/\s/g,'_');
        groupAmount += isNaN(priceTable[price_idx]) ? 0 : priceTable[price_idx];
      });
      if (groupAmount > priceTable.ECM_Max) {
        groupAmount = priceTable.ECM_Max;
      }

    /* code for non-ECM */
    } else {

      /* for Violin or Cello groups, find the level / sub-class */
      if ((group == 'Violin Group') || (group == 'Cello Group')) {
        var level = (group == 'Violin Group') ?
                    $("[name$='_violin_group]']").val() :
                    $("[name$='_cello_group]']").val();
        if (level) {
          groupName = group + ' - ' + level + ' - ' + preferredDay;
          grpPriceKeys.push(parseOptionKey(level));
        }

      /* otherwise unset the group class settings */
      } else {
        groupName = '';
        grpPriceKeys = [];
      }
      
      /* determine the Kodaly / Theory option */
      var theorySel = "[name$='kodaly_theory_option]']";
      theoryName += $(theorySel).val();
      if (theoryName == 'null') theoryName = '';
      thryPriceKeys.push(parseOptionKey(theoryName));
      theoryName += ' - ' + preferredDayKodaly;
      
      /* get the price from the price table */
      var price_idx = grpPriceKeys.join('_').replace(/\s|-/g, '_');
      groupAmount = isNaN(priceTable[price_idx]) ? 0 : priceTable[price_idx];
      price_idx = thryPriceKeys.join('_').replace(/\s|-/g, '_');
      theoryAmount = isNaN(priceTable[price_idx]) ? 0 : priceTable[price_idx];
    } /* end of non-ECM */

    /* update the prices being displayed */
    if (memAmount > 0) {
      $("#membership-amount").html(`${memAmount}.00`);
      $("#line-item-membership").show();
    } else {
      $("#line-item-membership").hide();
    }

    if (groupAmount > 0) {
      groupName = groupName.replace(/ \(.*?\)/g,"");
      $("#group-name").html(groupName);
      $("#group-amount").html(`${groupAmount}.00`);
      $("#line-item-group").show();
    } else {
      if (group == 'Chamber') {
        $("#group-name").html(groupName);
        $("#group-amount").html('<b>tbd</b>');
        $("#line-item-group").show();
      } else {
        $("#line-item-group").hide();
      }
    }

    if (theoryAmount > 0) {
      theoryName = theoryName.replace(/ \(.*?\)/g,"");
      $("#theory-name").html(theoryName);
      $("#theory-amount").html(`${theoryAmount}.00`);
      $("#line-item-theory").show();
    } else {
      $("#line-item-theory").hide();
    }

    $("[name*='enrichment_programs]']").each(function( i ) {
      ths = $(this);
      lid = '#line-item-enhance-' + (i+1);

      if(ths.prop('checked')) {
        var name = ths.prop('name');
        var val  = name.substring(name.lastIndexOf("[") + 1, name.length - 1);
        enhAmount = isNaN(priceTable[val]) ? 0 : priceTable[val];
        enhTotal += enhAmount;

        $("#enhance-" + (i+1) + "-name").html(ths.next().text().replace(/ \(.*?\)/g,""));
        $("#enhance-" + (i+1) + "-amount").html(`${enhAmount}.00`);
        $(lid).show();
      } else {
        $(lid).hide();
      }
    });

    // previous paid not used
    $("#line-item-prev-paid").hide();
    
    var totalAmount = memAmount + groupAmount + theoryAmount + enhTotal;
    $("#total-amount").html(totalAmount.toFixed(2));
  }

  // re-calculate prices when something changes
  $(".webform-client-form :input").change(function() {
    generatePrice();
  });

  // dynamically build rows in price table for enhancement programs
  numEnh = $("[name*='enrichment_programs]']").size();
  theory_line = $("#line-item-theory");
  last_line = theory_line;
  $("[name*='enrichment_programs]']").each(function( i ) {
    new_line = theory_line.clone().prop('id','line-item-enhance-' + (i+1));
    new_line.html(theory_line.html().replaceAll('theory-', 'enhance-' + (i+1) + '-'));
    last_line.after(new_line);
    last_line = new_line;
  });
  
  generatePrice();
});
