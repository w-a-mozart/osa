/**
 * javaScript included in the registration webform
 */
jQuery(document).ready(function($) {

  var $checkboxes = $("[name*='submitted[students]']");
  function updateStudentCount() {
    var countCheckedCheckboxes = $checkboxes.filter(':checked').length;
    $('#edit-submitted-number-of-students').val(countCheckedCheckboxes);
  }

  if ($checkboxes) {
    $checkboxes.change(updateStudentCount);
    updateStudentCount();
  }

  // function to close class option (e.g. when they are full)
  function closeOption(select_id, opt_value, reason) {
    var sel = $("[id$='"+ select_id + "']");
    sel.focus(function() {
      var opt = $("[id$='"+ select_id + "'] option[value='full']");
      opt.attr('disabled','disabled');
    });
    sel.change(function() {
      this.blur();
    });
    var opt = $("[id$='"+ select_id + "'] option[value='" + opt_value + "']");
    opt.html(opt.html() + ' - ' + reason);
    opt.attr('value', 'full');
    opt.attr('disabled', 'disabled');
  }

  // function to close checkbox option
  function closeCheckBoxOption(cb_id, reason) {
    var cb = $("[id$='"+ cb_id + "']");
    cb.attr('disabled', 'disabled');
    cb.prop("checked", false);
    var lbl = cb.next();
    lbl.html('<span style="text-decoration-line: line-through;">' + lbl.html() + '</span> &nbsp;' + reason);
    cb.change();
  }

  // function to rename checkbox option
  function relabelCheckBoxOption(cb_id, label) {
    var cb = $("[id$='"+ cb_id + "']");
    var lbl = cb.next();
    lbl.html(label);
  }

  // Hack: the group class and theory classes must be on opposite days
  $("[name$='preferred_day]']").change(function() {
    if (this.value === 'Saturday') {
      $("[id$='preferred-day-kodaly-2']").prop("checked", true);
      $("label[for$='preferred-day-kodaly-2']").css({'color':'red','font-weight':'bold'});
      $("label[for$='preferred-day-kodaly-1']").css({'color':'','font-weight':''});
    } else {
      $("[id$='preferred-day-kodaly-1']").prop("checked", true);
      $("label[for$='preferred-day-kodaly-1']").css({'color':'red','font-weight':'bold'});
      $("label[for$='preferred-day-kodaly-2']").css({'color':'','font-weight':''});      
    }
    $("label[for$='preferred-day-1']").css({'color':'','font-weight':''});
    $("label[for$='preferred-day-2']").css({'color':'','font-weight':''});      
  });
  $("[name$='preferred_day_kodaly]']").change(function() {
    if (this.value === 'Saturday') {
      $("[id$='preferred-day-2']").prop("checked", true);
      $("label[for$='preferred-day-2']").css({'color':'red','font-weight':'bold'});
      $("label[for$='preferred-day-1']").css({'color':'','font-weight':''});
    } else {
      $("[id$='preferred-day-1']").prop("checked", true);
      $("label[for$='preferred-day-1']").css({'color':'red','font-weight':'bold'});
      $("label[for$='preferred-day-2']").css({'color':'','font-weight':''});      
    }
    $("label[for$='preferred-day-kodaly-1']").css({'color':'','font-weight':''});
    $("label[for$='preferred-day-kodaly-2']").css({'color':'','font-weight':''});      
  });

  // Hack:
  // Cello group class is only on Thursday
  // ECM1 is only on Thursday, ECM2 is only on Saturday
  // Beginner Violin is only on Saturday, Kodaly Prep is only on Thursday
  $(".webform-client-form :input").change(function() {
    var group = $("[name$='_group_class]']").val();
    var day_on = '';
    var day_off = '';

    if (group == 'Cello Group') {
      day_on = '2';  // Thursday
      day_off = '1'; // Saturday
    }

    if (group == 'ECM') {
      var ecm_level = $("[name$='_ecm_level]']").val();
      if (!ecm_level) {
        ecm_level = 'ECM 1';
        $("[name$='_ecm_level]']").val(ecm_level);
      }

      if (ecm_level == 'ECM 1') {
        day_on = '2';  // Thursday
        day_off = '1'; // Saturday
      } else { // ECM 2
        day_on = '1'; // Saturday
        day_off = '2';  // Thursday
      }
      $("[id$='kodaly-theory-option']").val('');
    }

    if (group == 'Violin Group') {
      var violin_level = $("[name$='_violin_group]']").val();
      if (violin_level == 'Beginner (30 min.)') {
        day_on = '1'; // Saturday
        day_off = '2';  // Thursday
      }
    }

    var kodaly_level = $("[id$='kodaly-theory-option']");
    if (kodaly_level.val() == 'Kodaly Prep (20 min.)') {
      if (day_on == '2')  {
        kodaly_level.addClass('error');
        alert('You cannot select "Kodaly Prep" since it is on Thursday only');
        kodaly_level.focus();
      } else {
        day_on = '1'; // Saturday
        day_off = '2';  // Thursday - group off, kodaly on
      }
    }

    if (day_on) {
      // enable and disable the appropriate preferred day
      $("[id$='preferred-day-" + day_on + "']").prop("checked", true);
      $("[id$='preferred-day-" + day_off + "']").prop("checked", false).prop('disabled', true);
      $("label[for$='preferred-day-" + day_on + "']").css({'color':'','font-weight':'', 'text-decoration':''});
      $("label[for$='preferred-day-" + day_off + "']").css({'color':'','font-weight':'', 'text-decoration':'line-through 3px'});
      
      // Kodaly / Theory must be on opposite day
      $("[id$='preferred-day-kodaly-" + day_off + "']").prop("checked", true);
      $("[id$='preferred-day-kodaly-" + day_on + "']").prop("checked", false).prop('disabled', true);
      $("label[for$='preferred-day-kodaly-" + day_off + "']").css({'color':'','font-weight':'', 'text-decoration':''});
      $("label[for$='preferred-day-kodaly-" + day_on + "']").css({'color':'','font-weight':'', 'text-decoration':'line-through 3px'});

    } else {
      $("[id$='preferred-day-1']").prop('disabled', false);
      $("[id$='preferred-day-2']").prop('disabled', false);
      $("[id$='preferred-day-kodaly-1']").prop('disabled', false);
      $("[id$='preferred-day-kodaly-2']").prop('disabled', false);
      $("label[for$='preferred-day-1']").css({'text-decoration':''});
      $("label[for$='preferred-day-2']").css({'text-decoration':''});
      $("label[for$='preferred-day-kodaly-1']").css({'text-decoration':''});
      $("label[for$='preferred-day-kodaly-2']").css({'text-decoration':''});
    }
  });

  // add formatting to descriptions
  $(".description").each(function( i ) {
    txt = $(this).html();
    txt = txt.replace("or none of the above", "or <span style=\"font-weight: 900; font-style: italic;\">none of the above</span>");
    $(this).html(txt);
  });

  // Hack: these are for term 1 only
  $("label[for$='enrichment-programs-1']").text('Chamber Music Program - Term 1');
  $("label[for$='enrichment-programs-3']").text('Creative Ability Development (Musical Improvisation) - Term 1');

  // close any full options
  // closeOption('kodaly-theory-option-thursday', 'RCM Theory Levels 3 / 4 / 5 (45 min.)', 'Full');
  // closeOption('kodaly-theory-option-thursday', 'RCM Theory Levels 6 / 7 / 8 (45 min.)', 'Full');
});
