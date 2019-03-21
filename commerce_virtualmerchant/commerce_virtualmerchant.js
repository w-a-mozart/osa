// js to modify the checkout form
(function($) {
  Drupal.behaviors.commercevirtualmerchant = {
    attach: function (context, settings) {

      // function to toggle Pay Now button
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

      // check to see if we have the response from VM
      // if so, hide everything and auto submit the form for validation
      if (document.forms['commerce-checkout-form-checkout'].elements['commerce_payment[payment_details][virtualmerchant_response]'].value) {
        $('#edit-cart-contents').hide();
        $('#edit-commerce-payment-payment-method').hide();
        $('#edit-buttons').hide();
        $('#edit-continue').click();
      }
      // otherwise use the form as normal
      else {
        // set event handler on payment method
        $("input:radio[name='commerce_payment[payment_method]']").change(function(){
          showContinueBtn();
        });

        // initialize the form elements
        showContinueBtn();
      }
    }
  }
})(jQuery);
