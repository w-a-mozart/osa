/**
 * javaScript included in the volunteer_duties component in the registration webform
 *
 * manually bootstraps the AngularJS app provided by the CiviVolunteer extension
 * Note: the crm_volunteer_angular_frame <div> is in the webform body, since it
 * needs to be positioned outside of the <form> element
 */
var $scope;  // initialized below by AngularJS, but needed outside of AngularJS
var $route;

jQuery(document).ready((function($, angular, CRM, _) {return function() {

  // start the bootstrap of AngularJS
  var frame = document.getElementById("crm_volunteer_angular_frame");
  angular.element(function() {
    angular.bootstrap(frame, ['crmApp']);
  });

  // disable the 'Submit' button until the user has picked all of their
  // volunteer requirements or selected to pay in lieu
  var submitButton = $('input[value="Add items to Cart"]').prop('disabled', true);
  var payInLieuCB = $('#edit-submitted-donation-in-lieu-1').change(function() {
    updateSubmitButton();
  });
  var needIDs = $('[name="submitted[volunteer_duties]"]').change(function() {
    updateSubmitButton();
    updatePayinLieu();
  });

  // set the 'Submit' button's state
  function updateSubmitButton() {
    var disabled = true;
    if (CRM.vars.osa.paidInLieu || payInLieuCB.prop('checked')) {
      disabled = false;
    }
    if (disabled && $scope) {
      if ($scope.totalCreditsSelected >= $scope.totalCreditsRequired) {
        disabled = false;
      }
    }
    submitButton.prop('disabled', disabled);
  };

  // show/hide 'Pay in Lieu' button
  function updatePayinLieu() {
    if (CRM.vars.osa.paidInLieu) {
      if (payInLieuCB.is(':visible')) {
        payInLieuCB.prop('checked', false).trigger('change');
        jQuery('#edit-submitted-donation-in-lieu-1').trigger('change'); // civicrm and webform use different instances of jQuery
        $('.webform-component--donation-in-lieu').hide();
      }
      return;
    }

    if ($scope) {
      if (($scope.totalCreditsSelected >= $scope.totalCreditsRequired) && payInLieuCB.is(':visible')) {
        payInLieuCB.prop('checked', false).trigger('change');
        jQuery('#edit-submitted-donation-in-lieu-1').trigger('change'); // civicrm and webform use different instances of jQuery
        $('.webform-component--donation-in-lieu').hide();
      }
      else if (($scope.totalCreditsSelected < $scope.totalCreditsRequired) && payInLieuCB.is(':hidden')) {
        $('.webform-component--donation-in-lieu').show();
      } 
    }
  };

  // wait for AngularJS to load
  angular.element(document).ready(function($) {
    var rootScope = angular.element('#crm_volunteer_angular_frame').scope();

    // wait for the AngularJS content to load
    rootScope.$on('$viewContentLoaded', function(event) {
      $scope = angular.element('#crm_volunteer_angular_view').scope();

      // add javascript to the AngularJS controller in VolOppsCtrl.js
      // note there is some code that had to be modified directly
      angular.extend($scope, {
        totalCreditsRequired: CRM.vars.osa.totalCreditsRequired,
        creditsPrevSelected:  CRM.vars.osa.creditsPrevSelected,
        needsFromSavedForm:   CRM.vars.osa.needsFromSavedForm,
        sortedRoles: _.sortBy(_.map($scope.roles, function (value, key) {return {key: key, value: value};}), 'value'),
        helpText: function() {
          return 'Use this form to find volunteer opportunities that match with your interests, and time availability, then click the checkbox next to all of the opportunities you are willing to commit to fulfill.';
        },
        showRoleDescription: function (need) {
          var description = (need.description) ? need.description : '';
          if (need.role_description)
            description = need.role_description + ((need.description) ? ' - ' + need.description : '');
          CRM.alert(description, need.role_label, 'info', {expires: 0});
        },
        afterFirstLoad: function() {
          // re-select any saved volunteer selections
          var needs = $scope.volOppData();
          var needIds = $scope.needsFromSavedForm.split(',');
          
          // hack for Internet Explorer (easier than using shim library)
          if (!needs.find) { needs.find = function(f) { for (var i = 0; i < needs.length; i++) { if (f(needs[i])) return needs[i]; }; return false; }; }

          // loop through previously selected items
          needIds.forEach(function(needId) {
            need = needs.find(function(n) {return n.id == needId;});
            if (need) {
              need.inCart = true;
              $scope.shoppingCart[need.id] = need;
            }
          });
          // hide the 'spinner'
          $('#form-loading').hide();
        },
      });

      $scope.sumCreditsSelected = function() {
        return _.reduce($scope.shoppingCart, function(memo, need) {
          return memo + Number(need.duration);
        }, 0);
      };

      $scope.$watch('shoppingCart', function(oldValue, newValue) {
        $scope.totalCreditsSelected = CRM.vars.osa.creditsPrevSelected + $scope.sumCreditsSelected();
        $('[name="submitted[volunteer_duties]"]').val(_.keys($scope.shoppingCart)).trigger('change');
      }, true);

      updateSubmitButton();
    });
  });

  // add 'Are you sure' confirmation to the 'submit' button
  // - you'd think this would be easier
  var allowSubmit = false;
  submitButton.click(function() {
    if (!allowSubmit) {
      CRM.confirm({
        message: 'Your <span style="color:#0000FF;"><strong>volunteer selections</strong></span> will now be <span style="color:#0000FF;"><strong>recorded</strong></span> and your <span style="color:#FF0000;"><strong>fees for membership and group class</strong></span> will be added to your <span style="color:#FF0000;"><strong>shopping cart</strong></span>.',
        modal: true,
        width: '50%',
      }).on('crmConfirm:yes', function() {
        // turn the loading gif back on
        $('#form-loading').show();
        // reclick the submit button 
        allowSubmit = true;
        submitButton.trigger('click');
      });
    }
    return allowSubmit;
  });

}})(CRM.$, angular, CRM, CRM._));
