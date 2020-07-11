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

  // function to close class option when they are full
  function closeOption(select_id, opt_value) {
    var sel = $("[id$='"+ select_id + "']");
    sel.focus(function() {
      var opt = $("[id$='"+ select_id + "'] option[value='full']");
      opt.attr('disabled','disabled');
    });
    sel.change(function() {
      this.blur();
    });
    var opt = $("[id$='"+ select_id + "'] option[value='" + opt_value + "']");
    opt.html(opt.html() + ' - Full');
    opt.attr('value', 'full');
    opt.attr('disabled', 'disabled');
  }

  // thursday RCM is full
  closeOption('kodaly-theory-option-thursday', 'RCM Theory Levels 3 / 4 / 5 (45 min.)');
  closeOption('kodaly-theory-option-thursday', 'RCM Theory Levels 6 / 7 / 8 (45 min.)');
  
  // Saturday ECM1 9:30 is full
  closeOption('ecm-saturday-time', '9:30am');

});
