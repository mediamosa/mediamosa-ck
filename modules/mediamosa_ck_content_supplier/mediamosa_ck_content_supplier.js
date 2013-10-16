(function($) {

Drupal.behaviors.contentsupplier = {
  attach: function (context, settings) {

    // Ajax pager fixes. Hard coded for now.
    $('#mediamosa-ck-content-supplier-search-result-wrapper').find('ul.pager > li > a').once(function (id, link) {
      Drupal.mediamosaCK.CS.attachPagerLinkAjax(link, 'mediamosa-ck-content-supplier-search-result-wrapper');
    });

    $('#mediamosa-ck-content-supplier-batch-selection-wrapper').find('ul.pager > li > a').once(function (id, link) {
      Drupal.mediamosaCK.CS.attachPagerLinkAjax(link, 'mediamosa-ck-content-supplier-batch-selection-wrapper');
    });

    $('#mediamosa-ck-content-supplier-queue-listing-wrapper').find('ul.pager > li > a').once(function (id, link) {
      Drupal.mediamosaCK.CS.attachPagerLinkAjax(link, 'mediamosa-ck-content-supplier-queue-listing-wrapper');
    });

    $('#mediamosa-ck-content-supplier-queue-done-listing-wrapper').find('ul.pager > li > a').once(function (id, link) {
      Drupal.mediamosaCK.CS.attachPagerLinkAjax(link, 'mediamosa-ck-content-supplier-queue-done-listing-wrapper');
    });

    $('#mediamosa-ck-content-supplier-queue-done-listing-wrapper').find('ul.pager > li > a').once(function (id, link) {
      Drupal.mediamosaCK.CS.attachPagerLinkAjax(link, 'mediamosa-ck-content-supplier-queue-done-listing-wrapper');
    });

    $('#mediamosa-ck-content-supplier-log-wrapper').find('ul.pager > li > a').once(function (id, link) {
      Drupal.mediamosaCK.CS.attachPagerLinkAjax(link, 'mediamosa-ck-content-supplier-log-wrapper');
    });

    // Click on search.
    $('#edit-mediamosa-ck-content-supplier-search').click(function (e) {
      e.preventDefault();

      // Unhide tab.
      $('#ck-tab-search-result').show();
      jQuery($('#ck-tab-search-result')).trigger('click');
    });

    // Select the inner-most table in case of nested tables.
    //$('th.select-all').closest('table').once('table-select-content-supplier', Drupal.tableSelectContentSupplier);

    $('.mm-ck-cs-checkbox-search').click(function (e) {
      Drupal.mediamosaCK.CS.updateContentSupplier(e.target.value, e.target.checked, true);
    });

    $('.mm-ck-cs-checkbox').click(function (e) {
      Drupal.mediamosaCK.CS.updateContentSupplier(e.target.value, e.target.checked, false);
    });

    // Anchor links in 'Batches' tab opens hidden tab for delete, queueing forms
    // etc. in the tab contents of 'batches'.
    $('a.cs-link-overlay-batch').once('cs-link-overlay').click(function (e) {
      e.preventDefault();

      $('#ck-tab-content-batches').hide();
      $('#ck-tab-overlay-batch').load($(this).attr('href'),'ajax=1',function() {
        $('#ck-tab-overlay-batch').html($(this).find('form'));
      });

      // Show the overlay.
      $('#ck-tab-overlay-batch').show();
    });

    // Handle popup calls.
    $('a.cs-link-popup').once('cs-link-popup').click(function (e) {
      e.preventDefault();

      var popup = $('#mediamosa-ck-cs-popup-full');
      if ($(this).hasClass('cs-popup-left')) {
        popup = $('#mediamosa-ck-cs-popup-left');
      }
      else if ($(this).hasClass('cs-popup-right')) {
        popup = $('#mediamosa-ck-cs-popup-right');
      }

      popup.load($(this).attr('href'),'ajax=1',function() {
        popup.html($(this).find('div.mediamosa-ck-popup'));
      });
    });
  }
};

Drupal.mediamosaCK = Drupal.mediamosaCK || {};
Drupal.mediamosaCK.CS = Drupal.mediamosaCK.CS || {};

/**
 * Set or clear the selection status.
 *
 * @param id
 *   The ID of asset.
 * @param state
 *   The state of the checkbox.
 * @param refresh
 *   Refesh the selection listing?
 *
 * @return boolean
 *   The result either true (success) or false (failure).
 */
Drupal.mediamosaCK.CS.updateContentSupplier = function(id, state, refresh) {
  var result = false;
  $.ajax({
    type: 'GET',
    url: Drupal.settings.basePath + 'mediamosa/ck/json/supplier/selection/state/' + encodeURIComponent(id) + '/' + encodeURIComponent(state),
    dataType: 'json',
    success: function (status) {
      if (refresh) {
        // Refresh.
        jQuery($('#edit-mediamosa-ck-content-supplier-batch-selection-refresh')).trigger('click');

        // Show content selection tab contents.
        $('#ck-tab-content-selection').show();
        $('#ck-tab-content-selection').siblings('.mediamosa-ck-tab-content').hide();

        // Remove the 'active' status of all tabs.
        $('#ck-tab-selection').parent('li').siblings('li').removeClass('active');

        // Make visible.
        $('#ck-tab-selection').parent('li').addClass('active');
      }
      result = true;
    },
    error: function (xmlhttp) {
      alert(Drupal.ajaxError(xmlhttp, 'mediamosa/ck/json/supplier/selection/state'));
    }
  });

  return result;
};

//Drupal.tableSelectContentSupplier = function() {
//  var table = $(this);
////  $('th.select-all input:checkbox:enabled', table).click(function (e) {
////    console.log(e);
////  });
//
//  // When clicked on checkbox, call ajax to store selection.
//  $('.mm-ck-cs-checkbox-selection').click(function (e) {
//    Drupal.mediamosaCK.updateContentSupplier(e.target.value, e.target.checked, TRUE);
////    // Either add or remove the selected class based on the state of the check all checkbox.
////    $(this).closest('tr').toggleClass('selected', this.checked);
////
////    // If this is a shift click, we need to highlight everything in the range.
////    // Also make sure that we are actually checking checkboxes over a range and
////    // that a checkbox has been checked or unchecked before.
////    if (e.shiftKey && lastChecked && lastChecked != e.target) {
////      // We use the checkbox's parent TR to do our range searching.
////      Drupal.tableSelectRange($(e.target).closest('tr')[0], $(lastChecked).closest('tr')[0], e.target.checked);
////    }
////
////    // If all checkboxes are checked, make sure the select-all one is checked too, otherwise keep unchecked.
////    updateSelectAll((checkboxes.length == $(checkboxes).filter(':checked').length));
////
////    // Keep track of the last checked checkbox.
////    lastChecked = e.target;
//  });
//};

/**
 * Attach ajax click link to the pager link.
 *
 * @param link
 *   The <a> link object.
 * @param wrapper
 *   The wrapper to load in.
 */
Drupal.mediamosaCK.CS.attachPagerLinkAjax = function(link, wrapper) {
  var href = $(link).attr('href');
  var pos = href.indexOf('?');
  var path = (pos !== -1 ? href.substring(0, pos) : href);

  var submitPager = {
    wrapper: wrapper,
    path: path
  };

  $.extend(
    submitPager,
    Drupal.mediamosaCK.parseQueryString(href)
  );

  var element_settings = {
    url: '/mediamosa/ck/cs/ajax/paging',
    submit: submitPager,
    setClick: true,
    event: 'click',
    wrapper: wrapper,
    progress: { type: 'throbber' }
  };

  this.pagerAjax = new Drupal.ajax(false, $(link), element_settings);
};

})(jQuery);
