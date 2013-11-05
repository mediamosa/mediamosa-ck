/**
 * Basic JS functions.
 */

(function($) {

Drupal.behaviors.mediamosa_ck = {
  attach: function (context, settings) {
    // Process links for AJAX tabs.
    $('div.mediamosa-ck-tabs').once('mediamosa-ck-tabs', Drupal.mediamosaCK.mediamosa_ck_tab);

    // For closing the popups.
    $('div.mediamosa-ck-icon-close').once('ck-icon-close').click(function(e) {
      // Hide it first.
      $(this).parent('div.mediamosa-ck-popup').hide();

      // Remove the contents of the popup, will make the next popup faster.
      $(this).parent('div.mediamosa-ck-popup').remove();
    });
  }
};

Drupal.mediamosaCK = Drupal.mediamosaCK || {};

/**
 * Create upload ticket.
 *
 * @return array
 *   - 'action'
 *     The upload URL for the file.
 *   - 'progress_id'
 *     Progress ID to use when getting upload progression from server.
 *   - 'uploadprogress_url'
 *     A json based progress URL.
 *   - 'asset_id'
 *     The asset ID of the upload.
 *   - 'mediafile_id'
 *     The mediafile ID of the upload.
 */
Drupal.mediamosaCK.getUploadTicket = function(file) {
  var result = null;
  $.ajax({
    type: 'GET',
    url: Drupal.settings.basePath + 'mediamosa/ck/json/uploadticket/create?filename=' + escape(file.name),
		async: false,
    dataType: 'json',
    success: function (uploadticket) {
      if (typeof uploadticket.action !== undefined) {
        result = uploadticket;
      }
    },
    error: function (xmlhttp) {
      alert(Drupal.ajaxError(xmlhttp, 'mediamosa/ck/json/uploadticket/create'));
      console.log('Unable to get upload ticket!');
    }
  });
  return result;
}

/**
 * Check if MediaMosa connector is setup correctly.
 *
 * @return boolean
 *   Either true (success) or false (failure).
 */
Drupal.mediamosaCK.getConnectorStatus = function() {
  var result = false;
  $.ajax({
    type: 'GET',
    url: Drupal.settings.basePath + 'mediamosa/ck/json/connector/status',
		async: false,
    dataType: 'json',
    success: function (status) {
      result = (parseInt(status.ok) === 1);
    },
    error: function (xmlhttp) {
      alert(Drupal.ajaxError(xmlhttp, 'mediamosa/ck/json/connector/status'));
    }
  });

  return result;
}

/**
 * Helper function to parse a querystring.
 */
Drupal.mediamosaCK.parseQueryString = function (query) {
  var args = {};
  var pos = query.indexOf('?');
  if (pos !== -1) {
    query = query.substring(pos + 1);
  }
  var pairs = query.split('&');
  for(var i in pairs) {
    if (typeof(pairs[i]) === 'string') {
      var pair = pairs[i].split('=');
      // Ignore the 'q' path argument, if present.
      if (pair[0] !== 'q' && pair[1]) {
        args[decodeURIComponent(pair[0].replace(/\+/g, ' '))] = decodeURIComponent(pair[1].replace(/\+/g, ' '));
      }
    }
  }
  return args;
};

Drupal.mediamosaCK.mediamosa_ck_tab = function() {
  var navigation = $(this);

  // Hide all tab content.
  navigation.find('.mediamosa-ck-tab-content').hide();

  // If the current URL has a fragment and one of the tabs contains an
  // element that matches the URL fragment, activate that tab.
  var tab_focus;

  if (window.location.hash && $(this).find(window.location.hash).length) {
    tab_focus = navigation.find(window.location.hash).closest('ul.mediamosa-ck-tabs > li');
    navigation.find(window.location.hash).parents('ul.mediamosa-ck-tabs > li').map(function() {
      $('#' + $(this).children('a').attr('name')).show();
      $(this).addClass('active');
    });
  }

  if (tab_focus === undefined) {
    // Make first tab active.
    navigation.find('ul.mediamosa-ck-tabs > li').first().addClass('active');
    navigation.find('.mediamosa-ck-tab-content').first().show();
  }

  navigation.find('a.ck-tab-link').once('ck-tab-link').click(function(e) {
    // Show my contents.
    $('#' + $(this).attr('name')).show();

    // Hide other tab contents, except mine.
    $('#' + $(this).attr('name')).siblings('.mediamosa-ck-tab-content').hide();

    // Remove the 'active' status of all tabs.
    $(this).parents('ul').children('li').removeClass('active');

    // Activate my tab.
    $(this).parent('li').addClass('active');
  });
};

})(jQuery);
