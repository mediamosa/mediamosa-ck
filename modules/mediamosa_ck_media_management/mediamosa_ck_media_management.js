(function($) {

Drupal.behaviors.MediaManagement = {
  attach: function (context, settings) {

    // Ajax pager fixes. Hard coded for now.
    $('#mediamosa-ck-media-management-search-result-wrapper').find('ul.pager > li > a').once(function (id, link) {
      Drupal.mediamosaCK.MM.attachPagerLinkAjax(link, 'mediamosa-ck-media-management-search-result-wrapper');
    });

    $('#mediamosa-ck-media-management-batch-selection-wrapper').find('ul.pager > li > a').once(function (id, link) {
      Drupal.mediamosaCK.MM.attachPagerLinkAjax(link, 'mediamosa-ck-media-management-batch-selection-wrapper');
    });

    $('#mediamosa-ck-media-management-queue-listing-wrapper').find('ul.pager > li > a').once(function (id, link) {
      Drupal.mediamosaCK.MM.attachPagerLinkAjax(link, 'mediamosa-ck-media-management-queue-listing-wrapper');
    });

    $('#mediamosa-ck-media-management-queue-done-listing-wrapper').find('ul.pager > li > a').once(function (id, link) {
      Drupal.mediamosaCK.MM.attachPagerLinkAjax(link, 'mediamosa-ck-media-management-queue-done-listing-wrapper');
    });

    $('#mediamosa-ck-media-management-queue-done-listing-wrapper').find('ul.pager > li > a').once(function (id, link) {
      Drupal.mediamosaCK.MM.attachPagerLinkAjax(link, 'mediamosa-ck-media-management-queue-done-listing-wrapper');
    });

    $('#mediamosa-ck-media-management-log-wrapper').find('ul.pager > li > a').once(function (id, link) {
      Drupal.mediamosaCK.MM.attachPagerLinkAjax(link, 'mediamosa-ck-media-management-log-wrapper');
    });

    // Click on search.
    $('#edit-mediamosa-ck-media-management-search').click(function (e) {
      e.preventDefault();

      // Unhide tab.
      $('#ck-tab-search-result').show();
      jQuery($('#ck-tab-search-result')).trigger('click');
    });

    $('#ck-tab-selection').once().click(function (e) {
      jQuery($('#edit-mediamosa-ck-media-management-batch-selection-refresh')).trigger('click');
    });

    $('.mm-ck-mm-checkbox-search').once().click(function (e) {
      Drupal.mediamosaCK.MM.updateMediaManagement(e.target.value, e.target.checked, true);
      var checkboxes = $('table.mediamosa-ck-media-management-search-result').find('input:checkbox.mm-ck-mm-checkbox-search');
      var checked = $(checkboxes).filter(':checked').length;
      $('table.mediamosa-ck-media-management-search-result').find('input:checkbox.mm-ck-mm-checkbox-search-all').each(function() {
        this.checked = checkboxes.length && (checkboxes.length === checked);
      });
    });

    $('.mm-ck-mm-checkbox-search-all').once().click(function (e) {
      Drupal.mediamosaCK.MM.updateMediaManagement(e.target.value, e.target.checked, true);

      $('table.mediamosa-ck-media-management-search-result').find('input:checkbox.mm-ck-mm-checkbox-search').each(function() {
        this.checked = e.target.checked;
      });
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


    $('input.ck-mm-search-text-submit-on-enter').keypress(function(e) {
      // If keypress enter.
      if (e.keyCode === 13) {
        jQuery($('#edit-mediamosa-ck-media-management-search')).trigger('click');
      }
    });

  }
};

Drupal.mediamosaCK = Drupal.mediamosaCK || {};
Drupal.mediamosaCK.MM = Drupal.mediamosaCK.MM || {};

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
Drupal.mediamosaCK.MM.updateMediaManagement = function(id, state, refresh) {
  var result = false;
  $.ajax({
    type: 'GET',
    url: Drupal.settings.basePath + 'mediamosa/ck/json/mediamanagement/selection/state/' + encodeURIComponent(id) + '/' + encodeURIComponent(state),
    dataType: 'json',
    success: function (text) {
      $('#ck-tab-mm-selection').text(text.text);

      if (refresh) {
        // Refresh.
        jQuery($('#edit-mediamosa-ck-media-management-batch-selection-refresh')).trigger('click');
      }
      result = true;
    },
    error: function (xmlhttp) {
      alert(Drupal.ajaxError(xmlhttp, 'mediamosa/ck/json/mediamanagement/selection/state'));
    }
  });

  return result;
};

/**
 * Attach ajax click link to the pager link.
 *
 * @param link
 *   The <a> link object.
 * @param wrapper
 *   The wrapper to load in.
 */
Drupal.mediamosaCK.MM.attachPagerLinkAjax = function(link, wrapper) {
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
