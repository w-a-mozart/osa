/**
 * JavaScript for the Summer Camp webform.
 */

(function ($) {

  Drupal.behaviors.osaCamp = {
    attach: function (context, settings) {
      if (settings.osa_camp.display) {
        $('#osa-camp-block').show();
        $('#osa-camp-markup').html(settings.osa_camp.markup);
      }
      else {
        $('#osa-camp-block').hide();
      }
    }
  };

})(jQuery);
