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
    
    cb.click(function(event) {
      event.stopPropagation();
      $(this).attr('disabled', 'disabled');
      $(this).prop("checked", false);
    });
  }

  // function to rename checkbox option
  function relabelCheckBoxOption(cb_id, label) {
    var cb = $("[id$='"+ cb_id + "']");
    var lbl = cb.next();
    lbl.html(label);
  }

  // add formatting to descriptions
  $(".description").each(function( i ) {
    txt = $(this).html();
    txt = txt.replace("or none of the above", "or <span style=\"font-weight: 900; font-style: italic;\">none of the above</span>");
    txt = txt.replaceAll("\n",'<br/>').replaceAll('{','<').replaceAll('}','>');
    $(this).html(txt);
  });

  // add formatting to required checkbox options
  $("label.option[for*='-confirmation']").each(function( i ) {
    txt = $(this).html();
    txt = txt.replace("*", "<span class=\"form-required\" style=\"font-weight: 900; font-style: italic;\" title=\"This field is required.\">*</span>");
    $(this).html(txt);
  });

  // change formatting on confirmation check box
  $("[name*='_confirmation]']").change(function() {
    cb_id = $(this).attr('id');
    if ($(this).prop('checked')) {
      $("label.option[for*='"+ cb_id + "']").css({'color': 'grey', 'font-weight': 'normal'});
    } else {
      $("label.option[for*='"+ cb_id + "']").css({'color': '', 'font-weight': ''});     
    }
  });

  // add formatting to label
  $("label[for*='enrichment-programs']").each(function( i ) {
    txt = $(this).html();
    txt = txt.replaceAll("\n",'<br/>').replaceAll('{','<').replaceAll('}','>');
    $(this).html(txt);
  });


  // close any full options
  // closeOption('kodaly-theory-option-thursday', 'RCM Theory Levels 3 / 4 / 5 (45 min.)', 'Full');
  // closeCheckBoxOption('ecm-term-1', 'Closed'); // ECM Term 1
  // closeCheckBoxOption('ecm-term-2', 'Closed'); // ECM Term 2
  // closeCheckBoxOption('enrichment-programs-1', 'Closed'); // Chamber Term 1
  // closeCheckBoxOption('enrichment-programs-2', 'Closed'); // Chamber Term 2
});
