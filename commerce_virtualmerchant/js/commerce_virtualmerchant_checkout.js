// js to modify the checkout form
(function($) {
  Drupal.behaviors.commercevirtualmerchant = {
    attach: function (context, settings) {

      function showContinueBtn() {
        if($("input:radio[name='commerce_payment[payment_method]']:checked").val() == 'commerce_virtualmerchant|commerce_payment_commerce_virtualmerchant') {
          $('#edit-continue').hide();
          $('.button-operator').hide();
        }
        else{
          $('#edit-continue').show();
          $('.button-operator').show();
        }
      }

      // if the user selected our payment method, hide the Pay Now button
      showContinueBtn();
      $("input:radio[name='commerce_payment[payment_method]']").change(function(){
        showContinueBtn();
      });
      
      count = 0;
      
      $(document).ajaxComplete(function(event, request, settings) {
        if (count > 0) {
          return;
        }
        
        if ($("input:radio[name='commerce_payment[payment_method]']:checked").val() == 'commerce_virtualmerchant|commerce_payment_commerce_virtualmerchant') {
          $('#virtualmerchant_iframe').iFrameResize({enablePublicMethods: true});
        }
      });
    }
  }
})(jQuery);
