/**
 * Custom javaScript for Family Profile page
 */
var OSA = OSA || {};

jQuery(document).ready(function($) {

  // load a Drupal View in a CiviCRM formatted dialog (see CRM.loadPage)
  OSA.loadView = function(view_name, display_id, options) {

    // global var for tracking views that are loaded
    OSA.views = OSA.views || {
      dialogCount: 1000,
      url:         Drupal.settings.basePath + 'views/ajax'
    };
    // global vars for specific view
    OSA.views[view_name] = OSA.views[view_name] || {
      title:      (options && options.title) ? options.title : "",
      view_name:  view_name
    };
    OSA.views[view_name].view_display_id = display_id;

    // id of the DOM element to hold the view in a dialog
    var target_id = (options && options.target_id) ? '#' + options.target_id : '#crm-ajax-dialog-' + (OSA.views.dialogCount++);

    // create the DOM element if it doesn't exist
    if (CRM.$(target_id).length == 0) {
      var crm_dialog_settings = {
        title: OSA.views[view_name].title
      };
      crm_dialog_settings = CRM.utils.adjustDialogDefaults(crm_dialog_settings);
      CRM.$('<div id="' + target_id.substring(1) + '"></div>')
        .dialog(crm_dialog_settings);

      // fudge the z-index of the dialog so it is in front of the superfish menus
      CRM.$('.ui-dialog').css('z-index', 1001);
      CRM.$('.ui-widget-overlay').css('z-index', 1000);
      
      // make sure the element is destroyed when it the dialog is closed
      CRM.$(target_id)
        .on('dialogclose', function() {
          CRM.$(this).dialog('destroy').remove();
        });
    }

    // save a ref to the DOM element
    var target_el = CRM.$(target_id);

    // obj to hold AJAX request settings
    var view_ajax_settings = {
      url:  OSA.views.url,
      type: 'post',
      data: {
        view_name:        view_name,
        view_display_id:  display_id,
        view_args:        (options && options.view_args) ? options.view_args : {}
      },
      dataType: 'json',
    };
    if (options && options.view_parms)
      CRM.$.extend(view_ajax_settings.data, options.view_parms);

    // process AJAX response
    view_ajax_settings.success = function (response) {
      if (response[1] !== undefined) {
        // paste the view html into the dialog
        var html = response[1].data;
        target_el.html(html);

        // move the overlay back if we moved it on paging
        CRM.$('.ui-widget-overlay').css('z-index', 1000);
        target_el.scrollTop(0);

        // change the view pager links, so they also use AJAX
        var pager = CRM.$("ul.pager", target_el);
        if (pager.length == 0)
          return;
        var links = CRM.$('a', pager);
        if (links.length == 0)
          return;
        var view_settings = Object.values(response[0].settings.views.ajaxViews)[0];
        links.click(function() {
          // bring the overlay to the foreground during paging
          CRM.$('.ui-widget-overlay').css('z-index', 1002);

          // call load view with the view settings and the page number
          var view_name = view_settings.view_name;
          var tmp = this.href.match(/page=(\d+)/);
          var page_num = (tmp) ? tmp[1] : 0;
          OSA.loadView(view_name, OSA.views[view_name].view_display_id, {target_id: target_el.attr('id'), view_parms: {page: page_num}});
          return false;
        });
      }
    }

    // get the view contents
    CRM.$.ajax(view_ajax_settings);
  };

});
