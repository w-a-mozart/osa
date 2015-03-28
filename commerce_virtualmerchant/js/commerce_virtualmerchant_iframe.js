/*
 * Code used to control the iframe used to hold the Converge payment form.
 * Requires: iframeResizer.min.js
 */
(function($) {
  Drupal.behaviors.commercevirtualmerchantiframe = {
    attach: function (context, settings) {
    
      $('#virtualmerchant_iframe').iFrameResize({enablePublicMethods: true});
    
    }
  }
})(jQuery);
