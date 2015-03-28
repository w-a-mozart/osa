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
      $("input:radio[name='commerce_payment[payment_method]']").change(function(){
        showContinueBtn();
      });
      
      // initialize the form elements
      showContinueBtn();
      document.forms['commerce-checkout-form-checkout'].elements['commerce_payment[payment_details][virtualmerchant_response]'].value = '';
    }
  }
})(jQuery);
