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

  // hack to remove Preludio and Vivace
  $("[name$='_group_class]'] option[value^='Preludio']").remove();
  $("[name$='_group_class]'] option[value^='Vivace']").remove();
  $('[title="Preludio Violin Ensemble"]').hide().prev().hide();
  $('[title="Vivace Violin Ensemble"]').hide().prev().hide();
  // hack to remove Reading
  $("[name$='_kodaly_theory_option_thursday]'] option[value^='Reading']").remove();
  

  // function to close class option when they are full
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

  // function to close checkbox type option
  function closeCheckBoxOption(cb_id, reason) {
    var cb = $("[id$='"+ cb_id + "']");
    cb.attr('disabled', 'disabled');
    cb.prop("checked", false);
    var lbl = cb.next();
    lbl.html('<span style="text-decoration-line: line-through;">' + lbl.html() + '</span> &nbsp;' + reason);
    cb.change();
  }

  // function to close checkbox type option
  function relabelCheckBoxOption(cb_id, label) {
    var cb = $("[id$='"+ cb_id + "']");
    var lbl = cb.next();
    lbl.html(label);
  }

  /* Covid-hacks for ECM
  // closeCheckBoxOption('ecm-term-1', 'CANCELLED due to Covid-19');
  // relabelCheckBoxOption('ecm-term-2', 'Term 2 (6 classes. Jan - March Break)');
  // closeCheckBoxOption('ecm-term-2', ' <span style="color: red;"> <b>FULL</b></span>');
  // closeCheckBoxOption('ecm-term-3', 'schedule pending');

  function covidHacksForECM(group) {
    var saturday_rb = $("[id$='preferred-day-1']");
    if (group == 'ECM') {
      saturday_rb.prop("checked", false);
      saturday_rb.parent().hide();
      saturday_rb.change();
      var thursday_rb = $("[id$='preferred-day-2']");
      thursday_rb.prop("checked", true);
      thursday_rb.change();
      
      $("[id$='ecm-term-1']").attr('disabled', 'disabled');
      $("[id$='ecm-term-2']").attr('disabled', 'disabled');
      // $("[id$='ecm-term-3']").attr('disabled', 'disabled');
    } else {
      saturday_rb.parent().show();
    }
  }

  var gc_sel = $("[id$='group-class']");
  covidHacksForECM(gc_sel.val());
  gc_sel.change(function() {
    covidHacksForECM(gc_sel.val());
  });
  */

  // close any full options
  // closeOption('kodaly-theory-option-thursday', 'RCM Theory Levels 3 / 4 / 5 (45 min.)', 'Full');
  // closeOption('kodaly-theory-option-thursday', 'RCM Theory Levels 6 / 7 / 8 (45 min.)', 'Full');

});
