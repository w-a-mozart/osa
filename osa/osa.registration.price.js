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
    } else if (selectedOption.includes('Parent Training')) {
      optionKey = 'Parent_Training';
    } else if (selectedOption.includes('Reading')) {
      optionKey = 'Reading';
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
    var memAmount = ((membership) && (Number(membership.end_date.substring(0,4)) >= Number(schoolDates.end.date.substring(0,4)))) ? 0 : 95;
    var groupName = '';
    var groupAmount = 0;
    var theoryName = '';
    var theoryAmount = 0;
    var group = $("[name$='_group_class]']").val();
    var preferredDay = $("[name*='_preferred_day]']:checked").val();
    if (preferredDay) { preferredDay = preferredDay.toLowerCase(); }

    /* hack - Kodaly Only cannot choose Reading */
    if ((group == 'Kodaly-Theory') && (preferredDay == 'thursday')) {
      $("[name$='_theory_option_thursday]'] option[value^='Reading']").hide();
      var tmpStr = '' + $("[name$='_theory_option_thursday]']").val();
      if (tmpStr.includes('Reading')) {
        $("[name$='_theory_option_thursday]']").val('');
      }
    } else {
      $("[name$='_theory_option_thursday]'] option[value^='Reading']").show();
    }
    /* end of hack */

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
      if (!group) { group = ''; }
      var grpPriceKeys = [ group ];
      var thryPriceKeys = [ group ];
      var theorySel = "[name$='_theory_option_" + preferredDay + "]']";

      /* for Violin or Cello groups, find the level / sub-class */
      if ((group == 'Violin Group') || (group == 'Cello Group')) {
        var level = (group == 'Violin Group') ?
                    $("[name$='_violin_group_" + preferredDay + "]']").val() :
                    $("[name$='_cello_group]']").val();
        if (level) {
          groupName = group + ' - ' + level;
          grpPriceKeys.push(parseOptionKey(level));
        }
        /* Cello students can only take theory on Saturday due to scheduling conflicts */
        if (group == 'Cello Group') { theorySel = "[name$='_theory_option_saturday]']"; }

      /* for advanced Violin use the advance theory options */
      } else if ((group == 'Preludio') || (group == 'Vivace')) {
        groupName = group.concat(' Violin Ensemble');
        theorySel = "[name$='_theory_option_advanced]']";

      /* for chamber groups also use the advance theory options */
      } else if (group == 'Chamber') {
        groupName = 'Chamber Music Program';
        theorySel = "[name$='_theory_option_advanced]']";
      
      /* for Kodaly / Theory only, unset the group class settings */
      } else if (group == 'Kodaly-Theory') {
        groupName = '';
        grpPriceKeys = [];
      }
      
      /* now determine the Kodaly / Theory option */
      theoryName += $(theorySel).val();
      thryPriceKeys.push(parseOptionKey(theoryName));
      if (theoryName.includes('Reading')) {
        var bundleOption = $("[name$='_theory_option_bundle]']:enabled").val();
        if (bundleOption) {
          theoryName += ' + ' + bundleOption;
          thryPriceKeys.push(parseOptionKey(bundleOption));
        }
      }
      
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

    if ((theoryAmount > 0) || (theoryName.includes('Kodaly') && (groupName.includes('Violin Group') || groupName.includes('Cello Group')))) {
      theoryName = theoryName.replace(/ \(.*?\)/g,"");
      $("#theory-name").html(theoryName);
      $("#theory-amount").html(`${theoryAmount}.00`);
      $("#line-item-theory").show();
    } else {
      $("#line-item-theory").hide();
    }

    var totalAmount = memAmount + groupAmount + theoryAmount;
    $("#total-amount").html(`${totalAmount}.00`);
  }

  $(".webform-client-form :input").change(function() {
    generatePrice();
  });

  generatePrice();
});
