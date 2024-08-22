// js to modify the checkout form
(function($) {
  Drupal.behaviors.commercevirtualmerchant = {
    attach: function (context, settings) {

      // add jQuery.validation plugin
      var validator = $('#commerce-checkout-form-checkout').validate({
        focusCleanup: true,
        errorPlacement: function(error, element) {
          if (element[0].id.endsWith('month')) {
            error.appendTo('.commerce-credit-card-expiration');
          } else {
            error.insertAfter(element);
          }
        }
      });
      var current_date = (new Date()).toISOString().replace(/\D/g,'').slice(0,6);
      $.validator.addMethod('expry', function(v,e) {
        var exp_date = $('[id$="exp-year"]').val() + $('[id$="exp-month"]').val();
        return !(exp_date < current_date);
      }, "Expiration  must be in the future.");
      $('#edit-commerce-payment-payment-details-credit-card-number').rules('add', {creditcard: true, required: true});
      $('#edit-commerce-payment-payment-details-credit-card-exp-month').rules('add', {expry: true});

      // clean CC number as they type
      $('#edit-commerce-payment-payment-details-credit-card-number').on('input', function() {
        position = $(this)[0].selectionEnd;
        old_len = $(this).val().length;
        cc_number = $(this).val().replace(/\D/g,'');
        $(this).val((cc_number.substr(0,4) + " " + cc_number.substr(4,4) + " " + cc_number.substr(8,4) + " " + cc_number.substr(12,4)).trim());
        $(this)[0].selectionEnd = position + ($(this).val().length - old_len);

        if (cc_number.length > 0) {
          if (cc_number.at(0) == '4') {
            $('#edit-commerce-payment-payment-details-credit-card-type').val('visa');
          } else {
            $('#edit-commerce-payment-payment-details-credit-card-type').val('mastercard');
          }
          $('#edit-commerce-payment-payment-details-credit-card-type').change();
        }
      });

      // define a function to post the form
      function postForm(btn, status, response) {
        $('#commerce-checkout-form-checkout').validate().settings.ignore = "*";
        $('#edit-commerce-payment-payment-details-credit-card-number').val('');        
        $('#edit-commerce-payment-payment-details-credit-card-exp-month').val('');
        $('#edit-commerce-payment-payment-details-credit-card-exp-year').val('');

        $('[name="commerce_payment[payment_details][virtualmerchant][vms]"]').val(status.toString());
        $('[name="commerce_payment[payment_details][virtualmerchant][vmr]"]').val(response.toString());

        btn.click();
      }

      function paynow() {
        // ensure form inputs are valid
        if ( !$('#commerce-checkout-form-checkout').valid() ) {
          return false;
        }

        // mitiigate double clicks
        $('#vm-paynow').attr('disabled',true).css('opacity', '50%');
        $('span.checkout-processing.element-invisible').removeClass('element-invisible');

        // build Converge request
        var paymentData = {
          ssl_txn_auth_token: $('[name="commerce_payment[payment_details][virtualmerchant][token]"]').val(),
          ssl_card_number: $('#edit-commerce-payment-payment-details-credit-card-number').val().replace(/\D/g,''),
          ssl_exp_date: $('#edit-commerce-payment-payment-details-credit-card-exp-month').val() + $('#edit-commerce-payment-payment-details-credit-card-exp-year').val().slice(-2),
        };

        // add osa specifc values - TODO move to hook (somehow)
        var card_type = $('#edit-commerce-payment-payment-details-credit-card-type').val();
        paymentData.osa_card_type = card_type && card_type[0].toUpperCase() + card_type.slice(1);
        paymentData.osa_name_card = $('#edit-commerce-payment-payment-details-credit-card-owner').val();

        // call Converge
        var callbacks = {
          onError: function (error) {
              postForm($('#edit-continue'), 'error', error);
          },
          onDeclined: function (response) {
              postForm($('#edit-continue'), 'declined', JSON.stringify(response));
          },
          onApproval: function (response) {
              postForm($('#edit-continue'), 'approved', JSON.stringify(response));
          }
        };

        ConvergeEmbeddedPayment.pay(paymentData, callbacks);

        return false;
      }

      function cncl() {
        postForm($('#edit-cancel'), "cancel", '' );
        return false;
      }
      
      // build my own pay & cancel buttons
      function buildBtn() {
        if (!$('#vm-paynow').length) {
          $('<input type=button id="vm-paynow" value="Pay Now" />').insertBefore('#edit-continue');
          $('#vm-paynow').click( paynow );
        }
        if (!$('#vm-cancel').length) {
          $('<input class="checkout-cancel" id="vm-cancel" value="Cancel" />').insertBefore('#edit-cancel');
          $('#vm-cancel').click( cncl );
        }
      }

      // function to toggle buttons
      function toggleBtns() {
        if($("input:radio[name='commerce_payment[payment_method]']:checked").val() == 'commerce_virtualmerchant|commerce_payment_commerce_virtualmerchant') {
          $('#vm-paynow').show();
          $('#edit-continue').hide();
          $('#vm-cancel').show();
          $('#edit-cancel').hide();
        }
        else{
          $('#edit-continue').show();
          $('#vm-paynow').hide();
          $('#edit-cancel').show();
          $('#vm-cancel').hide();
        }
      }

      buildBtn();
      toggleBtns();
    }
  }
})(jQuery);
