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
    var prevPaidAmount = 0.00;
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
      if (theoryName == 'null') theoryName = '';
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

    /* calculate amounts already paid */
    if (($('[name$="_group_class]"]').val()) && (groupDefaults[_contact_id])) {
      grpDefault = groupDefaults[_contact_id];
      key = group.replace(' ', '_');
      if ((groupAmount > 0) && (grpDefault[key])) {
        prevPaidAmount += parseFloat(grpDefault[key].prev_paid);
      }
      if ((theoryAmount > 0) && (grpDefault['Kodaly-Theory'])) {
        prevPaidAmount += parseFloat(grpDefault['Kodaly-Theory'].prev_paid);
      }
    }
    
    /* update the prices being displayed */
    if (memAmount > 0) {
      $("#membership-amount").html(`${memAmount}.00`);
      $("#line-item-membership").show();
    } else {
      $("#line-item-membership").hide();
    }

    if (groupAmount > 0) {
      groupName = groupName.replace(/ \(.*?\)/g,"");
      if ((groupAmount <= 40) && (groupName.includes('Early Childhood') || groupName.includes('Chamber'))) {
        groupName = groupName + " - DEPOSIT";
      } else {
        groupName = groupName + " - Term 3";
      }
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

//    if ((theoryAmount > 0) || (theoryName.includes('Kodaly') && (groupName.includes('Violin Group') || groupName.includes('Cello Group')))) {
//    if ((theoryAmount > 0) || ((theoryName.length > 0) && (groupName.includes('Violin Group') || groupName.includes('Cello Group') || groupName.includes('Chamber')))) {
    if (theoryAmount > 0) {
      theoryName = theoryName.replace(/ \(.*?\)/g,"") + " - Term 3";
      $("#theory-name").html(theoryName);
      $("#theory-amount").html(`${theoryAmount}.00`);
      $("#line-item-theory").show();
    } else {
      $("#line-item-theory").hide();
    }

    if (prevPaidAmount > 0) {
      $("#prev-paid-amount").html(prevPaidAmount.toFixed(2));
      $("#line-item-prev-paid").show();
    } else {
      $("#line-item-prev-paid").hide();
    }

    var totalAmount = memAmount + groupAmount + theoryAmount - prevPaidAmount;
    $("#total-amount").html(`${totalAmount}.00`);
  }

//  $("[name$='_custom_205]']").change(function() {
  $(".webform-client-form :input").change(function() {
    generatePrice();
  });

  // function to set default values if the student is already registered
  function setDefaults(grpDefault) {
    console.log(grpDefault);

    var grp_class = '';
    var found = false;
    var participant;
    var tmp_element = '';

    // loop through the group class select until find match
    $('[name$="_group_class]"] option').each(function(index,item) {
      if (found)
        return;
      grp_class = item.value;
      key = grp_class.replace(' ', '_');
      if (grpDefault[key]) {
        found = true;
        $('[name$="_group_class]"]').val(grp_class).change();
        participant = grpDefault[key];
      }
    });
  
    if (!found)
      return;

    if (participant.custom_104) {
      $('[name$="_preferred_day]"][value="' + participant.custom_104 + '"]').prop('checked', true).change();
      tmp_element = participant.custom_104.toLowerCase();
    }

    if (grp_class == 'ECM') {
      $('[name$="_ecm_level]"]').val(participant.participant_fee_level.substr(0,5)).change();      
    } else if (grp_class == 'Violin Group') {
      $('[name$="_violin_group_'+ tmp_element + ']"]').val(participant.participant_fee_level).change();
    } else if (grp_class == 'Cello Group') {
      $('[name$="_cello_group]"]').val(participant.participant_fee_level).change();
      tmp_element = 'saturday';
    } else if (grp_class == 'Chamber') {
      tmp_element = 'advanced';
    } else if (grp_class == 'Kodaly-Theory') {
    }

    if (grpDefault['Kodaly-Theory']) {
      participant = grpDefault['Kodaly-Theory'];
      $('[name$="_kodaly_theory_option_'+ tmp_element + ']"]').val(participant.participant_fee_level).change();
    }
  }

  // if the student has previously registered, and hasn't selected anything, set the defaults to previous registration values
  if ((!$('[name$="_group_class]"]').val()) && (groupDefaults[_contact_id])) {
    setDefaults(groupDefaults[_contact_id]);
  }

  generatePrice();
});
